// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
// TODO:
//      - variables del shm y sempahore gloales suena feo

#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "shm.h"

#define BUFFER_SIZE 1024
#define MAX_CHILDREN 20

/*
 Whenever we create a two pipes to connect a master/slave pair we store it in
 the file_descriptors arrays at child_tag (an index) and the four are stored
 in this order:
    --------    1                         0  ----------
   | Master |   ---------->---------------  |  Slave   |
   |        |   ----------<---------------  |          |
    --------    2                         3  ----------
*/
#define SLAVE_READ_END 0
#define MASTER_WRITE_END 1
#define MASTER_READ_END 2
#define SLAVE_WRITE_END 3

#define SHM_SIZE 1024

void pipe_and_fork(int file_num, char *files[]);
void write_result_file();

// TODO que no sea global ta feo
char *memaddr;  // Pointer to shm
int shmdx = 0;
int shm_info_idx = 0;
sem_t *toread;  // semafore to read

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf(
            "Expected:   ./master <filename_1> "
            "<filename_n>\n");
        return 1;
    }

    // Lets create the shared mem!
    char *shm_name = "/shamone";

    //shm_unlink(shm_name);//Just to make sure it doesnt already exist

    int shm_fd = create_shm(shm_name, SHM_SIZE);

    memaddr = map_rw_shm(SHM_SIZE, shm_fd);

    // Semaphores
    char *toread_path = "/toread_sem";

    //Just to make sure they dont already exist
    sem_unlink(toread_path);

    //Share the semaphores by copy in shm
    memcpy(memaddr + shmdx, toread_path, strlen(toread_path) + 1);  //+1 to preserve the null terminated
    shmdx += strlen(toread_path) + 1;

    shm_info_idx = shmdx;

    toread = sem_open(toread_path, O_CREAT, 0777, 0);
    if ((toread) == SEM_FAILED){
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    // Pass the shm_name to STDOUT
    write(STDOUT_FILENO, shm_name, strlen(shm_name));
    sleep(2); //time for view process

    pipe_and_fork(argc - 1, argv + 1);

    //Save -1 as end of file
    int shm_end = -1;
    memcpy(memaddr + shmdx, &shm_end, sizeof(int));

    //Now we save the results
    write_result_file();
    
    // Lets unmap and close the shms
    if (munmap(memaddr, SHM_SIZE) == -1){
        perror("munmap");
        exit(EXIT_FAILURE);
    }
    
    if (close(shm_fd) == -1){
        perror("close");
        exit(EXIT_FAILURE);
    }
    shm_unlink(shm_name);
    
    if (sem_close(toread) == -1){
        perror("sem_close");
        exit(EXIT_FAILURE);
    }

    sem_unlink(toread_path);
}

void pipe_and_fork(int file_num, char *arg_files[]) {
    char **files = arg_files;
    int sent_count = 0;
    int read_count = 0;
    int max_fd = 0;

    // We set how many children we are going to create
    int child_count = file_num * 0.1 + 1;
    child_count = (child_count > 20) ? 20 : child_count;

    // We are going to have 4 fds for each child
    // Order is defined in the top comment with the defines
    int file_descriptors[child_count * 4];

    // We create child_count child processes
    for (int i = 0; i < child_count; i++) {
        int child_tag = i * 4;

        // Send and recieve pipes
        if (pipe(&file_descriptors[child_tag]) == -1) {
            perror("send pipe");
            exit(EXIT_FAILURE);
        }

        if (pipe(&file_descriptors[child_tag + 2]) == -1) {
            perror("receive pipe");
            exit(EXIT_FAILURE);
        }

        // Each time we create new pipes we check if they are the max_fd (this
        // is for the select call later on)
        max_fd = (file_descriptors[child_tag + MASTER_READ_END] > max_fd)
                     ? file_descriptors[child_count + MASTER_READ_END]
                     : max_fd;

        pid_t cpid;

        // Fork!
        cpid = fork();
        if (cpid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        else if (cpid == 0) {
            // We close the fds we aren't using in the slave, these are the
            // master read/write ends
            close(file_descriptors[child_tag + MASTER_WRITE_END]);
            close(file_descriptors[child_tag + MASTER_READ_END]);

            // The dup2 function will make communicating with the child
            // processes way easier since we avoid having to send the fds of the
            // master pipe
            if (dup2(file_descriptors[child_tag + SLAVE_READ_END],
                     STDIN_FILENO) < 0 ||
                dup2(file_descriptors[child_tag + SLAVE_WRITE_END],
                     STDOUT_FILENO) < 0) {
                perror("dup");
                exit(EXIT_FAILURE);
            }

            // After changing them with dup2 we close the old fds since we now
            // only use STDIN and STDOUT
            close(file_descriptors[child_tag + SLAVE_READ_END]);
            close(file_descriptors[child_tag + SLAVE_WRITE_END]);
            
            // Params for the execv call
            char *arges[] = {"./slave", NULL};

            // Execv here to change into the slave process
            if (execv("./slave", arges) == -1) {
                perror("execve");
                exit(EXIT_FAILURE);
            }

        } else {
            // Same as in the parent, we close the unused fds which are now the
            // slave read/write
            close(file_descriptors[child_tag + SLAVE_READ_END]);
            close(file_descriptors[child_tag + SLAVE_WRITE_END]);

            // We initially send each child a file so as to have them running
            // asap
            if (write(file_descriptors[child_tag + MASTER_WRITE_END],
                      files[sent_count], strlen(files[sent_count])) == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            
            sent_count++;
        }
    }

    fd_set readFds;

    // Now comes the main while loop
    while (read_count < file_num) {
        // We set the set to zero
        FD_ZERO(&readFds);

        // We set every master read end in the set
        for (int i = 0; i < child_count; i++) {
            FD_SET(file_descriptors[i * 4 + MASTER_READ_END], &readFds);
        }

        // Select call
        int num_ready_fds = select(max_fd + 1, &readFds, NULL, NULL, NULL);

        if (num_ready_fds == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        } else {
            // We iterate over every child's master read end to see which are
            // available to read
            for (int n = 0; n < child_count; n++) {
                // This checks if the fd is readable
                if (FD_ISSET(file_descriptors[n * 4 + MASTER_READ_END],
                             &readFds)) {
                    char return_buffer[1024] = {0};
                    // If it is, we try and read it
                    ssize_t read_bytes =
                        read(file_descriptors[n * 4 + MASTER_READ_END],
                             return_buffer, sizeof(return_buffer));
                    if (read_bytes == -1) {
                        perror("read");
                        exit(EXIT_FAILURE);
                    } else if (read_bytes == 0) {
                        printf("End of file encountered.\n");
                    } else {
                        // If we got data, we send it to the shm. The slave
                        // process is in charge of formatting the data correctly
                        memcpy(memaddr + shmdx, return_buffer, read_bytes);

                        // The +1 is for the null term
                        shmdx += read_bytes + 1;
                        up(toread);
                        
                        read_count++;

                        // If we have any remaining files, we sent it over to the
                        // slave that was just freed
                        if (sent_count < file_num) {
                            write(file_descriptors[n * 4 + MASTER_WRITE_END],
                                files[sent_count], strlen(files[sent_count]));
                            sent_count++;
                        }
                    }
                }
            }
        }
    }

    for(int i = 0; i < child_count; i++) {
        close(file_descriptors[i*4 + MASTER_WRITE_END]);
    }
    
    return;
}

void write_result_file() {
    char *file_mode = "w";

    FILE * file = fopen("md5res.txt", file_mode);
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int aux_idx=shm_info_idx;

    while (*(memaddr+aux_idx)!=-1) {  //until end of shm
        int length = fprintf(file, "%s", memaddr + aux_idx);
        if (length < 0) {
            perror("fprintf");
            exit(EXIT_FAILURE);
        }
        aux_idx += length + 1;
    }

    if (fclose(file) == EOF) {
        perror("fclose");
        exit(EXIT_FAILURE);
    }
}