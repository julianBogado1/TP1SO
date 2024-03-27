// Nota curiosa, la linea del print(bytesRead leidos: buffer) tiene un bug curioso de que si no imprime cuantos bytes imprimio imprime basura al final del buffer
// Lo raro de esto es que si los separo en dos prints tambien funciona bien salvo si no printeo el bytesRead
// No es necesario solucionarlo (mas que nada porque no vamos a estar usandolo) pero era intereante

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    char buffer[BUFFER_SIZE]={0};
    char command[BUFFER_SIZE]={0};
    char md5[BUFFER_SIZE]={0};
    char filename[BUFFER_SIZE]={0};

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
            fprintf(stderr, "%s y buffer es de tamaño: %d\n", buffer, bytesRead);
            sprintf(command, "md5sum %s", buffer);
            FILE *pipe = popen(command, "r"); // ejecuta el comando "command" con el argumento recibido por consola en slave
            if (pipe == NULL){
                perror("popen");
                return 1;
            }

            fscanf(pipe, "%s%s", md5, filename);
            pclose(pipe);
            printf("%s %s %d", md5, filename, getpid());
        }
    }
    return 0;
}
