#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define MAX_CHILDREN 20

/*
 Cuando creamos 2 pipes para conectar master a slave estas estan en el indice
 childTag con este orden:
    --------    1                         0  ----------
   | Master |   ---------->---------------  |  Slave   |
   |        |   ----------<---------------  |          |
    --------    2                         3  ----------
*/
#define SLAVE_READ_END 0
#define MASTER_WRITE_END 1
#define MASTER_READ_END 2
#define SLAVE_WRITE_END 3

void pipeAndFork(int fileNum, char *files[]);
int processedCount = 0;

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf(
            "Formato esperado:   ./master <nombre_archivo1> "
            "<nombre_archivo_n>\n");
        return 1;
    }
    pipeAndFork(argc - 1, argv + 1);
}

void pipeAndFork(int fileNum, char *files[]) {
    int processedCount = 0;
    int maxFd = 0;

    // TO=DO IMPORTANTE ES HACER BIEN FILE COUNT, AHORA ESTA HARDCODEADO PARA
    // HACERME LA VIDA FACIL

    // Puede ser un one liner pero esto es mas legible
    int childCount = fileNum * 0.1 + 1;  // -1 para sacar el ./master

    // int childCount = 3;
    childCount = (childCount > 20) ? 20 : childCount;

    // ese *4 esta porq cada hijo va a tener 2 pipes -> 4fds
    // El orden siempre va a ser masterRead, masterWrite, slaveRead, slaveWrite
    int fileDescriptors[childCount * 4];

    // creo childCount procesos y a cada uno inicialmente le voy pasando 2
    // archivos
    for (int i = 0; i < childCount; i++) {
        int childTag = i * 4;

        if (pipe(&fileDescriptors[childTag]) == -1) {
            perror("send pipe");
            exit(EXIT_FAILURE);
        }

        if (pipe(&fileDescriptors[childTag + 2]) == -1) {
            perror("receive pipe");
            exit(EXIT_FAILURE);
        }

        maxFd = (fileDescriptors[childTag + MASTER_READ_END] > maxFd)
                    ? fileDescriptors[childCount + MASTER_READ_END]
                    : maxFd;

        pid_t cpid;
        int status;

        cpid = fork();
        if (cpid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        else if (cpid == 0) {
            // Primero cierro los que no se usan en el child que son mater read
            // y master write
            close(fileDescriptors[childTag + MASTER_READ_END]);
            close(fileDescriptors[childTag + MASTER_WRITE_END]);

            // Vamos a hacer un execve para llamar el slave
            // Notar que un conjunto vacio por completo rompe execv
            char *arges[] = {NULL};

            if (dup2(fileDescriptors[childTag + SLAVE_READ_END], STDIN_FILENO) <
                    0 ||
                dup2(fileDescriptors[childTag + SLAVE_WRITE_END],
                     STDOUT_FILENO) < 0) {
                perror("dup");
                exit(EXIT_FAILURE);
            }

            // Ejecutamos el slave process
            if (execv("./slave", arges) == -1) {
                perror("execve");
                exit(EXIT_FAILURE);
            }
        } else {
            // Parent process
            // Wait for the child process to finish
            char buffer[BUFFER_SIZE] = {0};
            ssize_t bytesRead;

            // Ahora cierro los que no se usan en el master que son slave read y
            // slave write

            close(fileDescriptors[childTag + SLAVE_READ_END]);
            close(fileDescriptors[childTag + SLAVE_WRITE_END]);

            if (write(fileDescriptors[childTag + MASTER_WRITE_END],
                      files[processedCount],
                      strlen(files[processedCount])) == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }

            processedCount++;

            /*
            close(fileDescriptors[childTag + MASTER_WRITE_END]);

            waitpid(cpid, &status, 0);

            // Nos fijamos que el child haya cortado bien
            if (WIFEXITED(status)) {
                bytesRead = read(fileDescriptors[childTag + MASTER_READ_END],
                                 buffer, BUFFER_SIZE);
                puts(buffer);
                printf("Child process exited with status %d\n",
                       WEXITSTATUS(status));
            } else {
                printf("Child process exited abnormally\n");
            }
            */
        }
    }

    fd_set readFds;
    while (processedCount < fileNum) {
        /*
            -FD_ZERO
            -FD_SET
            -select()
            -For(int i < ans)
                + leo
                + escribo
        */

        // Setteamos el set a zero
        FD_ZERO(&readFds);

        // Iteramos por los children metiendo todos los fds
        for (int i = 0; i < childCount; i++) {
            FD_SET(fileDescriptors[i * 4 + MASTER_READ_END], &readFds);
        }

        // Llamamos a select
        int numReadyFds = select(maxFd + 1, &readFds, NULL, NULL, NULL);

        if (numReadyFds == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        } else {
            for (int n = 0; n < childCount; n++) {
                if (FD_ISSET(fileDescriptors[n + MASTER_READ_END], &readFds)) {
                    // Read data from stdin
                    char returnBuffer[1024] = {0};
                    ssize_t readBytes =
                        read(fileDescriptors[n + MASTER_READ_END], returnBuffer, sizeof(returnBuffer));
                    if (readBytes == -1) {
                        perror("read");
                        exit(EXIT_FAILURE);
                    } else if (readBytes == 0) {
                        printf("End of file encountered.\n");
                    } else {
                        // Process the data
                        returnBuffer[readBytes] = '\0';
                        printf("Read %zd bytes: %s\n", readBytes, returnBuffer);
                    }

                    processedCount++;
                    write(fileDescriptors[n + MASTER_WRITE_END], files[processedCount], strlen(files[processedCount]));

                }

            }
        }
    }

    return;
}
