#include "types.h"
#include "stat.h"
#include "user.h"

int main(){
    int pid = getpid();

    fork();
    fork();
    fork();
    fork();

    if(getpid() != pid){
        sleep(1000);
    }
    //only check for parent;
    if(getpid() == pid){
        printf(1, "parent ID: %d\nchilds: ", getpid());
        int children = getchildren();
        for(int j = children; j > 0; j /=100){
            int childID = j%100;
            printf(1, "%d ", childID);
        }
        printf(1,"\n");
    }
    exit();
}