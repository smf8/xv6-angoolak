#include "types.h"
#include "stat.h"
#include "user.h"

#define PROC 10
#define LOOP 1000

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
    int pids[PROC];

    for (int i = 0; i < PROC; ++i) {
        pid = fork();
        if (pid == 0) {
            pids[i] = getpid();
            break;
        }
    }

    if (pid == 0) {
        for (int i = 0; i < LOOP; ++i) {
            printf(1, "/%d/: /%d/\n", getpid(), i + 1);
        }
    } else {
        for (int i = 0; i < PROC; ++i) {
            wait();
        }

        for (int i = 0; i < PROC; ++i) {
            info *pinfo = (info *) malloc(sizeof(info));
            if (getinfo(pids[i], pinfo) != -1) {
                long long *turnAroundTime = (long long *) malloc(sizeof(long long));
                *turnAroundTime = pinfo->termination_time - pinfo->creation_time;

                printf(1, "%d) turn around time:%d, waiting time: %d, CBT: %d\n", pids[i], *turnAroundTime,
                       pinfo->ready_time, pinfo->running_time);

                increment(pinfo, suminfo, turnAroundTime);
            }

        }
//        double tat = suminfo->tat / 10, w = suminfo->w / 10, cbt = suminfo->cbt / 10;
        printf(1, "\navg) turn around time:%d\nwaiting time: %d\nCBT: %d", suminfo->tat, suminfo->w, suminfo->cbt);
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