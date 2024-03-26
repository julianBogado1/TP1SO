#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    char buffer[BUFFER_SIZE];
    char command[BUFFER_SIZE];
    char filename[100];

    ssize_t bytesRead;

    while ((bytesRead = read(STDIN_FILENO, buffer, BUFFER_SIZE)) != 0) {
        if (bytesRead == -1) {
            perror("Error reading from fd");
            exit(EXIT_FAILURE);
        } else {
            // Mandamos por salida estandar el output con md5sum
            strcpy(filename, buffer);
            sprintf(command, "md5sum %s\n", filename);

            puts(command);

            // FILE *md5Command = popen(command, "r");
            // if (md5Command == NULL){
            //     perror("popen");
            //     exit(EXIT_FAILURE);
            // }
        }
    }

    return 0;
}
