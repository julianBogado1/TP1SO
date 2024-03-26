#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    while((bytesRead = read(STDIN_FILENO, buffer, BUFFER_SIZE)) != 0){
        if (bytesRead == -1) {
            perror("Error reading from fd");
            exit(EXIT_FAILURE);
        }
        else {
            // Print the read data
            printf("Received: %.*s\n", (int)bytesRead, buffer);
        }
    }

        

    return 0;
}
