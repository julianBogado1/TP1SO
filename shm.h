#ifndef SHM_H
#define SHM_H

/**
*@brief     Creates a shared memory by name, with specific size.
*@param[in] shm_name Name for the shm.
*@param[in] size Desired size for the shm.
*@return    File descriptor of the shm.
*@note      The shm_name has to start with a "/".
*/
int create_shm(char *shm_name, size_t size) {
    int oflag = O_CREAT | O_RDWR | O_EXCL;
    mode_t mode = 0777;  // all permissions
    int to_ret = shm_open(shm_name, oflag, mode);
    if (to_ret == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    // now we assign the desired size
    if (ftruncate(to_ret, size) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }
    return to_ret;
}

/**
*@brief     Opens an existing shared memory by name as read only.
*@param[in] shm_name Name of the shm.
*@return    File descriptor of the shm.
*/
int open_ro_shm(char *shm_name) {
    int oflag = O_RDONLY ;
    mode_t mode = 0444;  // read only
    int to_ret = shm_open(shm_name, oflag, mode);
    if (to_ret == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    return to_ret;
}

/**
*@brief     Maps desired shared memory to a virtual address for the process to use as read only.
*@param[in] size Size of the shm.
*@param[in] fd File descriptor of the shared memory.
*@return    Pointer to the start of the shared memory of chars.
*@note      Needs an existing shm file descriptor.
*/
char *map_ro_shm(size_t size, int fd) {
    int prot = PROT_READ;
    int flags = MAP_SHARED;
    char *to_ret =  (char *) mmap(NULL, size, prot, flags, fd, 0);
    if (to_ret == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    return to_ret;
}

/**
*@brief     Maps desired shared memory to a virtual address for the process to use as read and write.
*@param[in] size Size of the shm.
*@param[in] fd File descriptor of the shared memory.
*@return    Pointer to the start of the shared memory of chars.
*@note      Needs an existing shm file descriptor.
*/
char *map_rw_shm(size_t size, int fd) {
    int prot = PROT_READ | PROT_WRITE;
    int flags = MAP_SHARED;
    char *to_ret =  (char *) mmap(NULL, size, prot, flags, fd, 0);  // NULL since we dont have a specific one in mind (but the kernel decides
                                                                    // anyway). 0 since no offset.
    if (to_ret == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    return to_ret;
}

/**
*@brief     Downs operation on a desired semaphore.
*@param[in] sem Chosen semaphore.
*/
void down(sem_t *sem) {
    if (sem_wait(sem) == -1){
        perror("sem_wait");
        exit(EXIT_FAILURE);
   }
}

/**
*@brief     Up operation on a desired semaphore.
*@param[in] sem Chosen semaphore.
*/
void up(sem_t *sem) { 
   if (sem_post(sem) == -1){
        perror("sem_post");
        exit(EXIT_FAILURE);
   }
}

#endif //SHM_H