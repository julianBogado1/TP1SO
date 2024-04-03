// Nota curiosa, la linea del print(bytesRead leidos: buffer) tiene un bug curioso de que si no imprime cuantos bytes imprimio imprime basura al final del buffer
// Lo raro de esto es que si los separo en dos prints tambien funciona bien salvo si no printeo el bytesRead
// No es necesario solucionarlo (mas que nada porque no vamos a estar usandolo) pero era intereante

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
            // strcpy(filename, buffer);
            // sprintf(command, "md5sum %s\n", filename);

            // Validamos en caso de un filename demasiado largo 
            if(bytesRead >= BUFFER_SIZE){
                perror("invalid filename");
                exit(EXIT_FAILURE);
            }

            buffer[bytesRead] = '\0'; // le agrego el null term q write no manda
            write(STDOUT_FILENO, buffer, strlen(buffer));
            // FILE *md5Command = popen(command, "r");
            // if (md5Command == NULL){
            //     perror("popen");
            //     exit(EXIT_FAILURE);
            // }
        }
    }
    return 0;
}
