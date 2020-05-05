#ifndef RAMDISK_H
#define RAMDISK_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/ioctl.h>
#include <linux/spinlock.h>

/* getconf ARG_MAX */
#define CMD_MAX_LENGTH          128		// 2097152 in bytes

#define RAMDISK_SIZE            1 << 21 // 2 MB
#define BLOCK_SIZE              256     // in bytes
#define INODE_SIZE              64      // in bytes
#define INODE_ARRAY_SIZE        256     // in blocks
#define BITMAP_SIZE             4       // in blocks
#define MAX_NUM_FILE            1024  
#define MAX_FILE_SIZE           1067008 // in bytes      
#define INODE_NUM_DIRECT_PTR    8
#define DATA_BLOCKS_NUM         7931   
#define THERAD_POOL_SIZE        10 
#define FD_TABLE_SIZE           1024

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
#define RD_CHMOD  _IOW(MAGIC_NUM, 10, ioctl_args_t*)

#define RD  (S_IRUSR | S_IRGRP | S_IROTH)
#define RW  (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define WR  (S_IWUSR | S_IRGRP | S_IROTH)

// typedef unsigned short uint16_t;
// typedef unsigned int uint32_t;
typedef union DATA_BLOCK data_block_struct;

typedef struct ioctl_args {
	int size;  //size of read
	int mode;
	int pid;
	int fd;
	char* pathname;
    char* data;
} ioctl_args_t;

/* Data structure for file system */

/* super block */
typedef struct SUPERBLOCK {
    unsigned int free_blocks;   // 4 bytes
    unsigned int free_inodes;   // 4 bytes
    
    char padding[248];         // 256 - 4 - 4

} superblock_struct;


/* directory entry */
typedef struct DIR_ENTRY {
	char name[14];              // filename or directory name
	unsigned short inode_num;   // the corresponding inode index 2 bytes
} dir_entry_struct;


/* A data block: dir, reg files & index blocks */
union DATA_BLOCK {
    char data[BLOCK_SIZE];
    dir_entry_struct entries[BLOCK_SIZE/sizeof(dir_entry_struct)];
    data_block_struct* index_block[BLOCK_SIZE/4];    // 4 bytes = sizeof(pointer)
};


/* i node */
typedef struct I_NODE {
    char type[4];      // either “dir” or “reg” (4 bytes)
    unsigned int size;      // current file size in bytes (4 bytes)

    data_block_struct *pointers[INODE_NUM_DIRECT_PTR];       // 8 direct block pointer
    
    data_block_struct *single_indirect_ptrs;   // 4 bytes
    data_block_struct *double_indirect_ptrs;    // 4 bytes

    unsigned int access;    // 4 bytes

    char padding[12];

} inode_struct;



/* file system */
typedef struct FILESYS {
    // 1 block
    superblock_struct superblock;

    // 256 blocks
    // 256*256/64
    inode_struct inodes[INODE_ARRAY_SIZE*BLOCK_SIZE/INODE_SIZE];

    // 4 blocks
    // 1 byte * 4 * 256 = 1024 bytes -> 1024 * 8 bit
    // 7931 blocks data blocks, 8192 blocks in all
    unsigned char bitmap[BITMAP_SIZE*BLOCK_SIZE]; 

    // 7931 blocks
    data_block_struct data_blocks[DATA_BLOCKS_NUM];

} filesys_struct;


/* file object */
typedef struct FILE_object {
    inode_struct* inode_ptr;
    unsigned int status;
    unsigned int cursor;
    unsigned int pos;
    unsigned int usable;
} file_object;


/* file descriptor table*/
typedef struct FILE_DESCRIPTOR_TABLE {
    file_object file_objects[1024];
} file_descriptor_table;


/* thread pool */
typedef struct PROCESS_FD_TABLE {
    unsigned int pid;
    file_descriptor_table fd_table;
} process_fd_table;



/* file system initialization*/
int init_file_sys();


/* helper functions */
int clear_bitmap(unsigned char* map, int index);
int set_bitmap(unsigned char* map, int index);
void parse_absolute_path(char* _path, char* _current_dir, char* _target);
int my_find_inode_number(char* pathname);
int get_free_block_num_from_bitmap(unsigned char* map);
int get_free_inode_num();
inode_struct* allocate_inode(int free_inode_num);
data_block_struct* allocate_data_block(int free_block_number);
dir_entry_struct* get_next_dir_entry(data_block_struct* block);
dir_entry_struct* get_next_entry(inode_struct* node);
dir_entry_struct* get_next_dir_entry_single(data_block_struct* index_block, int _segment);
dir_entry_struct* get_next_dir_entry_double(data_block_struct* index_block, int _segment);
void clear_inode_single_indirect(data_block_struct* single_indirect);
void clear_inode_double_indirect(data_block_struct* double_indirect);
file_object* create_file_object(int pid);
file_descriptor_table* get_fd_table(int pid);
int clear_entry_in_current_dir(inode_struct* cur_dir_inode, char* filename);

void print_data_block(int index);

/* file operations */
int rd_mkdir(char* pathname);
int rd_create(char *pathname, char* type, int mode);
int rd_unlink(char *pathname);
int rd_chmod(char *_pathname, unsigned int mode, int pid);
int rd_open(char *_pathname, unsigned int flags, int pid);
int write(int fd, int pid, char *data, int num_bytes);

void cleanup_fs();

// int rd_chmod(char *pathname, mode_t mode);

#endif // !RAMDISK_H
