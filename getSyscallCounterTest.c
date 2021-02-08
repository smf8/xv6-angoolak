#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]){
    int num = atoi(argv[1]);
    printf(1, "system call counter %d: %d\n", num, getsyscallcounter(num));
    exit();
}