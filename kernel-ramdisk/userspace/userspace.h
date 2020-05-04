#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/ioctl.h>

#define MAGIC_NUM 88

#define RD_INIT    _IOW(MAGIC_NUM, 0, ioctl_args_t*)
#define RD_CREATE  _IOW(MAGIC_NUM, 1, ioctl_args_t*)
#define RD_MKDIR   _IOW(MAGIC_NUM, 2, ioctl_args_t*)
#define RD_OPEN    _IOW(MAGIC_NUM, 3, ioctl_args_t*)
#define RD_CLOSE   _IOW(MAGIC_NUM, 4, ioctl_args_t*)
#define RD_READ    _IOW(MAGIC_NUM, 5, ioctl_args_t*)
#define RD_WRITE   _IOW(MAGIC_NUM, 6, ioctl_args_t*)
#define RD_LSEEK   _IOW(MAGIC_NUM, 7, ioctl_args_t*)
#define RD_UNLINK  _IOW(MAGIC_NUM, 8, ioctl_args_t*)
#define RD_READDIR _IOW(MAGIC_NUM, 9, ioctl_args_t*)

typedef struct ioctl_args {
	int size;  //size of read
	int mode;
	int pid;
	int fd;
	char* pathname;
} ioctl_args_t;

int rd_init();
int rd_creat(char* pathname);