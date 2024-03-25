// uso de este escalvo: ./slave <archivo>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_COMMAND_LEN 100

int main(int argc, char *argv[]){

    int readFd;  
    int writeFd; 
    sscanf(argv[1], "%d\n%d", &readFd, &writeFd); //dejamos el fd del pipe en argv[1]
    //ahora en vez de recibir los nombres de los archivos por consola, los recibo por pipe
    
    printf("soy: %d     readfd: %d  writefd: %d\n", getpid(), readFd, writeFd);

    char filename[100];
    char buff;
    int strSize = 0;
    close(writeFd);
    while(read(readFd, buff, 3)>0){
        putchar(buff);
        filename[strSize++] = buff;
    }
    filename[strSize] = '\0';
    printf("%s\n", filename);
    
    //en vez de printear los arhivos, los debe enviar por pipe
    close(readFd);


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
