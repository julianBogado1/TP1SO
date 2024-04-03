// TODO ERR return checks!!!!!!!!!!!!!
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define STDIN_FD 0
#define SHM_NAME_LEN 256
#define SHM_SIZE 1024
#define BUFFER_SIZE 1024

#define SHM_PARAMETER 1
#define SHM_STDIN 2

char *open_and_map_shm(char *shm_name, size_t size);
int get_sem(char *mem, sem_t *mutex, sem_t *toread);
void down(sem_t *sem);
void up(sem_t *sem);

int main(int argc, char *argv[]) {
    // lets search shm_name first
    fd_set rfds;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(STDIN_FD, &rfds);

    char shm_name[SHM_NAME_LEN] = {0};

    char input_type = (argc == 2) ? SHM_PARAMETER : 0;
    input_type += select(STDIN_FD + 1, &rfds, NULL, NULL, &tv) ? SHM_STDIN : 0;

    switch (input_type) {
        case SHM_PARAMETER:
            strcpy(shm_name, argv[1]);
            break;
        case SHM_STDIN:
            int bytes_read = read(STDIN_FD, shm_name, SHM_NAME_LEN);
            shm_name[bytes_read] = '\0';
            break;
        default:
            printf("manda bien la shm, pelele\n");
            printf("Expected: '<info> | ./view', or './view <info>'\n");
            return 0;
    }

    // lets open it and map it here
    char *memaddr = open_and_map_shm(shm_name, SHM_SIZE);

    // semaphores
    char mutex_path[BUFFER_SIZE] = {0};
    char toread_path[BUFFER_SIZE] = {0};

    int i = 0, j = 0;

    while (memaddr[i] != '\0') {
        mutex_path[i] = memaddr[i];
        i++;
    }
    mutex_path[++i] = '\0';

    while (memaddr[i + j] != '\0') {
        toread_path[j] = memaddr[i + j];
        j++;
    }
    toread_path[++j] = '\0';

    int idx = i + j;

    sem_t *mutex = sem_open(mutex_path, 0);
    sem_t *toread = sem_open(toread_path, 0);

    // now the actual view process
    while (1) {  // this should change though
        down(toread);
        down(mutex);
        int length = printf("%s", memaddr + idx);
        up(mutex);
        idx += length + 1;
    }

    // lets say goodbye now!
    munmap(shm_name, SHM_SIZE);

    shm_unlink(shm_name);
    sem_unlink(mutex_path);
    sem_unlink(toread_path);

    return 0;
}

char *open_and_map_shm(char *shm_name, size_t size) {
    int oflag = O_RDONLY;
    mode_t mode = 0444;  // read only
    // printf("\n[openandmap] shm_name:%s\n", shm_name);
    int fd = shm_open(shm_name, oflag, mode);
    // printf("\n[openandmap] fd:%d\n", fd);

    int prot = PROT_READ;
    int flags = MAP_SHARED;
    return (char *)mmap(NULL, size, prot, flags, fd, 0);
}

void down(sem_t *sem) { sem_wait(sem); }

void up(sem_t *sem) { sem_post(sem); }