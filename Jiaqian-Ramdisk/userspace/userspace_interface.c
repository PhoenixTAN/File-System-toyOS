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
    args->pid = 100; //(int)getpid();
    printf("<> rd_open: args->pid=%d\n", args->pid);
    ret = ioctl(fd, RD_OPEN, args);
    free(args);
    return ret;
}

int rd_write(int _fd, char* data, int number_of_data) {
    int ret;
    ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
    memset(args, 0, sizeof(ioctl_args_t));
    args->pid = 100; //(int)getpid();
    printf("<> rd_write: args->pid=%d\n", args->pid);

    args->fd = _fd;
    args->size = number_of_data;
    args->data = data;
    printf("userspace write\n");
    ret = 0;
    ret = ioctl(fd, RD_WRITE, args);
    /*if (ret < 0) {
        fprintf(stderr, "%s\n", explain_ioctl(fd, RDD_WRITE, args));
        exit(EXIT_FAILURE);
    }*/
    printf("userspace ret: %d\n", ret);
    free(args);
    return ret;
}
