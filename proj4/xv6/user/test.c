#include "types.h"
#include "stat.h"
#include "user.h" 
#include "pstat.h"

int main(void) {
    printf(1, "Hello \n");
    struct pstat p;
    struct pstat *ptr = &p;
    settickets(3);

    getpinfo(ptr);
    for(int i=0;i<NPROC;i++) {
        printf(1, "Process ID: %d, In Use: %d, Tickets: %d, Strides: %d, Pass: %d\n",ptr->pid[i],ptr->inuse[i], ptr->tickets[i], ptr->strides[i], ptr->pass[i]); 
    }

    exit();
}