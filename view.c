#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>

#define STDIN_FD 0
#define SHM_NAME_LEN 256
#define SHM_SIZE 1024

#define SHM_PARAMETER 1
#define SHM_STDIN 2

char * open_and_map_shm(char *shm_name, size_t size);

int main(int argc, char *argv[]){
	//lets search shm_name first
	fd_set rfds;
	struct timeval tv;
	tv.tv_sec=1;
	tv.tv_usec=0;
	FD_ZERO(&rfds);
	FD_SET(STDIN_FD, &rfds);

	char shm_name[SHM_NAME_LEN];

	char input_type = (argc==2)? SHM_PARAMETER:0;
	input_type += select(STDIN_FD + 1, &rfds, NULL, NULL, &tv)? SHM_STDIN:0;

	switch(input_type){
		case SHM_PARAMETER:
				strcpy(shm_name,argv[1]);
			break;
		case SHM_STDIN:
				int bytes_read = read(STDIN_FD, shm_name, SHM_NAME_LEN);
				shm_name[bytes_read]='\0';
			break;
		default:
			printf("manda bien la shm, pelele\n");
			printf("Expected: '<info> | ./view', or './view <info>'\n");
			return 0;
	}

	//lets open it and map it here
	char * memaddr = open_and_map_shm(shm_name, SHM_SIZE);

	//now the actual view process
	int read_flag=1;
	int idx=0;
	while(read_flag){
		//down(toread)
		//down(mutex)
		int flag = *(memaddr+idx)!=0? 1:0;
		if(flag){
			int length=printf("%s",memaddr+idx);
			idx+=length;
		}
		//up(mutex)
	}

	//lets say goodbye now!

	return 0;
}

char * open_and_map_shm(char *shm_name, size_t size){
	int oflag = O_RDONLY;
	mode_t mode = 0444;//read only
	int fd = shm_open(shm_name, oflag, mode);

	int prot = PROT_READ;
	int flags = MAP_SHARED;
	return (char *) mmap(NULL,size,prot,flags,fd,0);
}