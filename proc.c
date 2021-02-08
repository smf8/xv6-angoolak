#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

struct {
    struct spinlock lock;
    struct proc proc[NPROC];
} ptable;


struct queue{
    int front, rear, size;
    struct spinlock lock;
    struct proc* array[NPROC];
};

int lastQueue[NCPU];

struct queue schedulingQueues[4];


int policy = POLICY_MLQ;

static struct proc *initproc;

int nextpid = 1;

extern void forkret(void);

extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void) {
    initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
    return mycpu() - cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu *
mycpu(void) {
    int apicid, i;

    if (readeflags() & FL_IF)
        panic("mycpu called with interrupts enabled\n");

    apicid = lapicid();
    // APIC IDs are not guaranteed to be contiguous. Maybe we should have
    // a reverse map, or reserve a register to store &cpus[i].
    for (i = 0; i < ncpu; ++i) {
        if (cpus[i].apicid == apicid)
            return &cpus[i];
    }
    panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc *
myproc(void) {
    struct cpu *c;
    struct proc *p;
    pushcli();
    c = mycpu();
    p = c->proc;
    popcli();
    return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc *
allocproc(void) {
    struct proc *p;
    char *sp;

    acquire(&ptable.lock);

    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
        if (p->state == UNUSED)
            goto found;

    release(&ptable.lock);
    return 0;

    found:
    p->state = EMBRYO;
    p->priority = 3;
    p->scheduled_times = 0;
    p->queueNumber = QUEUE_DEFAULT;

    acquire(&schedulingQueues[0].lock);
    schedulingQueues[0].rear = (schedulingQueues[0].rear + 1) % NPROC;
    schedulingQueues[0].array[schedulingQueues[0].rear] = p;
    schedulingQueues[0].size++;
    release(&schedulingQueues[0].lock);

    cprintf("allocating shit %d = %d\n", schedulingQueues[0].rear, schedulingQueues[0].array[schedulingQueues[0].rear]);

    p->pid = nextpid++;
    p->syscallhistory = 0;

    release(&ptable.lock);

    // Allocate kernel stack.
    if ((p->kstack = kalloc()) == 0) {
        p->state = UNUSED;
        return 0;
    }
    sp = p->kstack + KSTACKSIZE;

    // Leave room for trap frame.
    sp -= sizeof *p->tf;
    p->tf = (struct trapframe *) sp;

    // Set up new context to start executing at forkret,
    // which returns to trapret.
    sp -= 4;
    *(uint *) sp = (uint) trapret;

    sp -= sizeof *p->context;
    p->context = (struct context *) sp;
    memset(p->context, 0, sizeof *p->context);
    p->context->eip = (uint) forkret;

    p->ready_time = 0;
    p->running_time = 0;
    p->sleep_time = 0;
    p->last_time = p->creation_time = ticks;

    return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void) {
    struct proc *p;
    extern char _binary_initcode_start[], _binary_initcode_size[];


    schedulingQueues[0].front = 0;
    schedulingQueues[0].rear = NPROC-1;
    schedulingQueues[0].size = 0;

    schedulingQueues[1].front = 0;
    schedulingQueues[1].rear = NPROC-1;
    schedulingQueues[1].size = 0;

    schedulingQueues[2].front = 0;
    schedulingQueues[2].rear = NPROC-1;
    schedulingQueues[2].size = 0;

    schedulingQueues[3].front = 0;
    schedulingQueues[3].rear = NPROC-1;
    schedulingQueues[3].size = 0;

    for (int i = 0; i < NPROC; i++) {
        schedulingQueues[0].array[i] = 0;
        schedulingQueues[1].array[i] = 0;
        schedulingQueues[2].array[i] = 0;
        schedulingQueues[3].array[i] = 0;
    }

    p = allocproc();

    initproc = p;
    if ((p->pgdir = setupkvm()) == 0)
        panic("userinit: out of memory?");
    inituvm(p->pgdir, _binary_initcode_start, (int) _binary_initcode_size);
    p->sz = PGSIZE;
    memset(p->tf, 0, sizeof(*p->tf));
    p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
    p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
    p->tf->es = p->tf->ds;
    p->tf->ss = p->tf->ds;
    p->tf->eflags = FL_IF;
    p->tf->esp = PGSIZE;
    p->tf->eip = 0;  // beginning of initcode.S

    safestrcpy(p->name, "initcode", sizeof(p->name));
    p->cwd = namei("/");

    // this assignment to p->state lets other cores
    // run this process. the acquire forces the above
    // writes to be visible, and the lock is also needed
    // because the assignment might not be atomic.
    acquire(&ptable.lock);

    p->state = RUNNABLE;

    release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n) {
    uint sz;
    struct proc *curproc = myproc();

    sz = curproc->sz;
    if (n > 0) {
        if ((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
            return -1;
    } else if (n < 0) {
        if ((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
            return -1;
    }
    curproc->sz = sz;
    switchuvm(curproc);
    return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void) {
    int i, pid;
    struct proc *np;
    struct proc *curproc = myproc();

    // Allocate process.
    if ((np = allocproc()) == 0) {
        return -1;
    }

    // Copy process state from proc.
    if ((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0) {
        kfree(np->kstack);
        np->kstack = 0;
        np->state = UNUSED;
        return -1;
    }
    np->sz = curproc->sz;
    np->parent = curproc;
    *np->tf = *curproc->tf;

    // Clear %eax so that fork returns 0 in the child.
    np->tf->eax = 0;

    for (i = 0; i < NOFILE; i++)
        if (curproc->ofile[i])
            np->ofile[i] = filedup(curproc->ofile[i]);
    np->cwd = idup(curproc->cwd);

    safestrcpy(np->name, curproc->name, sizeof(curproc->name));

    pid = np->pid;

    acquire(&ptable.lock);

    np->state = RUNNABLE;

    release(&ptable.lock);

    return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void) {
    struct proc *curproc = myproc();
    struct proc *p;
    int fd;

    if (curproc == initproc)
        panic("init exiting");

    curproc->termination_time = ticks;

    // Close all open files.
    for (fd = 0; fd < NOFILE; fd++) {
        if (curproc->ofile[fd]) {
            fileclose(curproc->ofile[fd]);
            curproc->ofile[fd] = 0;
        }
    }

    // Free syscall history linked list
    while (curproc->syscallhistory != 0) {
        syscallcounter *temp = curproc->syscallhistory;
        curproc->syscallhistory = curproc->syscallhistory->next;
        kfree((char *) temp);
    }


    begin_op();
    iput(curproc->cwd);
    end_op();
    curproc->cwd = 0;



    acquire(&ptable.lock);

    // Parent might be sleeping in wait().
    wakeup1(curproc->parent);

    // Pass abandoned children to init.
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->parent == curproc) {
            p->parent = initproc;
            if (p->state == ZOMBIE)
                wakeup1(initproc);
        }
    }

    // Jump into the scheduler, never to return.
    curproc->state = ZOMBIE;
    sched();
    panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void) {
    struct proc *p;
    int havekids, pid;
    struct proc *curproc = myproc();

    acquire(&ptable.lock);
    for (;;) {
        // Scan through table looking for exited children.
        havekids = 0;
        for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
            if (p->parent != curproc)
                continue;
            havekids = 1;
            if (p->state == ZOMBIE) {
                // Found one.
                pid = p->pid;
                kfree(p->kstack);
                p->kstack = 0;
                freevm(p->pgdir);
                p->pid = 0;
                p->parent = 0;
                p->name[0] = 0;
                p->killed = 0;
                p->state = UNUSED;
                release(&ptable.lock);
                return pid;
            }
        }

        // No point waiting if we don't have any children.
        if (!havekids || curproc->killed) {
            release(&ptable.lock);
            return -1;
        }

        // Wait for children to exit.  (See wakeup1 call in proc_exit.)
        sleep(curproc, &ptable.lock);  //DOC: wait-sleep
    }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void) {
    struct proc *p;
    struct cpu *c = mycpu();
    c->proc = 0;

    for (;;) {
        // Enable interrupts on this processor.
        sti();


        acquire(&ptable.lock);

        if (policy == POLICY_PRIORITY) {
            struct proc *selectedP = 0;
            selectedP = findPriority(1);
            if (selectedP != 0) {
                cprintf("process %d selected with priority %d\n", selectedP->pid, selectedP->priority);

                c->proc = selectedP;
                switchuvm(selectedP);
                selectedP->state = RUNNING;
                selectedP->scheduled_times++;

                swtch(&(c->scheduler), selectedP->context);
                switchkvm();

                c->proc = 0;
            }
        } else if(policy == POLICY_MLQ){
//            cprintf("cpu %d last queue : %d\n", cpuid(), lastQueue[cpuid()]);
            p = 0;
            for (int i = 1; i <= 4; i++) {
                int newQueue = (lastQueue[cpuid()] + i) % 4;
                if (schedulingQueues[newQueue].size == 0) {
                    continue;
                }

                if (newQueue == QUEUE_DEFAULT || newQueue == QUEUE_PRIORITY_RR) {
//                    for (int i = 0; i < NPROC; i++) {
//                        cprintf("queue[%d] : %d\n", i, schedulingQueues[newQueue].array[i]);
//                    }
                    for (;;) {
                        p = schedulingQueues[newQueue].array[schedulingQueues[newQueue].front];
                        schedulingQueues[newQueue].front = (schedulingQueues[newQueue].front + 1) % NPROC;
                        if (p->state == UNUSED || p->state == ZOMBIE) {
                            schedulingQueues[newQueue].size--;
                            continue;
                        } else {
                            schedulingQueues[newQueue].rear = (schedulingQueues[newQueue].rear + 1) % NPROC;
                            schedulingQueues[newQueue].array[schedulingQueues[newQueue].rear] = p;
                            break;
                        }
                    }
                } else {
                    struct proc *selectedP = 0;
                    if (newQueue == QUEUE_PRIORITY)
                        selectedP = findPriority(1);
                    else
                        selectedP = findPriority(0);

                    if (selectedP != 0) {
                        cprintf("process %d selected with priority %d\n", selectedP->pid, selectedP->priority);

                        c->proc = selectedP;
                        switchuvm(selectedP);
                        selectedP->state = RUNNING;
                        selectedP->scheduled_times++;

                        swtch(&(c->scheduler), selectedP->context);
                        switchkvm();

                        c->proc = 0;
                    }
                }
                if (p != 0 && p->state == RUNNABLE) {
                    cprintf("running process %d from queue[%d]\n", p->pid, newQueue);

                    c->proc = p;
                    switchuvm(p);
                    p->state = RUNNING;
                    p->scheduled_times++;

                    swtch(&(c->scheduler), p->context);
                    switchkvm();

                    c->proc = 0;
                }
            }
        } else{
            // Loop over process table looking for process to run.
            for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
                if (p->state != RUNNABLE)
                    continue;
                // Switch to chosen process.  It is the process's job
                // to release ptable.lock and then reacquire it
                // before jumping back to us.

                c->proc = p;
                switchuvm(p);

                p->ready_time += ticks - p->last_time;
                p->state = RUNNING;
                p->last_time = ticks;

                swtch(&(c->scheduler), p->context);
                switchkvm();

                // Process is done running for now.
                // It should have changed its p->state before coming back.
                c->proc = 0;
            }
        }
        release(&ptable.lock);

    }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void) {
    int intena;
    struct proc *p = myproc();

    lastQueue[cpuid()] = p->queueNumber;

    if (!holding(&ptable.lock))
        panic("sched ptable.lock");
    if (mycpu()->ncli != 1)
        panic("sched locks");
    if (p->state == RUNNING)
        panic("sched running");
    if (readeflags() & FL_IF)
        panic("sched interruptible");
    intena = mycpu()->intena;
    swtch(&p->context, mycpu()->scheduler);
    mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void) {
    acquire(&ptable.lock);  //DOC: yieldlock
    myproc()->running_time += ticks - myproc()->last_time;
    myproc()->state = RUNNABLE;
    sched();
    myproc()->last_time = ticks;
    release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void) {
    static int first = 1;
    // Still holding ptable.lock from scheduler.
    release(&ptable.lock);

    if (first) {
        // Some initialization functions must be run in the context
        // of a regular process (e.g., they call sleep), and thus cannot
        // be run from main().
        first = 0;
        iinit(ROOTDEV);
        initlog(ROOTDEV);
    }

    // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk) {
    struct proc *p = myproc();

    if (p == 0)
        panic("sleep");

    if (lk == 0)
        panic("sleep without lk");

    // Must acquire ptable.lock in order to
    // change p->state and then call sched.
    // Once we hold ptable.lock, we can be
    // guaranteed that we won't miss any wakeup
    // (wakeup runs with ptable.lock locked),
    // so it's okay to release lk.
    if (lk != &ptable.lock) {  //DOC: sleeplock0
        acquire(&ptable.lock);  //DOC: sleeplock1
        release(lk);
    }
    // Go to sleep.
    p->chan = chan;
    p->state = SLEEPING;

    sched();

    // Tidy up.
    p->chan = 0;

    // Reacquire original lock.
    if (lk != &ptable.lock) {  //DOC: sleeplock2
        release(&ptable.lock);
        acquire(lk);
    }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan) {
    struct proc *p;

    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
        if (p->state == SLEEPING && p->chan == chan) {
            p->sleep_time += ticks - p->last_time;
            p->state = RUNNABLE;
            p->last_time = ticks;
        }
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan) {
    acquire(&ptable.lock);
    wakeup1(chan);
    release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid) {
    struct proc *p;

    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->pid == pid) {
            p->termination_time = ticks;
            p->killed = 1;
            // Wake process from sleep if necessary.
            if (p->state == SLEEPING)
                p->state = RUNNABLE;

            while (p->syscallhistory != 0) {
                syscallcounter *temp = p->syscallhistory;
                p->syscallhistory = p->syscallhistory->next;
                kfree((char *) temp);
            }

            release(&ptable.lock);
            return 0;
        }
    }
    release(&ptable.lock);
    return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void) {
    static char *states[] = {
            [UNUSED]    "unused",
            [EMBRYO]    "embryo",
            [SLEEPING]  "sleep ",
            [RUNNABLE]  "runble",
            [RUNNING]   "run   ",
            [ZOMBIE]    "zombie"
    };
    int i;
    struct proc *p;
    char *state;
    uint pc[10];

    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state == UNUSED)
            continue;
        if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
            state = states[p->state];
        else
            state = "???";
        cprintf("%d %s %s", p->pid, state, p->name);
        if (p->state == SLEEPING) {
            getcallerpcs((uint *) p->context->ebp + 2, pc);
            for (i = 0; i < 10 && pc[i] != 0; i++)
                cprintf(" %p", pc[i]);
        }
        cprintf("\n");
    }
}

int getchildren() {
    struct proc *currproc = myproc();

    int resultchilds = 0;
    struct proc *p;
    acquire(&ptable.lock);
    cprintf("looking for childs of %d...\n", currproc->pid);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->parent->pid == currproc->pid) {
            //found a child
            cprintf("found child %d\n", p->pid);
            resultchilds *= 100;
            resultchilds += p->pid;
        }
    }
    release(&ptable.lock);

    return resultchilds;
}

int getsyscallcounter(int num) {
    struct proc *currproc = myproc();

    if (currproc == 0)
        return -1;

    syscallcounter *temp = currproc->syscallhistory;
    while (temp->next != 0 && temp->num != num)
        temp = temp->next;

    if (temp->num == num) {
        return temp->count;
    }

    return 0;
}

// mode = 1 : less priority value = higher priority
struct proc *findPriority(int mode){
    struct proc *p;
    struct proc *selectedP = 0;

    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state != RUNNABLE)
            continue;
        if (selectedP == 0) {
            selectedP = p;
        }

        if(mode == 1){
            if (selectedP->priority > p->priority) {
                selectedP = p;
            }

        }else{
            if (selectedP->priority < p->priority) {
                selectedP = p;
            }
        }

        // this trick is to find a process with same priority but less scheduled
        if (selectedP->priority == p->priority && selectedP->scheduled_times > p->scheduled_times) {
            selectedP = p;
        }
    }


    return selectedP;
}

int setPriority(int pid, int priority) {
    struct proc *p;

    int result = -1;

    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->pid == pid) {
            if (p->priority != priority) {
                p->scheduled_times = 0;
            }

            p->priority = priority;
            result = pid;
            break;
        }
    }
    release(&ptable.lock);

    return result;
}

int changepolicy(int p) {
    if (p < 0 || p > 2)
        return -1;

    return policy = p;
}

int getinfo(int pid, struct info *pinfo) {
    struct proc *p;

    int result = -1;

    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->pid == pid) {
            pinfo->sleep_time = p->sleep_time;
            pinfo->ready_time = p->ready_time;
            pinfo->running_time = p->running_time;
            pinfo->termination_time = p->creation_time;
            pinfo->termination_time = p->termination_time;

            result = pid;
            break;
        }
    }
    release(&ptable.lock);

    return result;
}

int setqueue(int pid, int queue){
    struct proc *p;

    int result = -1;
    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->pid == pid) {
            p->queueNumber = queue;
            result = pid;
        }
    }
    release(&ptable.lock);

    return result;
}

void increment(struct info* pinfo, sum * suminfo){
    acquire(&calculationlock);
    suminfo.cbt += pinfo->running_time;
    suminfo.w += pinfo->ready_time;
    suminfo.tat += turnAroundTime;
    release(&calculationlock);
}