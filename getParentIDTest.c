#include "types.h"
#include "stat.h"
#include "user.h"

int main() {
    int pid = getpid();
    printf(1, "parent id: %d\n", pid);

    int child1 = fork();
    if (child1 == 0) {
        printf(1, "child1 (%d) parent id: %d\n", getpid(), getparentid());
        int child2 = fork();
        if (child2 == 0) {
            printf(1, "child2 (%d) parent id: %d\n", getpid(), getparentid());
        } else {
            wait();
            return 0;
        }

    } else {
        wait();
        return 0;
    }
}