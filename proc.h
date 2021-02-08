// Per-CPU state
struct cpu {
    uchar apicid;                // Local APIC ID
    struct context *scheduler;   // swtch() here to enter scheduler
    struct taskstate ts;         // Used by x86 to find stack for interrupt
    struct segdesc gdt[NSEGS];   // x86 global descriptor table
    volatile uint started;       // Has the CPU started?
    int ncli;                    // Depth of pushcli nesting.
    int intena;                  // Were interrupts enabled before pushcli?
    struct proc *proc;           // The process running on this cpu or null
};

extern struct cpu cpus[NCPU];
extern int ncpu;

#define POLICY_DEFAULT 0
#define POLICY_ROUND_ROBIN 1
#define POLICY_PRIORITY 2 // process with higher priority runs first
#define POLICY_MLQ 3

#define QUEUE_DEFAULT 0
#define QUEUE_PRIORITY 1
#define QUEUE_PRIORITY_REVERSE 2
#define QUEUE_PRIORITY_RR 3

//PAGEBREAK: 17
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
    uint edi;
    uint esi;
    uint ebx;
    uint ebp;
    uint eip;
};

enum procstate {
    UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE
};

typedef struct syscallcounter {
    int num;
    int count;
    struct syscallcounter *next;
} syscallcounter;

typedef struct info{
    int creation_time;        // time in which the process created
    int running_time;         // total time in which the process is running
    int ready_time;           // total time in which the process is ready
    int sleep_time;           // total time in which the process is sleeping
    int termination_time;     // time in which the process terminated
} info;

struct info infos[NPROC];

struct sum {
    int tat;
    int w;
    int cbt;
};

// Per-process state
struct proc {
    uint sz;                        // Size of process memory (bytes)
    pde_t *pgdir;                   // Page table
    char *kstack;                   // Bottom of kernel stack for this process
    enum procstate state;           // Process state
    int pid;                        // Process ID
    struct proc *parent;            // Parent process
    struct trapframe *tf;           // Trap frame for current syscall
    struct context *context;        // swtch() here to run process
    void *chan;                     // If non-zero, sleeping on chan
    int killed;                     // If non-zero, have been killed
    struct file *ofile[NOFILE];     // Open files
    struct inode *cwd;              // Current directory
    char name[16];                  // Process name (debugging)
    syscallcounter *syscallhistory; // history of called syscalls
    int priority;                   // priority value for scheduling
    int queueNumber;
    int scheduled_times;      // number of times this process was scheduled
    int creation_time;        // time in which the process created
    int running_time;         // total time in which the process is running
    int ready_time;           // total time in which the process is ready
    int sleep_time;           // total time in which the process is sleeping
    int termination_time;     // time in which the process terminated
    int last_time;
};

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap

