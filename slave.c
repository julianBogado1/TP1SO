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

    ssize_t bytes_read;

    while ((bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE)) != 0) {
        if (bytes_read == -1) {
            perror("Error reading from fd");
            exit(EXIT_FAILURE);
        } else {
            // Validate in case of a filename bigger than BUFFER_SIZE
            if (bytes_read >= BUFFER_SIZE) {
                perror("invalid filename");
                exit(EXIT_FAILURE);
            }

            buffer[bytes_read] = '\0';  // We add null term

            // We run the md5hash command and save it to the buffer
            sprintf(command, "md5sum %s", buffer);
            FILE* md5_command = popen(command, "r");
            if (md5_command == NULL) {
                perror("popen");
                exit(EXIT_FAILURE);
            }

            // Storing into buffer the hash and filename
            fgets(buffer, sizeof(buffer), md5_command);
            pclose(md5_command);

            // Truncate the buffer to remove the newline char
            buffer[strlen(buffer) - 1] = 0;

            pid_t slave_pid = getpid();
            // The format we want is hash, filename, slave pid
            sprintf(command, "%s %d\n", buffer, slave_pid);

            write(STDOUT_FILENO, command, strlen(command));
        }
    }
    return 0;
}
