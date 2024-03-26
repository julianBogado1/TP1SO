#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

// este master genera en total 10 esclavos --> mejorable

void pipeAndFork(int argc, char *argv[]);

char *files[] = {"md5.c", "master.c", "slave.c"};

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf(
            "Formato esperado:   ./master <nombre_archivo1> "
            "<nombre_archivo_n>\n");
        return 1;
    }
    pipeAndFork(argc, argv);
}

void pipeAndFork(int argc, char *argv[]) {
    // PipeFd[0] es el read end y [1] es el write end
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

    pid_t cpid;
    int status;

    cpid = fork();
    if (cpid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (cpid == 0) {
        // Primero cierro los que no se usan en el slave
        close(sendPipeFd[1]);
        close(receivePipeFd[0]);

        // Vamos a hacer un execve para llamar el slave
        // Notar que un conjunto vacio totalmente rompe execv
        char *arges[] = {NULL};

        if (dup2(sendPipeFd[0], STDIN_FILENO) < 0 ||
            dup2(receivePipeFd[1], STDOUT_FILENO) < 0) {
            perror("dup");
            exit(EXIT_FAILURE);
        }

        // Execute the slave process
        if (execv("./slave", arges) == -1) {
            perror("execve");
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        // Wait for the child process to finish
        char buffer[BUFFER_SIZE];
        ssize_t bytesRead;

        // Primero cierro los que no se usan en el master
        close(sendPipeFd[0]);
        close(receivePipeFd[1]);

        // esto de aca esta mal, es un fake select q espera a q el otro proceso
        // termine porq sabe q ahi va a tener para leer
        for (int i = 0; i < 3; i++){
            write(sendPipeFd[1], files[i], strlen(files[i]));
            sleep(2);
        }

        close(sendPipeFd[1]);

        waitpid(cpid, &status, 0);

        // Check if the child process exited normally
        if (WIFEXITED(status)) {
            bytesRead = read(receivePipeFd[0], buffer, BUFFER_SIZE);
            printf("%s\n", buffer);
            printf("Child process exited with status %d\n",
                   WEXITSTATUS(status));
        } else {
            printf("Child process exited abnormally\n");
        }
    }
    return;
}
