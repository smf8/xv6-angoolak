#include "types.h"
#include "stat.h"
#include "user.h"

typedef struct sum {
    long long tat;
    long long w;
    long long cbt;
} sum;

typedef struct info {
    long long creation_time;        // time in which the process created
    long long running_time;         // total time in which the process is running
    long long ready_time;           // total time in which the process is ready
    long long sleep_time;           // total time in which the process is sleeping
    long long termination_time;     // time in which the process terminated
} info;

void testRoundRobin() {
    sum *suminfo = (sum *) malloc(sizeof(sum));
    suminfo->cbt = 0;
    suminfo->w = 0;
    suminfo->tat = 0;

    int pid;
    for (int i = 0; i < 10; ++i) {
        pid = fork();
        if (pid == 0)
            break;
    }

    if (pid == 0) {
        for (int i = 0; i < 1000; ++i) {
            printf(1, "/%d/: /%d/\n", getpid(), i + 1);
        }
    } else {
        for (int i = 0; i < 10; ++i) {
            wait();
        }
//        double tat = suminfo->tat / 10, w = suminfo->w / 10, cbt = suminfo->cbt / 10;
        printf(1, "\navg) turn around time:%d\nwaiting time: %d\nCBT: %d", suminfo->tat, suminfo->w, suminfo->cbt);
        exit();
    }

    info *pinfo = (info *) malloc(sizeof(info));
    if (getinfo(pid, pinfo) != -1) {
        long long *turnAroundTime = (long long *) malloc(sizeof(long long));
        *turnAroundTime = pinfo->termination_time - pinfo->creation_time;

        printf(1, "%d) turn around time:%d, waiting time: %d, CBT: %d\n", getpid(), *turnAroundTime,
               pinfo->ready_time, pinfo->running_time);

        increment(pinfo, suminfo, turnAroundTime);
    }

    exit();
}

int main(int argc, char *argv[]) {
    int policy;
    if (argc > 1)
        policy = atoi(argv[1]);
    else
        policy = 1;

    if (changepolicy(policy) != -1) {
        switch (policy) {
            case 1:
                testRoundRobin();
                break;
        }
    }

    exit();
}