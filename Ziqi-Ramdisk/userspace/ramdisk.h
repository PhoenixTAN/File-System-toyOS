#ifndef RAMDISK_H
#define RAMDISK_H

#include <stdio.h>
#include <string.h>		// strcmp

/* getconf ARG_MAX */
#define CMD_MAX_LENGTH          128		// 2097152 in bytes

#define RAMDISK_SIZE            1 << 21 // 2 MB
#define BLOCK_SIZE              256     // in bytes
#define INODE_SIZE              64      // in bytes
#define INODE_ARRAY_SIZE        256     // in blocks
#define BITMAP_SIZE             4       // in blocks
#define MAX_NUM_FILE            1023    
#define MAX_FILE_SIZE           1067008 // in bytes      
#define INODE_NUM_DIRECT_PTR    8



typedef struct DIR {
	char name[14];
	unsigned short inode_num;
} dir_entry_t;
// padding: __attribute__((packed))


typedef union DATA_BLOCK {
	char data[BLOCK_SIZE];
	dir_entry_t entries[BLOCK_SIZE/16];
} data_block;


/* super block */
typedef struct SUPERBLOCK {
    unsigned int free_blocks;   // 4 bytes
    unsigned int free_inodes;   // 4 bytes
    
    char padding[248];         // 256 - 4 - 4

} superblock;


typedef struct SINGLE_INDIRECT {
    // index block belons to data block
    // 改
} single_indirect;


typedef struct DOUBLE_INDIRECT {
    
} double_indirect;


/* i node */
typedef struct I_NODE {
    unsigned int type;      // either “dir” or “reg” (4 bytes)
    unsigned int size;      // current file size in bytes (4 bytes)

    unsigned int pointers[INODE_NUM_DIRECT_PTR];       // 8 direct block pointer
    
    single_indirect *single_indirect_ptrs;   // 4 bytes
    double_indirect *double_indrect_ptrs;    // 4 bytes

    unsigned int access;    // 4 bytes

    // padding

} i_node;




/* bit map */


/* file descriptor table*/

/* file object */

/* */

/* Files Operations */

/*
int rd_creat(char* pathname, mode_t mode);
int rd_mkdir(char* pathname);
int rd_open(char* pathname, int flags);
int rd_close(int fd);
int rd_read(int fd, char* address, int num_bytes);
int rd_write(int fd, char* address, int num_bytes);
int rd_lseek(int fd, int offset);
int rd_unlink(char* pathname);
int rd_chmod(char* pathname, mode_t mode);
*/

#endif // !RAMDISK_H


