#include "types.h"
#include "stat.h"
#include "user.h"

int main(){
    for (int i = 1; i <= 24; ++i)
        printf(1, "system call counter at beginning %d: %d\n", i, getsyscallcounter(i));

    int pid = getpid();
    printf(1, "parent id: %d\n", pid);

    int child1 = fork();
    if(child1 == 0){
        printf(1, "child1 (%d) parent id: %d\n", getpid(),getparentid());
        int child2 = fork();
        if(child2 == 0){
            printf(1, "child2 (%d) parent id: %d\n", getpid(), getparentid());
        }else{
            wait();
            exit();
        }

    }else{
        wait();
        for (int i = 1; i <= 24; ++i)
            printf(1, "system call counter at the end %d: %d\n", i, getsyscallcounter(i));
        exit();
    }
    exit();
}