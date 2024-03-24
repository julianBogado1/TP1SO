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
    sscanf(argv[1], "%d", &readFd); //dejamos el fd del pipe en argv[1]
    sscanf(argv[2], "%d", &writeFd);   

    //ahora en vez de recibir los nombres de los archivos por consola, los recibo por pipe
    char filename[100];
    int i = 0;                  //offset de filenames
    int strSize = 0;
    printf("soy: %d\n", getpid());
    while(read(readFd, filename, 1)>0 && i<argc-1){
        if(filename[strSize] == '\n'){
            i++;
        }
        else{
            printf("%c", filename[strSize]);
            strSize++;
        }      
    }
    
    for(i=0;i<argc-1; i++){
        printf("%s\n", filename);
    }

    //en vez de printear los arhivos, los debe enviar por pipe
    close(readFd);
    close(writeFd);


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
