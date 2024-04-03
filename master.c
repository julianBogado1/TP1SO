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

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf(
            "Formato esperado:   ./master <nombre_archivo1> "
            "<nombre_archivo_n>\n");
        return 1;
    }
    pipeAndFork(argc - 1, argv + 1);
}

void pipeAndFork(int fileNum, char *arg_files[]) {
    char **files = arg_files;
    int sentCount = 0;
    int readCount = 0;
    int maxFd = 0;

    // Puede ser un one liner pero esto es mas legible
    int childCount = fileNum * 0.1 + 1;  // -1 para sacar el ./master
    childCount = (childCount > 20) ? 20 : childCount;

    // ese *4 esta porq cada hijo va a tener 2 pipes -> 4fds
    // El orden siempre va a ser masterRead, masterWrite, slaveRead, slaveWrite
    int fileDescriptors[childCount * 4];

    // creo childCount procesos y a cada uno inicialmente le voy pasando un
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

        cpid = fork();
        if (cpid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        else if (cpid == 0) {
            // Primero cierro los que no se usan en el child que son mater read
            // y master write
            close(fileDescriptors[childTag + MASTER_WRITE_END]);
            close(fileDescriptors[childTag + MASTER_READ_END]);

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

            // Cerramos estos fds que ya no necesitamos despues de usar dup2
            close(fileDescriptors[childTag + SLAVE_READ_END]);
            close(fileDescriptors[childTag + SLAVE_WRITE_END]);

            // Ejecutamos el slave process
            if (execv("./slave", arges) == -1) {
                perror("execve");
                exit(EXIT_FAILURE);
            }

        } else {

            // Ahora cierro los que no se usan en el master que son slave read y
            // slave write

            close(fileDescriptors[childTag + SLAVE_READ_END]);
            close(fileDescriptors[childTag + SLAVE_WRITE_END]);

            // printf("%d, %s\n", sentCount, files[sentCount]);

            if (write(fileDescriptors[childTag + MASTER_WRITE_END],
                      files[sentCount], strlen(files[sentCount])) == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }

            sentCount++;
        }
    }

    fd_set readFds;

    while (readCount < fileNum) {
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
                if (FD_ISSET(fileDescriptors[n * 4 + MASTER_READ_END], &readFds)) {
                    // Leemos del fd en cuestion 
                    char returnBuffer[1024] = {0};
                    ssize_t readBytes =
                        read(fileDescriptors[n * 4 + MASTER_READ_END], returnBuffer,
                             sizeof(returnBuffer));
                    if (readBytes == -1) {
                        perror("read");
                        exit(EXIT_FAILURE);
                    } else if (readBytes == 0) {
                        printf("End of file encountered.\n");
                    } else {
                        // Si nos llego info, imprimimos la data
                        returnBuffer[readBytes] = '\0';
                        printf("%s\n", returnBuffer);
                    }

                    readCount++;
                    if (sentCount < fileNum) {
                        write(fileDescriptors[n * 4 + MASTER_WRITE_END],
                              files[sentCount], strlen(files[sentCount]));
                        sentCount++;
                    }
                }
            }
        }
    }

    return;
}
