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
typedef union DATA_BLOCK data_block_struct;

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
union DATA_BLOCK {
    char data[BLOCK_SIZE];
    dir_entry_struct entries[BLOCK_SIZE/sizeof(dir_entry_struct)];
    data_block_struct* index_block[BLOCK_SIZE/4];    // 4 bytes = sizeof(pointer)
};


/* i node */
typedef struct I_NODE {
    char type[4];      // either “dir” or “reg” (4 bytes)
    uint32_t size;      // current file size in bytes (4 bytes)

    data_block_struct *pointers[INODE_NUM_DIRECT_PTR];       // 8 direct block pointer
    
    data_block_struct *single_indirect_ptrs;   // 4 bytes
    data_block_struct *double_indirect_ptrs;    // 4 bytes

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

/* file descriptor table*/

/* file object */



/* file system initialization*/
int init_file_sys();
void cmd_daemon();

/* helper functions */
int clear_bitmap(unsigned char* map, int index);
int set_bitmap(unsigned char* map, int index);
void parse_absolute_path(char* _path, char* _current_dir, char* _target);
int find_inode_number(char* pathname);
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

/* file operations */
int rd_mkdir(char* pathname);
int rd_create(char *pathname, char* type, int mode);
int rd_unlink(char *pathname);


#endif // !RAMDISK_H
