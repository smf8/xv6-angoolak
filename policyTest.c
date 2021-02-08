#include "types.h"
#include "stat.h"
#include "user.h"

#define PROC 10
#define LOOP 1000

typedef struct sum {
    int tat;
    int w;
    int cbt;
} sum;

typedef struct info {
    int creation_time;        // time in which the process created
    int running_time;         // total time in which the process is running
    int ready_time;           // total time in which the process is ready
    int sleep_time;           // total time in which the process is sleeping
    int termination_time;     // time in which the process terminated
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
        if (pid == 0)
            break;
        else
            pids[i] = pid;
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
                int *turnAroundTime = (int *) malloc(sizeof(int));
                *turnAroundTime = pinfo->termination_time - pinfo->creation_time;

                printf(1, "%d) turn around time: %d", pids[i]);
                printf(1, " CBT: %d,", pinfo->running_time);
                printf(1, " waiting time: %d\n", pinfo->sleep_time);

                increment(pinfo, suminfo, turnAroundTime);
            }

        }
//        double tat = suminfo->tat / 10, w = suminfo->w / 10, cbt = suminfo->cbt / 10;
        printf(1, "\navg) turn around time:%d\nwaiting time: %d\nCBT: %d", suminfo->tat, suminfo->w, suminfo->cbt);
    }

    exit();
}

void testPrioritySched() {
    sum *suminfo = (sum *) malloc(sizeof(sum));
    suminfo->cbt = 0;
    suminfo->w = 0;
    suminfo->tat = 0;

    sum *suminfo1 = (sum *) malloc(sizeof(sum));
    suminfo1->cbt = 0;
    suminfo1->w = 0;
    suminfo1->tat = 0;

    sum *suminfo2 = (sum *) malloc(sizeof(sum));
    suminfo2->cbt = 0;
    suminfo2->w = 0;
    suminfo2->tat = 0;

    sum *suminfo3 = (sum *) malloc(sizeof(sum));
    suminfo3->cbt = 0;
    suminfo3->w = 0;
    suminfo3->tat = 0;

    sum *suminfo4 = (sum *) malloc(sizeof(sum));
    suminfo4->cbt = 0;
    suminfo4->w = 0;
    suminfo4->tat = 0;

    sum *suminfo5 = (sum *) malloc(sizeof(sum));
    suminfo5->cbt = 0;
    suminfo5->w = 0;
    suminfo5->tat = 0;

    sum *suminfo6 = (sum *) malloc(sizeof(sum));
    suminfo6->cbt = 0;
    suminfo6->w = 0;
    suminfo6->tat = 0;

    int pid, index;
    int pids[30];

    for (int i = 0; i < 5; ++i) {
        pid = fork();
        if (pid == 0) {
            index = i;
            setpriority(pids[i], 6);
            break;
        } else {
            pids[i] = pid;
        }
    }

    if (pid != 0)
        for (int i = 5; i < 10; ++i) {
            pid = fork();
            if (pid == 0) {
                index = i;
                setpriority(pids[i], 5);
                break;
            } else {
                pids[i] = pid;
            }
        }

    if (pid != 0)
        for (int i = 10; i < 15; ++i) {
            pid = fork();
            if (pid == 0) {
                index = i;
                setpriority(pids[i], 4);
                break;
            } else {
                pids[i] = pid;
            }
        }

    if (pid != 0)
        for (int i = 15; i < 20; ++i) {
            pid = fork();
            if (pid == 0) {
                index = i;
                setpriority(pids[i], 3);
                break;
            } else {
                pids[i] = pid;
            }
        }

    if (pid != 0)
        for (int i = 20; i < 25; ++i) {
            pid = fork();
            if (pid == 0) {
                index = i;
                setpriority(pids[i], 2);
                break;
            } else {
                pids[i] = pid;
            }
        }

    if (pid != 0)
        for (int i = 25; i < 30; ++i) {
            pid = fork();
            if (pid == 0) {
                index = i;
                setpriority(pids[i], 1);
                break;
            } else {
                pids[i] = pid;
            }
        }

    if (pid == 0) {
        for (int i = 0; i < 250; ++i) {
            printf(1, "/%d/: /%d/\n", index + 1, i + 1);
        }
    } else {
        for (int i = 0; i < 30; ++i) {
            wait();
        }

        for (int i = 0; i < 30; ++i) {
            info *pinfo = (info *) malloc(sizeof(info));
            if (getinfo(pids[i], pinfo) != -1) {
                int *turnAroundTime = (int *) malloc(sizeof(int));
                *turnAroundTime = pinfo->termination_time - pinfo->creation_time;

                printf(1, "%d) turn around time:%d, waiting time: %d, CBT: %d\n", pids[i], *turnAroundTime,
                       pinfo->ready_time, pinfo->running_time);

                increment(pinfo, suminfo, turnAroundTime);
            }
        }

        for (int i = 0; i < 5; ++i) {
            info *pinfo = (info *) malloc(sizeof(info));
            if (getinfo(pids[i], pinfo) != -1) {
                int *turnAroundTime = (int *) malloc(sizeof(int));
                *turnAroundTime = pinfo->termination_time - pinfo->creation_time;

                increment(pinfo, suminfo6, turnAroundTime);
            }
        }

        for (int i = 5; i < 10; ++i) {
            info *pinfo = (info *) malloc(sizeof(info));
            if (getinfo(pids[i], pinfo) != -1) {
                int *turnAroundTime = (int *) malloc(sizeof(int));
                *turnAroundTime = pinfo->termination_time - pinfo->creation_time;

                increment(pinfo, suminfo5, turnAroundTime);
            }
        }

        for (int i = 10; i < 15; ++i) {
            info *pinfo = (info *) malloc(sizeof(info));
            if (getinfo(pids[i], pinfo) != -1) {
                int *turnAroundTime = (int *) malloc(sizeof(int));
                *turnAroundTime = pinfo->termination_time - pinfo->creation_time;

                increment(pinfo, suminfo4, turnAroundTime);
            }
        }

        for (int i = 15; i < 20; ++i) {
            info *pinfo = (info *) malloc(sizeof(info));
            if (getinfo(pids[i], pinfo) != -1) {
                int *turnAroundTime = (int *) malloc(sizeof(int));
                *turnAroundTime = pinfo->termination_time - pinfo->creation_time;

                increment(pinfo, suminfo3, turnAroundTime);
            }
        }

        for (int i = 20; i < 25; ++i) {
            info *pinfo = (info *) malloc(sizeof(info));
            if (getinfo(pids[i], pinfo) != -1) {
                int *turnAroundTime = (int *) malloc(sizeof(int));
                *turnAroundTime = pinfo->termination_time - pinfo->creation_time;

                increment(pinfo, suminfo2, turnAroundTime);
            }
        }

        for (int i = 25; i < 30; ++i) {
            info *pinfo = (info *) malloc(sizeof(info));
            if (getinfo(pids[i], pinfo) != -1) {
                int *turnAroundTime = (int *) malloc(sizeof(int));
                *turnAroundTime = pinfo->termination_time - pinfo->creation_time;

                increment(pinfo, suminfo1, turnAroundTime);
            }
        }

        printf(1, "\n6) turn around time:%d\nwaiting time: %d\nCBT: %d", suminfo6->tat, suminfo6->w,
               suminfo6->cbt);

        printf(1, "\n5) turn around time:%d\nwaiting time: %d\nCBT: %d", suminfo5->tat, suminfo5->w,
               suminfo5->cbt);

        printf(1, "\n4) turn around time:%d\nwaiting time: %d\nCBT: %d", suminfo4->tat, suminfo4->w,
               suminfo4->cbt);

        printf(1, "\n3) turn around time:%d\nwaiting time: %d\nCBT: %d", suminfo3->tat, suminfo3->w,
               suminfo3->cbt);

        printf(1, "\n2) turn around time:%d\nwaiting time: %d\nCBT: %d", suminfo2->tat, suminfo2->w,
               suminfo2->cbt);

        printf(1, "\n1) turn around time:%d\nwaiting time: %d\nCBT: %d", suminfo1->tat, suminfo1->w,
               suminfo1->cbt);

//        double tat = suminfo->tat / 10, w = suminfo->w / 10, cbt = suminfo->cbt / 10;
        printf(1, "\ntotal avg) turn around time:%d\nwaiting time: %d\nCBT: %d", suminfo->tat, suminfo->w,
               suminfo->cbt);
    }

    exit();
}

void testPriorityQueue() {
    sum *suminfo = (sum *) malloc(sizeof(sum));
    suminfo->cbt = 0;
    suminfo->w = 0;
    suminfo->tat = 0;

    sum *suminfo1 = (sum *) malloc(sizeof(sum));
    suminfo1->cbt = 0;
    suminfo1->w = 0;
    suminfo1->tat = 0;

    sum *suminfo2 = (sum *) malloc(sizeof(sum));
    suminfo2->cbt = 0;
    suminfo2->w = 0;
    suminfo2->tat = 0;

    sum *suminfo3 = (sum *) malloc(sizeof(sum));
    suminfo3->cbt = 0;
    suminfo3->w = 0;
    suminfo3->tat = 0;

    sum *suminfo4 = (sum *) malloc(sizeof(sum));
    suminfo4->cbt = 0;
    suminfo4->w = 0;
    suminfo4->tat = 0;

    int pid, index;
    int pids[40];

    for (int i = 0; i < 10; ++i) {
        pid = fork();
        if (pid == 0) {
            index = i;
            setqueue(pids[i], 0);
            setpriority(pids[i], (i % 6) + 1);
            break;
        } else {
            pids[i] = pid;
        }
    }

    if (pid != 0)
        for (int i = 10; i < 20; ++i) {
            pid = fork();
            if (pid == 0) {
                index = i;
                setqueue(pids[i], 1);
                setpriority(pids[i], (i % 6) + 1);
                break;
            } else {
                pids[i] = pid;
            }
        }

    if (pid != 0)
        for (int i = 20; i < 30; ++i) {
            pid = fork();
            if (pid == 0) {
                index = i;
                setqueue(pids[i], 2);
                setpriority(pids[i], (i % 6) + 1);
                break;
            } else {
                pids[i] = pid;
            }
        }

    if (pid != 0)
        for (int i = 30; i < 40; ++i) {
            pid = fork();
            if (pid == 0) {
                index = i;
                setqueue(pids[i], 3);
                setpriority(pids[i], (i % 6) + 1);
                break;
            } else {
                pids[i] = pid;
            }
        }

    if (pid == 0) {
        for (int i = 0; i < 200; ++i) {
            printf(1, "/%d/: /%d/\n", index + 1, i + 1);
        }
    } else {
        for (int i = 0; i < 30; ++i) {
            wait();
        }

        for (int i = 0; i < 30; ++i) {
            info *pinfo = (info *) malloc(sizeof(info));
            if (getinfo(pids[i], pinfo) != -1) {
                int *turnAroundTime = (int *) malloc(sizeof(int));
                *turnAroundTime = pinfo->termination_time - pinfo->creation_time;

                printf(1, "%d) turn around time:%d, waiting time: %d, CBT: %d\n", pids[i], *turnAroundTime,
                       pinfo->ready_time, pinfo->running_time);

                increment(pinfo, suminfo, turnAroundTime);
            }
        }

        for (int i = 0; i < 5; ++i) {
            info *pinfo = (info *) malloc(sizeof(info));
            if (getinfo(pids[i], pinfo) != -1) {
                int *turnAroundTime = (int *) malloc(sizeof(int));
                *turnAroundTime = pinfo->termination_time - pinfo->creation_time;

                increment(pinfo, suminfo1, turnAroundTime);
            }
        }

        for (int i = 5; i < 10; ++i) {
            info *pinfo = (info *) malloc(sizeof(info));
            if (getinfo(pids[i], pinfo) != -1) {
                int *turnAroundTime = (int *) malloc(sizeof(int));
                *turnAroundTime = pinfo->termination_time - pinfo->creation_time;

                increment(pinfo, suminfo2, turnAroundTime);
            }
        }

        for (int i = 10; i < 15; ++i) {
            info *pinfo = (info *) malloc(sizeof(info));
            if (getinfo(pids[i], pinfo) != -1) {
                int *turnAroundTime = (int *) malloc(sizeof(int));
                *turnAroundTime = pinfo->termination_time - pinfo->creation_time;

                increment(pinfo, suminfo3, turnAroundTime);
            }
        }

        for (int i = 15; i < 20; ++i) {
            info *pinfo = (info *) malloc(sizeof(info));
            if (getinfo(pids[i], pinfo) != -1) {
                int *turnAroundTime = (int *) malloc(sizeof(int));
                *turnAroundTime = pinfo->termination_time - pinfo->creation_time;

                increment(pinfo, suminfo4, turnAroundTime);
            }
        }

        printf(1, "\n1) turn around time:%d\nwaiting time: %d\nCBT: %d", suminfo1->tat, suminfo1->w,
               suminfo1->cbt);

        printf(1, "\n2) turn around time:%d\nwaiting time: %d\nCBT: %d", suminfo2->tat, suminfo2->w,
               suminfo2->cbt);

        printf(1, "\n3) turn around time:%d\nwaiting time: %d\nCBT: %d", suminfo3->tat, suminfo3->w,
               suminfo3->cbt);

        printf(1, "\n4) turn around time:%d\nwaiting time: %d\nCBT: %d", suminfo4->tat, suminfo4->w,
               suminfo4->cbt);

//        double tat = suminfo->tat / 10, w = suminfo->w / 10, cbt = suminfo->cbt / 10;
        printf(1, "\ntotal avg) turn around time:%d\nwaiting time: %d\nCBT: %d", suminfo->tat, suminfo->w,
               suminfo->cbt);
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
            case 2:
                testPrioritySched();
                break;
            case 3:
                testPriorityQueue();
                break;
        }
    }

    exit();
}