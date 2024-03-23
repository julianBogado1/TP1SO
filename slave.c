// uso de este escalvo: ./slave <archivo>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_COMMAND_LEN 100


int main(int argc, char *argv[])
{

    if (argc == 1){
        perror("argc pelele"); // >:(
        return 1;
    }

    char filename[100];
    strcpy(filename, argv[1]);
    char md5[200]; // el md5 sera de a lo sumo 16 bytes-128bit pero dejo mas x las dudas
    // pid_t mypid = getpid();
    // int pipeFd = argv[1];   //dejamos el fd del pipe en argv[1]
    char command[MAX_COMMAND_LEN];


    for (int i = 1; i < argc; i++)
    {

        sprintf(command, "md5sum %s", argv[i]);
        FILE *md5_pipe = popen(command, "r"); // ejecuta el comando "command" con el argumento recibido por consola en slave

        if (md5_pipe == NULL)
        {
            perror("popen");
            return 1;
        }

        fscanf(md5_pipe, "%s %s", md5, filename);
        pclose(md5_pipe);   
        printf("%s %s saracatunga lcdll\n", md5, filename);

        // write(pipeFd, "\n", 1);

    }
    return 0;
}
