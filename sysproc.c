#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void) {
    return fork();
}

int
sys_exit(void) {
    exit();
    return 0;  // not reached
}

int
sys_wait(void) {
    return wait();
}

int
sys_kill(void) {
    int pid;

    if (argint(0, &pid) < 0)
        return -1;
    return kill(pid);
}

int
sys_getpid(void) {
    return myproc()->pid;
}

int
sys_sbrk(void) {
    int addr;
    int n;

    if (argint(0, &n) < 0)
        return -1;
    addr = myproc()->sz;
    if (growproc(n) < 0)
        return -1;
    return addr;
}

int
sys_sleep(void) {
    int n;
    uint ticks0;

    if (argint(0, &n) < 0)
        return -1;
    acquire(&tickslock);
    ticks0 = ticks;
    while (ticks - ticks0 < n) {
        if (myproc()->killed) {
            release(&tickslock);
            return -1;
        }
        sleep(&ticks, &tickslock);
    }
    release(&tickslock);
    return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void) {
    uint xticks;

    acquire(&tickslock);
    xticks = ticks;
    release(&tickslock);
    return xticks;
}

int sys_getparentid(void) {
    // access parent process
    struct proc *currproc = myproc();
    return currproc->parent->pid;
}

int sys_getchildren(void) {
    return getchildren();
}

int sys_getsyscallcounter(void) {
    int num;

    if (argint(0, &num) < 0)
        return -1;

    return getsyscallcounter(num);
}

int sys_setpriority(void) {
    int pid;
    int priority;

    if (argint(0, &pid) < 0 || argint(1, &priority) < 0)
        return -1;

    if (pid < 0 || pid > NPROC)
        return -1;
    if (priority < 1)
        return -1;

    if (priority > 6)
        priority = 5;

    return setPriority(pid, priority);
}

int sys_setqueue(void) {
    int pid;
    int queue;

    if (argint(0, &pid) < 0 || argint(1, &queue) < 0)
        return -1;

    if (pid < 0 || pid > NPROC)
        return -1;
    if (queue < 0)
        return -1;

    return setqueue(pid, queue);
}

int sys_changepolicy(void) {
    int policy;

    if (argint(0, &policy) < 0)
        return -1;

    return changepolicy(policy);
}

int sys_getinfo(void) {
    int pid;
    struct info *pinfo;

    if (argint(0, &pid) < 0 || argptr(1, (void *) &pinfo, sizeof(*pinfo)))
        return -1;

    if (pid < 0 || pid > NPROC)
        return -1;

    return getinfo(pid, pinfo);
}

int sys_increment(void) {
    struct sum *suminfo;
    struct info *pinfo;
    long long *tat;

    if (argptr(2, (void *) &tat, sizeof(*tat)) < 0 || argptr(1, (void *) &suminfo, sizeof(*suminfo)) ||
        argptr(0, (void *) &pinfo, sizeof(*pinfo)))
        return -1;

    increment(pinfo, suminfo, tat);
    return 0;
}