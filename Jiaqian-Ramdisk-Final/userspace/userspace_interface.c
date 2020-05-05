#include "userspace.h"
static int fd;

int rd_init() {
    int ret;
    ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
    memset(args, 0, sizeof(ioctl_args_t));

    fd = open("/proc/discos", O_RDWR);
    ret = ioctl(fd, RD_INIT, args);
    free(args);
    return ret;
}

int rd_creat(char* pathname, unsigned int mode) {
    int ret;
    ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
    memset(args, 0, sizeof(ioctl_args_t));
    args->pathname = pathname;
    args->mode = mode;
    ret = ioctl(fd, RD_CREATE, args);
    free(args);
    return ret;
}

int rd_unlink(char* pathname) {
    int ret;
    ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
    memset(args, 0, sizeof(ioctl_args_t));
    args->pathname = pathname;
    ret = ioctl(fd, RD_UNLINK, args);
    free(args);
    return ret;
}

int rd_mkdir(char* pathname) {
    int ret;
    ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
    memset(args, 0, sizeof(ioctl_args_t));
    args->pathname = pathname;
    ret = ioctl(fd, RD_MKDIR, args);
    free(args);
    return ret;
}

int rd_open(char* pathname, unsigned int mode) {
    int ret;
    ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
    memset(args, 0, sizeof(ioctl_args_t));
    args->pathname = pathname;
    args->mode = mode;
    args->pid = (int)getpid();
    printf("<> rd_open: args->pid=%d\n", args->pid);
    ret = ioctl(fd, RD_OPEN, args);
    free(args);
    return ret;
}

int rd_write(int _fd, char* data, int number_of_data) {
    int ret;
    ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
    memset(args, 0, sizeof(ioctl_args_t));
    args->pid = (int)getpid();
    args->fd = _fd;
    args->size = number_of_data;
    args->data = data;
    printf("userspace write\n");
    ret = ioctl(fd, RD_WRITE, args);
    printf("userspace ret: %d\n", ret);
    free(args);
    return ret;
}

int rd_chmod(char* pathname, unsigned int mode) {
    int ret;
    ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
    memset(args, 0, sizeof(ioctl_args_t));
    args->mode = mode;
    args->pathname = pathname;
    args->pid = (int)getpid();
    ret = ioctl(fd, RD_CHMOD, args);
    free(args);
    return ret;
}

int rd_lseek(int _fd, int offset) {
    int ret;
    ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
    memset(args, 0, sizeof(ioctl_args_t));
    args->fd = _fd;
    args->size = offset;
    args->pid = (int)getpid();
    ret = ioctl(fd, RD_LSEEK, args);
    free(args);
    return ret;
}

int rd_read(int _fd, char* addr, int number_of_data) {
    int ret;
    ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
    memset(args, 0, sizeof(ioctl_args_t));
    args->fd = _fd;
    args->data = addr;
    args->size = number_of_data;
    args->pid = (int)getpid();
    ret = ioctl(fd, RD_READ, args);
    free(args);
    return ret;
}

int rd_close(int _fd) {
    int ret;
    ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
    memset(args, 0, sizeof(ioctl_args_t));
    args->fd = _fd;
    args->pid = (int)getpid();
    ret = ioctl(fd, RD_CLOSE, args);
    free(args);
    return ret;
}


