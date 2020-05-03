#include "discos.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

/*
pwd
ls
cd 
cd 
*/

/* Macro from test file */
// #define TEST1
// #define TEST2
// #define TEST3
// #define TEST4
#define TEST5
// #define TEST6

#define PATH_PREFIX ""

#define CREAT   rd_create
#define OPEN    rd_open
#define WRITE   rd_write
#define READ    rd_read
#define UNLINK  rd_unlink
#define MKDIR   rd_mkdir
#define CLOSE   rd_close
#define LSEEK   rd_lseek
#define CHMOD   rd_chmod

// File modes
#define RD  (S_IRUSR | S_IRGRP | S_IROTH)
#define RW  (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define WR  (S_IWUSR | S_IRGRP | S_IROTH)

filesys_struct* discos;

int main(void) {

	printf("\n\nDiscos ##########\n");

	// Data structure size check
    printf("Data structure size check:\n");
	printf("   superblock %d\n", sizeof(superblock_struct));
	printf("   dir entry %d\n", sizeof(dir_entry_struct));
	printf("   data block %d\n", sizeof(data_block_struct));
	printf("   single_indirect %d\n", sizeof(single_indirect_struct));
	printf("   double_indirect %d\n", sizeof(double_indirect_struct));
	printf("   inode %d\n", sizeof(inode_struct));
	printf("   filesys %d\n", sizeof(filesys_struct));
    // test mkdir 
    /*
    int ret;
    ret = rd_mkdir("/");    // output root exits
    ret = rd_mkdir("/folder1"); 
    ret = rd_mkdir("/folder1/floder2");
    ret = rd_mkdir("/folder1/floder3");
    ret = rd_mkdir("/folder1/floder3/floder4");
    ret = rd_create("/folder1/floder2/Hellooooooooo", "reg\0", 1);
    print_bitmap(discos->bitmap);*/

    // initialize file system
	init_file_sys();

    char pathname[80];

    int retval, i;
    int fd;
    int index_node_number;
    #ifdef TEST1

    /* ****TEST 1: MAXIMUM file creation**** */

    /* Generate MAXIMUM regular files */
    for ( i = 0; i < MAX_NUM_FILE - 1; i++ ) { 
        sprintf (pathname, PATH_PREFIX "/file%d", i);
    
        retval = CREAT (pathname, "reg\0", RD);
    
        if (retval < 0) {
            fprintf (stderr, "creat: File creation error! status: %d (%s)\n", retval, pathname);
            perror("Error!");
      
            if (i != MAX_NUM_FILE - 1)
	            exit(EXIT_FAILURE);
        }
    
        memset (pathname, 0, 80);
    }   

    /* Delete all the files created */
    /*for (i = 0; i < MAX_FILES; i++) { 
        sprintf (pathname, PATH_PREFIX "/file%d", i);
    
        retval = UNLINK (pathname);
    
        if (retval < 0) {
            fprintf (stderr, "unlink: File deletion error! status: %d\n", retval);
      
            exit(EXIT_FAILURE);
        }
    
        memset (pathname, 0, 80);
    }*/
    printf("<1> Test 1 pass!\n\n");
    #endif // TEST1

    #ifdef TEST5
  
    /* ****TEST 5: Make directory including entries **** */
    retval = MKDIR (PATH_PREFIX "/dir1");
    
    if (retval < 0) {
        fprintf (stderr, "mkdir: Directory 1 creation error! status: %d\n", retval);
        exit(EXIT_FAILURE);
    }

    retval = MKDIR (PATH_PREFIX "/dir1/dir2");
    
    if (retval < 0) {
        fprintf (stderr, "mkdir: Directory 2 creation error! status: %d\n", retval);

        exit(EXIT_FAILURE);
    }

    retval = MKDIR (PATH_PREFIX "/dir1/dir3");
    
    if (retval < 0) {
        fprintf (stderr, "mkdir: Directory 3 creation error! status: %d\n", retval);

        exit(EXIT_FAILURE);
    }
    printf("<1> TEST 5 pass!\n\n");
    #endif // TEST5

	free(discos);

	return 0;
}

/* get the free block number from bitmap 
    return the number of the free data block.
    If there is no data block, return -1.
*/
int get_free_block_num_from_bitmap(unsigned char* map) {

    if ( discos->superblock.free_blocks <= 0 ) {
        // no free blocks
        return -1;
    }
   
    // map: char bitmap[BITMAP_SIZE*BLOCK_SIZE]; 8192 bits in all
    // the first 7931 blocks are usable

    // find the first 0 bit in bit map
    int i;
    int free_byte_pos;
    unsigned char free_byte;
    // iterate bit map byte by byte
    for ( i = 0; i < /*BITMAP_SIZE*BLOCK_SIZE*/1024; i++ ) {
        unsigned char ch = map[i];
        if ( ch != 255 )  {
            free_byte_pos = i;
            free_byte = map[i];
            break;
        }
    }

    // no free data block
    if ( i == /*BITMAP_SIZE*BLOCK_SIZE*/1024 ) {
        return -1;
    }
    // printf("byte i=%d\n", free_byte);
    // printf("free_byte_pos=%d\n", free_byte_pos);
    // iterate a byte bit by bit
    int free_bit;
    for ( i = 0, free_bit = free_byte_pos * 8; i < 8; i++ ) {
        if ( (free_byte & (unsigned char)1 ) == 0 ) {
            free_bit += i;
            break;
        }
        free_byte = free_byte >> 1;
    }
    // printf("free_bit: %d\n", free_bit);
    // check it out whether it is greater than 7931-1
    if ( free_bit >= 7931 ) {
        return -1;
    }

    return free_bit;
}


int init_file_sys() {
	int retval;
	discos = (filesys_struct*)malloc(sizeof(filesys_struct));
	if ( !discos ) {
		printf("discos malloc fail.\n");
		return -1;
	}
	discos->superblock.free_blocks = DATA_BLOCKS_NUM;
	discos->superblock.free_inodes = MAX_NUM_FILE;
	retval = rd_mkdir("/");
	return retval;
}


int rd_mkdir(char* pathname) {
	char* pwd;
	char* filename;
	int len = strlen(pathname);
    printf("rd_mkdir strlen(pathname): %d\n", len);
	pwd = malloc(len);
	filename = malloc(len);
	if(strcmp(pathname, "/") == 0 && discos->superblock.free_inodes == MAX_NUM_FILE) {
		printf("init root dir\n");
		discos->superblock.free_inodes--;
        strcpy(discos->inodes[0].type, "dir\0");
		return 0;
	}
	if(strcmp(pathname, "/") == 0 && discos->superblock.free_inodes != MAX_NUM_FILE) {
		printf("root dir exist!\n");
		return -1;
	}

    /* modify 23:22 */
    int ret = rd_create(pathname, "dir\0", 1);
	free(pwd);
	free(filename);

    if ( ret == 0 ) {
        printf("<1> file/directory created successfully !!!!\n");
        return 0;
    }
	return -1;
}


void parse_absolute_path(char* _path, char* _current_dir, char* filename) {
	//absolute path, current category, file name
	int i;
	int len = strlen(_path);
	for ( i = len - 1; i >= 0; i-- ) {
		if(_path[i] == '/' && i != 0) {
            // char * strncpy ( char * destination, const char * source, size_t num );
			strncpy(_current_dir, _path, i);
			break;
		}   
		if(_path[i] == '/' && i == 0) {
			strcpy(_current_dir, "/\0");
            break;
		}
	}
    // char * strcpy ( char * destination, const char * source );
	strcpy(filename, &_path[i + 1]);  
}

/*
    return the inode number
*/
int find_inode_number(char* pathname) {
    
    const char* delim = "/";
	
    char* temp_pathname;
	temp_pathname = malloc(strlen(pathname));
	strcpy(temp_pathname, pathname);
    printf("deep copy path: %s\n", temp_pathname);
	
    char* split_string = strtok(temp_pathname, delim);
    printf("delete the first delim temp_pathname: %s\n", temp_pathname);
    printf("delete the first delim split_string: %s\n", split_string);

    if ( split_string == NULL ) {
        return 0;
    }

	int pre_node_num = 0;
	int cur_node_num = 0;

	data_block_struct* cur_data_block;
    // ret = rd_mkdir("/folderA/folderB/folderC"); 
	while(split_string) {
		printf("looking for dir: %s\n", split_string);
		
        int i;
		// directory entry
		for ( i = 0; i < INODE_NUM_DIRECT_PTR; i++ ) {
		    cur_data_block = discos->inodes[cur_node_num].pointers[i];
			// if cur_data_block is null, return cur_node_number, pathname = pathname
            if ( cur_data_block != NULL ) {
                printf("cur_data_block != NULL\n");
                int j;
                for ( j = 0; j < BLOCK_SIZE/sizeof(dir_entry_struct); j++ ) {
                    dir_entry_struct* entry = &cur_data_block->entries[j];
                    if ( entry != NULL && strcmp(entry->name, split_string) == 0 ) {
                        cur_node_num = entry->inode_num;
                        break;
                    }
                }
            }

            if ( cur_node_num != pre_node_num ) {
                // which means we have found the current dir_entry
                printf("which means we have found the current dir_entry in direct block.\n");
                break;
            }
		}

        // you find the current dir in the direct block
		if ( cur_node_num != pre_node_num ) {
			pre_node_num = cur_node_num;
            split_string = strtok(NULL, delim);
            printf("To be continued 1... \n");
			continue;   // to find the next level directory
		}

        // we cannot find the dir_entry in the first 8 direct entry
        // directory does not exits
        /* uncomment return -1 here */
        // return -1;

		/* try to find current dir in single indirect block */
        data_block_struct* single_indirect_ptr = &discos->inodes[cur_node_num].single_indirect_ptrs;
        // NULL pointer exception
        if ( single_indirect_ptr != NULL ) {
            for( i = 0; i < BLOCK_SIZE/4; i++ ) {
                // this data block contains 64 block pointers (see as index_block[] in the data struct)
                cur_data_block = single_indirect_ptr->index_block[i];   
                if ( cur_data_block != NULL ) {
                    int j;
                    for ( j = 0; j < BLOCK_SIZE/16; j++ ) {
                        dir_entry_struct* entry = &cur_data_block->entries[j];
                        if ( entry != NULL && strcmp(entry->name, split_string) == 0 ) {
                            cur_node_num = entry->inode_num;
                            printf("Single indirect: i=%dth pointer, j=%dth entry\n", i, j);
                            break;
                        }
                    }
                }
                if ( cur_node_num != pre_node_num ) {
                    // which means we have found the current dir_entry
                    printf("which means we have found the current dir_entry in the single indrect block.\n");
                    break;
                }
            }
        }		


        if ( cur_node_num != pre_node_num ) {
			pre_node_num = cur_node_num;
            split_string = strtok(NULL, delim);
            printf("To be continued 2... \n");
			continue;   // to find the next level directory
		}

        /* try to find current dir in double indirect block */
        data_block_struct* double_indirect_ptr = &discos->inodes[cur_node_num].double_indirect_ptrs;
        // NULL pointer exception
        if ( double_indirect_ptr != NULL ) {
            int k;
            for ( k = 0; k < BLOCK_SIZE/4; k++ ) {
                // we got 64 single indirect pointer here
                data_block_struct* single_indirect = double_indirect_ptr->index_block[k];
                // now we do the same thing as we do to single indrect pointer
                // NULL pointer exception
                if ( single_indirect != NULL ) {
                    for( i = 0; i < BLOCK_SIZE/4; i++ ) {
                        // this data block contains 64 block pointers (see as index_block[] in the data struct)
                        cur_data_block = single_indirect->index_block[i];   
                        if ( cur_data_block != NULL ) {
                            int j;
                            for ( j = 0; j < BLOCK_SIZE/16; j++ ) {
                                dir_entry_struct* entry = &cur_data_block->entries[j];
                                if ( entry != NULL && strcmp(entry->name, split_string) == 0 ) {
                                    cur_node_num = entry->inode_num;
                                    printf("Double indirect: k=%dth double ptr, i=%dth single ptr, j=%dth entry\n", k, i, j);
                                    break;
                                }
                            }
                        }
                        if ( cur_node_num != pre_node_num ) {
                            // which means we have found the current dir_entry
                            printf("which means we have found the current dir_entry in the single indrect block.\n");
                            break;
                        }
                    }
                }
                if ( cur_node_num != pre_node_num ) {
                    // which means we have found the current dir_entry
                    printf("which means we have found the current dir_entry in the single indrect block.\n");
                    break;
                }
            }
        }

        if ( cur_node_num != pre_node_num ) {
			pre_node_num = cur_node_num;
            split_string = strtok(NULL, delim);
            printf("To be continued 3... \n");
			continue;   // to find the next level directory
		}
        
        // which mean you cannot find the dir
        return -1;

	}

	free(temp_pathname);

    return cur_node_num;
}


/* clear the bit of loc to be zero in the bitmap */
int clear_bitmap(unsigned char* map, int index) {
    // location should be in range [0, 7931)
    if( index < 0 || index > 7931 ) {
        printf("location illeagal!\n");
        return -1;
    }

    int seg = index / 8;
    int offset = index % 8;

    map[seg] = map[seg] & ~( (unsigned char)1 << offset );
    
    return 0;
}


int set_bitmap(unsigned char* map, int index) {

    // location should be in range [0, 7931)
    if( index < 0 || index > 7931 ) {
        printf("location illeagal!\n");
        return -1;
    }

    int seg = index / 8;
    int offset = index % 8;

    map[seg] = map[seg] | ( (unsigned char)1 << offset );
    
    return 0;
}   


void print_bitmap (unsigned char* map) {
    printf("Printing bit map: \n");
    int i;
    for ( i = 0; i < 1024; i++ ) {
        printf("%u ", map[i]);
    }
    printf("\n\n");
}


// get free inode
int get_free_inode_num() {
    if ( discos->superblock.free_inodes <= 0 ) {
        return -1;
    }
    int i;
    for(i = 1; i < MAX_NUM_FILE; i++) {
        inode_struct temp_inode = discos->inodes[i];
        // 这里需要改进吗
        if(strcmp(temp_inode.type, "")==0) {
            return i;
        }
    }
    return -1;
}


inode_struct* allocate_inode( int free_inode_num ) {
    
    discos->superblock.free_inodes--;
    memset(&discos->inodes[free_inode_num], 0, INODE_SIZE);
    return &discos->inodes[free_inode_num];
}


/* initialize and allocate a new block */
data_block_struct* allocate_data_block( int free_block_number ) {

    // set the bit map
    printf("setting bitmap: free_block_number: %d\n", free_block_number);
    set_bitmap(discos->bitmap, free_block_number);

    // update super block
    discos->superblock.free_blocks--;

    // memory allocation
    memset(&discos->data_blocks[free_block_number], 0, BLOCK_SIZE);     

    return &discos->data_blocks[free_block_number];
}



/* create file or directory */
int rd_create(char *pathname, char* type, int mode) {
    
    printf("Creating file/directory ...\n");

    if ( discos->superblock.free_inodes <= 0 ) {
        printf("File/Directory failed: not enough free inodes.\n");
        return -1;
    }

    if ( discos->superblock.free_blocks <= 0 ) {
        printf("File/Directory failed: not enough free blocks.\n");
        return -1;
    }

    
    // find the current directory
    // get the file/directory name you want to create
    char *current_path = malloc(strlen(pathname));
    char *entry_name = malloc(strlen(pathname));
    parse_absolute_path(pathname, current_path, entry_name);
    printf("parse: current path: %s    entry_name: %s\n", current_path, entry_name);

    // find the inode of the current directory
    int cur_inode_num = find_inode_number(current_path);
    if ( cur_inode_num == -1 ) {
        printf("Cannot find such a directory.\n");
        return -1;
    }
    printf("find the inode number of current dir: %d\n", cur_inode_num);
    inode_struct* current_path_inode = &discos->inodes[cur_inode_num];

    // find the next free entry in current directory
    dir_entry_struct* free_dir_entry = get_next_entry(current_path_inode);
    if ( free_dir_entry == NULL ) {
        printf("No free entry!\n");
        // 如果后面继续遍历间接指针 也没有，就说明已经到最大entry数目
        return -1;
    }

    int free_inode_num = get_free_inode_num();
    if ( free_inode_num == -1 ) {
        printf("File/Directory failed: cannot find free inodes.\n");
        return -1;
    }
    printf("get a free inode number: %d\n", free_inode_num);
    inode_struct* free_inode = allocate_inode( free_inode_num );
    
    printf("strcpy entry_name: %s\n", entry_name);
    strcpy(free_dir_entry->name, entry_name);
    printf("free inode number: %d\n", free_inode_num);
    free_dir_entry->inode_num = free_inode_num;
    printf("inode type: %s\n", type);
    strcpy(free_inode->type, type);

    /* update size */
    current_path_inode->size += 16;

    free(current_path);
    free(entry_name);
    
    return 0;
}

/* get_next_dir_entry in direct block*/
dir_entry_struct* get_next_dir_entry(data_block_struct* block) {
    int j;
    for ( j = 0; j < BLOCK_SIZE/16; j++ ) {
        if ( block->entries[j].inode_num == 0 ) {
            printf("get next dir entry: return block->entries[%d]\n", j);
            return &block->entries[j];
        }
    }
    return NULL;
}


/* get next dir entry from single indirect block */
dir_entry_struct* get_next_dir_entry_single(data_block_struct* index_block, int _segment) {
    
    int seg = _segment; // segment has been substracted by 8
    // we start from the first block of single indirect blocks
    // data_block_struct* index_block is the single indirect pointer
    data_block_struct* block = &index_block[seg];
    if ( block == NULL ) {
        int free_block_num = get_free_block_num_from_bitmap(discos->bitmap);
        if ( free_block_num == -1 ) {
                printf("Cannot find free blocks.\n");
                return NULL;
        }
        // we find a new block
        printf("free block number: %d, single index_block[%d]\n", free_block_num, seg);

        data_block_struct* new_data_block = allocate_data_block( free_block_num );
        printf("After allocate datablock: \n");
        // print_bitmap(discos->bitmap);
        /* !! update ptr */
        block = new_data_block;
        return &block->entries[0];
    }

    return get_next_dir_entry(block);
}

dir_entry_struct* get_next_dir_entry_double(data_block_struct* index_block, int _segment) {

    // locate the single indirect
    int seg = _segment/64;
    data_block_struct* block = &index_block[seg];
    if ( block == NULL ) {
        int free_block_num = get_free_block_num_from_bitmap(discos->bitmap);
        if ( free_block_num == -1 ) {
                printf("Cannot find free blocks.\n");
                return NULL;
        }
        // we find a new block
        printf("free block number: %d, double index_block[%d]\n", free_block_num, seg);
        data_block_struct* new_index_block = allocate_data_block( free_block_num );
        printf("After allocate datablock: \n");
        // print_bitmap(discos->bitmap);
        /* !! update ptr */
        block = new_index_block;
    }
    return get_next_dir_entry_single(block, _segment%64);
}


/* get next entry from the current inode */
dir_entry_struct* get_next_entry(inode_struct* node) {

    // find the block that has not been full
    int seg = node->size / BLOCK_SIZE;
    printf("last block that is not full: node->size / BLOCK_SIZE = %d\n", seg);
    
    /* find the block that has not been full in direct block */
    int i;
    for ( i = seg; i < INODE_NUM_DIRECT_PTR; i++ ) {
        data_block_struct* dir_ptr = node->pointers[i];
        // this block is empty
        if ( dir_ptr == NULL ) {
            printf("We need a new block.\n");
            // print_bitmap(discos->bitmap);
            int free_block_num = get_free_block_num_from_bitmap(discos->bitmap);
            if ( free_block_num == -1 ) {
                 printf("Cannot find free blocks.\n");
                 return NULL;
            }
            // we find a new block
            printf("free block number: %d, pointers[%d]\n", free_block_num, i);

            data_block_struct* new_data_block = allocate_data_block( free_block_num );
            printf("After allocate datablock: \n");
            // print_bitmap(discos->bitmap);
            /* !! update ptr */
            node->pointers[i] = new_data_block;
            return &new_data_block->entries[0];
        }

        // this block has not been full yet
        // then we get the next free entry in this block
        dir_entry_struct* free_dir_entry = get_next_dir_entry(dir_ptr);
        if ( free_dir_entry != NULL ) {
            printf("return free_dir_entry\n");
            return free_dir_entry;
        }
    }

    /* single indirect block */
    // seg should be in range [8,72)
    if ( seg >= 8 && seg < (8 + 64) ) {
        if ( node->single_indirect_ptrs == NULL ) {
            printf("Single indirect (NULL). We need a new block.\n");
            
            // print_bitmap(discos->bitmap);
            int free_block_num = get_free_block_num_from_bitmap(discos->bitmap);
            if ( free_block_num == -1 ) {
                 printf("Cannot find free blocks.\n");
                 return NULL;
            }
            // we find a new block
            printf("free block number: %d\n", free_block_num);

            data_block_struct* new_index_block = allocate_data_block( free_block_num );
            printf("After allocate datablock: \n");
            // print_bitmap(discos->bitmap);
            /* !! update ptr */
            node->single_indirect_ptrs = new_index_block;
        }

        // this block has not been full yet
        // then we get the next free entry in this block
        dir_entry_struct* free_dir_entry = get_next_dir_entry_single(node->single_indirect_ptrs, seg - 8);
        if ( free_dir_entry != NULL ) {
            printf("return free_dir_entry\n");
            return free_dir_entry;
        }
        /* why seg = 72 */
    }

    /* double indrect block */
    if ( seg >= (8 + 64) && seg < (8 + 64 + 64*64) ) {
        if ( node ->double_indirect_ptrs ==  NULL ) {
            printf("Double indirect (NULL). We need a new block.\n");
            int free_block_num = get_free_block_num_from_bitmap(discos->bitmap);
            if ( free_block_num == -1 ) {
                 printf("Cannot find free blocks.\n");
                 return NULL;
            }
            // we find a new block
            printf("free block number: %d\n", free_block_num);

            data_block_struct* new_index_block = allocate_data_block( free_block_num );
            printf("After allocate datablock: \n");
            // print_bitmap(discos->bitmap);
            /* !! update ptr */
            node->double_indirect_ptrs = new_index_block;
        }

        dir_entry_struct* free_dir_entry = get_next_dir_entry_double(node->double_indirect_ptrs, seg - 72);
        if ( free_dir_entry != NULL ) {
            printf("return free_dir_entry\n");
            return free_dir_entry;
        }

    }

    if ( seg >= (8 + 64 + 64*64) ) {
        printf("Over 8 + 64 + 64*64\n");
    }

    return NULL;
}



