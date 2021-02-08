#include "types.h"
#include "stat.h"
#include "user.h"

typedef struct {
    long long tat;
    long long w;
    long long cbt;
} sum;

typedef struct {
    long long creation_time;        // time in which the process created
    long long running_time;         // total time in which the process is running
    long long ready_time;           // total time in which the process is ready
    long long sleep_time;           // total time in which the process is sleeping
    long long termination_time;     // time in which the process terminated
} info;

void testRoundRobin() {
    sum *suminfo = (sum *)malloc(sizeof(sum));
    suminfo->cbt = 0;
    suminfo->w = 0;
    suminfo->tat = 0;

    int pid;
    for (int i = 0; i < 10; ++i) {
        if (pid = fork(), pid == 0)
            break;
    }

    if (pid == 0) {
        for (int i = 0; i < 1000; ++i) {
            printf(1, "/%d/: %d", getpid(), i + 1);
        }
    } else {
        wait();
        printf(1, "avg) turn around time:%lld\nwaiting time: %lld\nCBT: %lld", suminfo->tat / 10,
               suminfo->w / 10, suminfo->cbt / 10);
        exit();
    }

    info *pinfo = (info *) malloc(sizeof(info));
    if (getinfo(pid, info) != -1) {
        long long turnAroundTime = pinfo->termination_time - pinfo->creation_time;

        printf(1, "%d) turn around time:%lld, waiting time: %lld, CBT: %lld", getpid(), turnAroundTime,
               pinfo->ready_time, pinfo->running_time);

        increment(pinfo, suminfo);
    }

    exit()
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