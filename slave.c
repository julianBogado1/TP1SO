// Nota curiosa, la linea del print(bytesRead leidos: buffer) tiene un bug
// curioso de que si no imprime cuantos bytes imprimio imprime basura al final
// del buffer Lo raro de esto es que si los separo en dos prints tambien
// funciona bien salvo si no printeo el bytesRead No es necesario solucionarlo
// (mas que nada porque no vamos a estar usandolo) pero era intereante

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 512
#define COMMAND_SIZE 1024
#define MD5_SIZE 32

int main(int argc, char* argv[]) {
    char buffer[BUFFER_SIZE];
    char command[COMMAND_SIZE];

    ssize_t bytesRead;

    while ((bytesRead = read(STDIN_FILENO, buffer, BUFFER_SIZE)) != 0) {
        if (bytesRead == -1) {
            perror("Error reading from fd");
            exit(EXIT_FAILURE);
        } else {
            // Validamos en caso de un filename demasiado largo
            if (bytesRead >= BUFFER_SIZE) {
                perror("invalid filename");
                exit(EXIT_FAILURE);
            }

            buffer[bytesRead] =
                '\0';  // le agrego el null term q write no manda

            // nombre md5 pid nullTerm
            sprintf(command, "md5sum %s", buffer);
            FILE* md5Command = popen(command, "r");
            if (md5Command == NULL) {
                perror("popen");
                exit(EXIT_FAILURE);
            }

            fgets(buffer, sizeof(buffer), md5Command);
            pclose(md5Command);

            // le saco el \n de buffer
            buffer[strlen(buffer) - 1] = 0;

            pid_t slavePid = getpid();
            sprintf(command, "%s %d\n", buffer, slavePid);

            write(STDOUT_FILENO, command, strlen(command));
        }
    }
    return 0;
}
