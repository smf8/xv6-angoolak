#include "types.h"
#include "stat.h"
#include "user.h"


long long doShit(long long n) {
    int j = 1;
    for (int i = 0; i < n; ++i) {
        j += i * j + j % 23;
    }
    return j;
}

int testRoundrobin() {
    int result = 0;

    int child1, child2 = 0;
    child1 = fork();
    if (child1 == 0) {
        for (int i = 0; i < 1000000000000; ++i) {
            result++;
        }
        printf(1, "%d %d:  %d\n", getpid(), child1, result);
    } else {

        child2 = fork();
        if (child2 == 0) {
            for (int i = 0; i < 10000; ++i) {
                result++;
            }
            printf(1, "%d %d:  %d\n", getpid(), child2, result);

        } else {
            for (int j = 0; j < 1000000000000; ++j) {
//                result++;
            }

            printf(1, "%d %d %d: %d\n", getpid(), child1, child2, result);
        }
    };


    exit();
}

int testPriority() {
    // we create 3 processes,
    // parent, child1, child2
    // execution time of theese processes are:
    // parent < child2 < child1
    // with a successful priority based scheduling,
    // if we prioritize  child1 over parent,
    // we expect child1 to finish sooner than others
    // play with priority and loop invariant

    int parentID = getpid(), child1ID, child2ID;

    int err = setpriority(parentID, 5);
    if (err == -1) {
        printf(1, "failed to set parent priority\n");
    }

    if (fork() == 0) {
        child1ID = getpid();
        int err = setpriority(child1ID, 1);
        if (err == -1) {
            printf(1, "failed to set child1 priority\n");
        }
//        int s = doShit(9000000000000);
        int j = 1;
        for (int i = 0; i < 900000000; ++i) {
            j += i * j + j % 23;
        }
        printf(1, "child1[%d] completed %d\n", child1ID, j);
    } else {
        if (fork() == 0) {
            child2ID = getpid();
            int err = setpriority(child2ID, 4);
            if (err == -1) {
                printf(1, "failed to set child2 priority\n");
            }

            int j = 1;
            for (int i = 0; i < 500000000; ++i) {
                j += i * j + j % 23;
            }

            printf(1, "child2[%d] completed %d\n", child2ID, j);
        } else {
            int j = 1;
            for (int i = 0; i < 100000000; ++i) {
                j += i * j + j % 23;
            }
            printf(1, "parent[%d] completed %d\n", parentID, j);

            wait();
            wait();

        }
    }

    exit();
}

int testMLQ(){

    int parentID = getpid();
    int pids[20];
    for (int i = 0; i <20 ; i++) {
        if(getpid() == parentID){
            pids[i] = fork();
        }else{
            int pid = getpid();
            if(pid % 6 == 0){
                setqueue(pid, 1);
                setpriority(pid, (pid/6)+1);
            }else if(pid % 5 == 0){
                setqueue(pid, 2);
                setpriority(pid, (pid/5)+1);
            }

            int j = 1;
            for (int i = 0; i < 500000000; ++i) {
                j += i * j + j % 23;
            }

            printf(1, "child[%d] finished it's shit = \n", pid, j);
            break;
        }
    }


    if(getpid() == parentID) {
        for (int i = 0; i < 20; ++i) {
            wait();
        }

        for (int i = 0; i < 20; i++) {
            printf(1, "child[%d] : %d\n", i, pids[i]);
        }
    }
    exit();
    return 0;
}

int main() {
    testMLQ();
}
