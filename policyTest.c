//#include "types.h"
//#include "stat.h"
//#include "user.h"
//#include "proc.h"
//#include "defs.h"
//
//typedef struct {
//    long long tat;
//    long long w;
//    long long cbt;
//} avg;
//
//void testRoundRobin() {
//    avg sum;
//    sum.cbt = 0;
//    sum.w = 0;
//    sum.tat = 0;
//
//    int pid;
//    for (int i = 0; i < 10; ++i) {
//        if (pid = fork(), pid == 0)
//            break;
//    }
//
//    if (pid == 0) {
//        for (int i = 0; i < 1000; ++i) {
//            printf(1, "/%d/: %d", getpid(), i + 1);
//        }
//    } else {
//        wait();
//        printf(1, "avg) turn around time:%lld\nwaiting time: %lld\nCBT: %lld", sum.tat / 10,
//               sum.w / 10, sum.cbt / 10);
//        exit();
//    }
//
//    struct info *pinfo = (struct info *) malloc(sizeof(info));
//    if (getinfo(pid, info) != -1) {
//        long long turnAroundTime = pinfo->termination_time - pinfo->creation_time;
//        long long burstTime = turnAroundTime - pinfo->ready_time;
//
//        printf(1, "%d) turn around time:%lld, waiting time: %lld, CBT: %lld", getpid(), turnAroundTime,
//               pinfo->ready_time, burstTime);
//
//        acquire(&calculationlock);
//        sum.cbt += burstTime;
//        sum.w = pinfo->ready_time;
//        sum.tat = turnAroundTime;
//        release(&calculationlock);
//    }
//
//    exit()
//}
//
//int main(int argc, char *argv[]) {
//    int policy;
//    if (argc > 1)
//        policy = atoi(argv[1]);
//    else
//        policy = 1;
//
//    if (changepolicy(policy) != -1) {
//        switch (policy) {
//            case 1:
//                testRoundRobin();
//                break;
//        }
//    }
//
//    exit();
//}