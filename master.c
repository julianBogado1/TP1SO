#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>

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

#define SHM_SIZE 1024

void pipeAndFork(int fileNum, char *files[]);

int create_shm(char *shm_name, size_t size);

void down(sem_t * sem);
void up(sem_t * sem);

//TODO que no sea global ta feo
char *memaddr;//puntero a la sharemem
int shm_idx=0;
sem_t *mutex;//semaforo mutex
sem_t *toread;//semaforo lector

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf(
            "Formato esperado:   ./master <nombre_archivo1> "
            "<nombre_archivo_n>\n");
        return 1;
    }

    //lets create the shared mem!
    char *shm_name = "/myshm";
    int prot = PROT_READ | PROT_WRITE;
	int flags = MAP_SHARED;

    //just to make sure it doesnt already exist
    shm_unlink(shm_name);

    int shm_fd = create_shm(shm_name, SHM_SIZE);
	if(shm_fd==-1){
		printf("%s shared memory failed\n", shm_name);
		return 0;
	}

    //now we assign address!
	//NULL since we dont have a specific one in mind (but the kernel decides anyway)
	//0 since no offset
    memaddr = (char *) mmap(NULL,SHM_SIZE,prot,flags,shm_fd,0);

    //semaphores!
    char *mutex_path = "/mutex_sem";
    char *toread_path = "/toread_sem";

    //just to make sure they arent still there
    sem_unlink(mutex_path);
    sem_unlink(toread_path);

    //copy in shm
    memcpy(memaddr+shm_idx, mutex_path, strlen(mutex_path));
    shm_idx+=strlen(mutex_path);
    *(memaddr+shm_idx)='\0';
    shm_idx++;
    memcpy(memaddr+shm_idx, toread_path, strlen(toread_path));
    shm_idx+=strlen(toread_path);
    *(memaddr+shm_idx)='\0';
    shm_idx++;

    mutex = sem_open(mutex_path, O_CREAT, 0777, 1);
    toread = sem_open(toread_path, O_CREAT, 0777, 0);

    //pass the shm_name to STDOUT
    write(STDOUT_FILENO,shm_name,strlen(shm_name));

    pipeAndFork(argc - 1, argv + 1);

    //lets unmap the shm
    munmap(shm_name,SHM_SIZE);

    //lets close it (and the semaphores!!)
    close(shm_fd);
    sem_unlink(mutex_path);
    sem_unlink(toread_path);
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
                        // returnBuffer[readBytes] = '\0';
                        //printf("%s\n", returnBuffer);//ACAAAAAAAAAA
                        down(mutex);
                        memcpy(memaddr+shm_idx, returnBuffer, readBytes);
                        shm_idx+=readBytes+1;
                        up(mutex);
                        up(toread);
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

void down(sem_t * sem){
    sem_wait(sem);
}

void up(sem_t * sem){
    sem_post(sem);
}

//shm_name should be /somename
int create_shm(char *shm_name, size_t size){
	int oflag = O_CREAT | O_RDWR;
	mode_t mode = 0777; //all permissions
	//now we create!
	int fd = shm_open(shm_name, oflag, mode);
	if(fd==-1){
	       printf("Open failed\n");
	       return -1;
	}
	//now we assign length!
	if(ftruncate(fd, size)==-1){
		printf("ftruncate\n");
		return -1;
	}
	return fd;
}