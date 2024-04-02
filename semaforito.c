#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <semaphore.h>

int main(int argc, char * argv[]){
    sem_t *sem = sem_open("/test", O_CREAT, 0777, 1);
    sem_close(sem);
}