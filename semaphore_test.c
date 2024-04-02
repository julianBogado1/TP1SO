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
    //down(sem)

    int childpid;
    FILE * file = fopen("test.txt", "w");

    for(int i=0; i<5; i++){
        if((childpid=fork())==0){
            //child code
            fprintf(file, "Child %d", getpid());
        }
        else if(childpid<0){
            perror("fork failed");
        }
        else{
            //parent
            int pid=getpid();
        
            waitpid(pid+1, NULL, 0);    //quiero q espere a TODOS los hijos
            waitpid(pid+2, NULL, 0);
            waitpid(pid+3, NULL, 0);
            waitpid(pid+4, NULL, 0);
            waitpid(pid+5, NULL, 0);
        }
    }

    fclose(file);
    sem_close(sem);
    sem_unlink("test");
    return 0;
}