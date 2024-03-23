// uso de este escalvo: ./slave <archivo>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_COMMAND_LEN 100

int main(int argc, char *argv[]){
    if(argc <2){
        printf("Formato esperado:   ./slave <nombre_archivo1> <nombre_archivo_n>\n");
        return 1;
    }
    char filename[100];
    char md5[200]; // el md5 sera de a lo sumo 16 bytes-128bit pero dejo mas x las dudas

    pid_t mypid = getpid();
    int readFd;  
    int writeFd; 
    sscanf(argv[1], "%d", &readFd); //dejamos el fd del pipe en argv[1]
    sscanf(argv[2], "%d", &writeFd);

    char command[MAX_COMMAND_LEN];
    printf("my pid is: %d   readFd: %d   writeFd: %d\n", mypid, readFd, writeFd);
    /*for (int i = 1; i < argc; i++)
    {
        
        sprintf(command, "md5sum %s", argv[i]);

        FILE *pipe = popen(command, "r"); // ejecuta el comando "command" con el argumento recibido por consola en slave
        if (pipe == NULL)
        {
            perror("popen");
            return 1;
        }

        fscanf(pipe, "%s %s", md5, filename);
        pclose(pipe);
        printf("%s %s saracatunga lcdll\n", md5, filename);


        //write(pipeFd, "\n", 1);

    }*/
    return 0;
}
