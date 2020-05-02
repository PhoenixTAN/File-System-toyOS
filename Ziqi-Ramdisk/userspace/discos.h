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
#define MAX_NUM_FILE            1024  
#define MAX_FILE_SIZE           1067008 // in bytes      
#define INODE_NUM_DIRECT_PTR    8
#define DATA_BLOCKS_NUM         7931    

typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

/* Data structure for file system */

/* super block */
typedef struct SUPERBLOCK {
    uint32_t free_blocks;   // 4 bytes
    uint32_t free_inodes;   // 4 bytes
    
    char padding[248];         // 256 - 4 - 4

} superblock_struct;


/* directory entry */
typedef struct DIR_ENTRY {
	char name[14];              // filename or directory name
	uint16_t inode_num;   // the corresponding inode index 2 bytes
} dir_entry_struct;


/* A data block: dir, reg files & index blocks */
typedef union DATA_BLOCK {
	char data[BLOCK_SIZE];
	dir_entry_struct entries[BLOCK_SIZE/sizeof(dir_entry_struct)];
} data_block_struct;


/* single indirect block pointer */
typedef struct SINGLE_INDIRECT {
    // index block belongs to data block
    data_block_struct *index_blocks[BLOCK_SIZE/4];     // 4 bytes = sizeof(pointer)
} single_indirect_struct;


/* single indirect block pointer */
typedef struct DOUBLE_INDIRECT {
    single_indirect_struct *index_blocks[BLOCK_SIZE/4];    
} double_indirect_struct;


/* i node */
typedef struct I_NODE {
    char type[4];      // either “dir” or “reg” (4 bytes)
    uint32_t size;      // current file size in bytes (4 bytes)

    data_block_struct *pointers[INODE_NUM_DIRECT_PTR];       // 8 direct block pointer
    
    single_indirect_struct *single_indirect_ptrs;   // 4 bytes
    double_indirect_struct *double_indrect_ptrs;    // 4 bytes

    uint32_t access;    // 4 bytes

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
    char bitmap[BITMAP_SIZE*BLOCK_SIZE]; 

    // 7931 blocks
    data_block_struct data_blocks[DATA_BLOCKS_NUM];

} filesys_struct;

int init_file_sys();
int rd_mkdir(char* pathname);


/* file descriptor table*/

/* file object */


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


