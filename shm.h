#ifndef SHM_H
#define SHM_H

/**
*@brief     Opens an existing shared memory by name as read only.
*@param[in] shm_name Name of the shm.
*@return    File descriptor of the shm.
*/
int open_ro_shm(char *shm_name) {
    int oflag = O_RDONLY;
    mode_t mode = 0444;  // read only
    return shm_open(shm_name, oflag, mode);
}

/**
*@brief     Maps desired shared memory to a virtual address for the process to use.
*@param[in] size Size of the shm.
*@param[in] fd File descriptor of the shared memory.
*@return    Pointer to the start of the shared memory of chars.
*@note      Needs an existing shm file descriptor.
*/
char *map_shm(size_t size, int fd) {
    int prot = PROT_READ;
    int flags = MAP_SHARED;
    return (char *) mmap(NULL, size, prot, flags, fd, 0);
}

/**
*@brief     Downs operation on a desired semaphore.
*@param[in] sem Chosen semaphore.
*/
void down(sem_t *sem) { sem_wait(sem); }

/**
*@brief     Up operation on a desired semaphore.
*@param[in] sem Chosen semaphore.
*/
void up(sem_t *sem) { 
   if (sem_post(sem) == -1){
    
   }
}

#endif //SHM_H