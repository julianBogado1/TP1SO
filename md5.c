
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

//este master genera en total 10 esclavos --> mejorable

void pipeAndFork(int argc, char * argv[]);

int  main(int argc, char * argv[]){
    if(argc <= 1){
        printf("Formato esperado:   ./md5 <nombre_archivo1> <nombre_archivo_n>\n");
        return 1;
    }
    pipeAndFork(argc, argv);
}

void pipeAndFork(int argc, char * argv[]){
    
    int sendPipeFd[2];
    if (pipe(sendPipeFd) == -1) {
        perror("send pipe");
        exit(EXIT_FAILURE);
    }

    int receivePipeFd[2];
    if (pipe(receivePipeFd) == -1) {
        perror("receive pipe");
        exit(EXIT_FAILURE);
    }


    pid_t  cpid;
    cpid = fork();
    if (cpid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    
    if (cpid == 0) {    /* Child reads from pipe */
        close(sendPipeFd[1]);          /* Close unused write end in child*/
        close(receivePipeFd[0]);      /* Close unused read end in child*/


        


        char filename[100];
        int strSize = 0;
        char   buf;
        while (read(sendPipeFd[0], &buf, 1) > 0)
            filename[strSize++] = buf;

        filename[strSize] = '\0';
        printf("%s  dont forget to saracatunga\n", filename);


        //capaz se puede reemplazar command y simplemente hacer strcat("md5sum", filename)
        char command[100];          
        sprintf(command, "md5sum %s", filename);

        FILE *pipe = popen(command, "r"); // ejecuta el comando "command" con el argumento recibido por consola en slave
        if (pipe == NULL){
            perror("popen");
            return 1;
        }

        char md5[100];
        char scannedFilename[100];
        fscanf(pipe, "%s %s", md5, scannedFilename);
        pclose(pipe);

        
        write(receivePipeFd[1], scannedFilename, strlen(scannedFilename)); //envio el nombre del archivo por pipe
        write(receivePipeFd[1], "\n", 1); 
        write(receivePipeFd[1], md5, strlen(md5)); //envio el md5 por pipe
        write(receivePipeFd[1], "\n", 1);
        char pid[100];
        sprintf(pid, "my pid: %d", getpid());
        write(receivePipeFd[1], pid, strlen(pid)); //envio el pid por pipe

        close(sendPipeFd[0]);
        close(receivePipeFd[1]);
        _exit(EXIT_SUCCESS);

    } else {            /* Parent writes argv[1] to pipe */
        close(sendPipeFd[0]);          /* Close unused read end */
        close(receivePipeFd[1]);      /* Close unused write end */

        write(sendPipeFd[1], argv[1], strlen(argv[1]));
        close(sendPipeFd[1]);          /* Reader will see EOF */

        wait(NULL);                /* Wait for child */

        char filename[100];
        int strSize = 0;
        char buf;
        while(read(receivePipeFd[0], &buf, 1)>0){
            filename[strSize++] = buf;
        }
        filename[strSize] = '\0';
        printf("%s soy el papasito y saracatunga btw\n", filename);

        exit(EXIT_SUCCESS);
    }
    return;
}



