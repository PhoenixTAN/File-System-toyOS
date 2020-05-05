#include "userspace.h"

int fd;
int ret;

int rd_init() {
    ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
    memset(args, 0, sizeof(ioctl_args_t));

    fd = open("/proc/discos", O_RDWR);
    ret = ioctl(fd, RD_INIT, args);
    free(args);
    return ret;
}

int rd_creat(char* pathname) {
    ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
    memset(args, 0, sizeof(ioctl_args_t));
    args->pathname = pathname;
    ret = ioctl(fd, RD_CREATE, args);
    free(args);
    return ret;
}

int rd_unlink(char* pathname) {
    ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
    memset(args, 0, sizeof(ioctl_args_t));
    args->pathname = pathname;
    ret = ioctl(fd, RD_UNLINK, args);
    free(args);
    return ret;
}

int rd_mkdir(char* pathname) {
    ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
    memset(args, 0, sizeof(ioctl_args_t));
    args->pathname = pathname;
    ret = ioctl(fd, RD_MKDIR, args);
    free(args);
    return ret;
}

