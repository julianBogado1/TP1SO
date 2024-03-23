//programa que recibe por linea de comando los nombres de los archivos que debe analizar
//deebe iniciar esclavos
//debe distribuir una cantidad mucho menor de los archivos a cada esclavo
//        -->esperar que terminen de obtener el md5, ellos le devuelven el resultado a el, EL LO PONE EN LA SHARE MEMORY
//        -->pasarle mas archivos hasta que se quede sin

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

//este master genera en total 10 esclavos --> mejorable

void pipeAndFork(){
    
    //creacion de send pipe
    int sendPipeFd[2]; // sendPipeFd[0] read, sendPipeFd[1] writing
    if (pipe(pipefd) == -1){
        perror("pipe");
        exit(1);
    }

    //creacion de receive pipe
    int receivePipeFd[2]; // sendPipeFd[0] read, sendPipeFd[1] writing
    if (pipe(pipefd) == -1){
        perror("pipe");
        exit(1);
    }

    char * argvSon[5];
    argvSon[0] = "./slave";             //por convencion el primer argumento es el nombre del programa   
    argvSon[1] = sendPipeFd[1] + '0';     //read-end para el hijo
    argvSon[2] = receivePipeFd[0] + '0';  //write-end para el hijo
    char * execArgs[3] = {"./slave", argvSon, NULL};

    pid_t childpid;
    childpid = fork();
    if (childpid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (childpid == 0){ 
        int status = execve("./sonCode", execArgs, NULL);
        perror("execve");
        printf("%d\n", status);
        exit(EXIT_FAILURE);
    }
    else{
        
        close(sendPipeFd[1]); //cierro el read-end del sendPipe para el padre
        close(receivePipeFd[0]); //cierro el write-end del receivePipe para el padre

        wait(NULL);       // espera hasta que **algun** hijo termine

        close(sendPipeFd[0]);
        close(receivePipeFd[1]);
        exit(EXIT_SUCCESS);
    }
    return 0;
}

int  main(int argc, char * argv[]){
    if(argc <= 1){
        printf("Formato esperado:   ./md5 <nombre_archivo1> <nombre_archivo_n>\n");
        return 1;
    }
    
    for(int i=1; i<argc; i++){

        pipeAndFork()
    }

}


