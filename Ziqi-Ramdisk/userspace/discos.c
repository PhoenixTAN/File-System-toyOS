#include "discos.h"


/* Macro from test file */
#define TEST1
#define TEST2
//#define TEST3
#define TEST4
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

// File open flags
#define READONLY   O_RDONLY
#define READWRITE  O_RDWR
#define WRITEONLY  O_WRONLY

#define TEST_SINGLE_INDIRECT
#define TEST_DOUBLE_INDIRECT


#define BLK_SZ 256		/* Block size */
#define DIRECT 8		/* Direct pointers in location attribute */
#define PTR_SZ 4		/* 32-bit [relative] addressing */
#define PTRS_PB  (BLK_SZ / PTR_SZ) /* Pointers per index block */
static char data1[DIRECT*BLK_SZ]; /* Largest data directly accessible */
static char data2[PTRS_PB*BLK_SZ];     /* Single indirect data size */
static char data3[PTRS_PB*PTRS_PB*BLK_SZ]; /* Double indirect data size */
static char addr[PTRS_PB*PTRS_PB*BLK_SZ+1]; /* Scratchpad memory */

/* global variables */
filesys_struct* discos;

// process_fd_table p_fd_table[THERAD_POOL_SIZE];
process_fd_table* p_fd_table;

int main(void) {

	printf("\n\nDiscos ##########\n");

	// Data structure size check
    printf("Data structure size check:\n");
	printf("   superblock %d\n", sizeof(superblock_struct));
	printf("   dir entry %d\n", sizeof(dir_entry_struct));
	printf("   data block %d\n", sizeof(data_block_struct));
	printf("   inode %d\n", sizeof(inode_struct));
	printf("   filesys %d\n", sizeof(filesys_struct));


    // initialize file system
	init_file_sys();

	/* test_file.c */
    char pathname[80];

    int retval, i;
    int fd;
    int index_node_number;

    /* Some arbitrary data for our files */
    memset (data1, '1', sizeof (data1));
    memset (data2, '2', sizeof (data2));
    memset (data3, '3', sizeof (data3));

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

    printf("<1> Test 1 created 1023 files succeeded!\n\n");
    print_bitmap(discos->bitmap);
    /*for ( i = 0; i < 65; i++ ) {
        print_block_entries_info(i);
    }*/
    /* Delete all the files created */
    for (i = 0; i < MAX_NUM_FILE - 1; i++) { 
        
        sprintf (pathname, PATH_PREFIX "/file%d", i);

        retval = UNLINK (pathname);
    
        if (retval < 0) {
            fprintf (stderr, "unlink: File deletion error! status: %d\n", retval);
      
            exit(EXIT_FAILURE);
        }
    
        memset (pathname, 0, 80);
    }
    printf("<1> Test 1 pass!\n\n");
    #endif // TEST1

    print_bitmap(discos->bitmap);

    print_inode_info(0);
    print_inode_info(1);


    #ifdef TEST2
    /* ****TEST 2: LARGEST file size**** */
    /* Generate one LARGEST file */
    retval = CREAT (PATH_PREFIX "/bigfile", "reg\0", RW);

    if ( retval < 0 ) {
        fprintf (stderr, "creat: File creation error! status: %d\n", retval);
        exit(EXIT_FAILURE);
    }

    retval =  OPEN (PATH_PREFIX "/bigfile", RW, 100);   /* Open file to write to it */
    
    if ( retval < 0 ) {
        fprintf (stderr, "open: File open error! status: %d\n", retval);
        exit(EXIT_FAILURE);
    }

    fd = retval;			/* Assign valid fd */

    /* Try writing to all direct data blocks */
    retval = WRITE (fd, 100, data1, sizeof(data1));
    
    if (retval < 0) {
        fprintf (stderr, "write: File write STAGE1 error! status: %d\n", retval);
        exit(EXIT_FAILURE);
    }
    printf("<2> Test 2 writes something to direct blocks.\n");

    for ( i = 0; i < 10; i++ ) {
        print_data_block(i);
    }

    print_inode_info(4);

    #ifdef TEST_SINGLE_INDIRECT
    
    /* Try writing to all single-indirect data blocks */
    retval = WRITE (fd, 100, data2, sizeof(data2));
    
    if ( retval < 0 ) {
        fprintf (stderr, "write: File write STAGE2 error! status: %d\n", retval);
        exit(EXIT_FAILURE);
    }
    printf("<1> TEST SINGLE INDIRECT DATA Success!!\n");
    print_inode_info(4);
    

    #ifdef TEST_DOUBLE_INDIRECT

    /* Try writing to all double-indirect data blocks */
    retval = WRITE (fd, 100, data3, sizeof(data3));
    
    if ( retval < 0 ) {
        fprintf (stderr, "write: File write STAGE3 error! status: %d\n", retval);
        exit(EXIT_FAILURE);
    }
    printf("<1> TEST DOUBLE INDIRECT DATA Success!!\n");
    print_inode_info(4);
    #endif // TEST_DOUBLE_INDIRECT

    #endif // TEST_SINGLE_INDIRECT
    printf("<1> TEST 2 FINISH!!!\n");
    #endif // TEST2



    #ifdef TEST3

    /* ****TEST 3: Seek and Read file test**** */
    retval = LSEEK (fd, 0, 100);	/* Go back to the beginning of your file */

    if (retval < 0) {
        fprintf (stderr, "lseek: File seek error! status: %d\n", retval);
        exit(EXIT_FAILURE);
    }

    /* Try reading from all direct data blocks */
    retval = READ (fd, addr, sizeof(data1), 100);
  
    if ( retval < 0 ) {
        fprintf (stderr, "read: File read STAGE1 error! status: %d\n", retval);
        exit(EXIT_FAILURE);
    }

    /* Should be all 1s here... */
    printf ("Data at addr: %s\n", addr);
    printf ("Press a key to continue\n");
    getchar(); // Wait for keypress

    #ifdef TEST_SINGLE_INDIRECT

    /* Try reading from all single-indirect data blocks */
    retval = READ (fd, addr, sizeof(data2), 100);
  
    if (retval < 0) {
        fprintf (stderr, "read: File read STAGE2 error! status: %d\n", retval);
        exit(EXIT_FAILURE);
    }
    /* Should be all 2s here... */
    printf ("Data at addr: %s\n", addr);
    printf ("Press a key to continue\n");
    getchar(); // Wait for keypress

    #ifdef TEST_DOUBLE_INDIRECT

    /* Try reading from all double-indirect data blocks */
    retval = READ (fd, addr, sizeof(data3), 100);
  
    if (retval < 0) {
        fprintf (stderr, "read: File read STAGE3 error! status: %d\n", retval);
        exit(EXIT_FAILURE);
    }
    /* Should be all 3s here... */
    printf ("Data at addr: %s\n", addr);
    printf ("Press a key to continue\n");
    getchar(); // Wait for keypress

    #endif // TEST_DOUBLE_INDIRECT

    #endif // TEST_SINGLE_INDIRECT

    /* Close the bigfile */
    retval = CLOSE(fd, 100);
  
    if (retval < 0) {
        fprintf (stderr, "close: File close error! status: %d\n", retval);
        exit(EXIT_FAILURE);
    }

    #endif // TEST3

    #ifdef TEST4

    /* ****TEST 4: Check permissions**** */
    retval = CHMOD(PATH_PREFIX "/bigfile", RD); // Change bigfile to read-only
  
    if (retval < 0) {
        fprintf (stderr, "chmod: Failed to change mode! status: %d\n", retval);
        exit(EXIT_FAILURE);
    }

    /* Now try to write to bigfile again, but with read-only permissions! */
    retval = WRITE (fd, 100, data1, sizeof(data1));
    if ( retval < 0 ) {
        fprintf (stderr, "chmod: Tried to write to read-only file!\n");
        printf ("Press a key to continue\n");
        getchar(); // Wait for keypress
    }
  
    /* Remove the biggest file */

    retval = UNLINK (PATH_PREFIX "/bigfile");
	
    if (retval < 0) {
        fprintf (stderr, "unlink: /bigfile file deletion error! status: %d\n", retval);   
        exit(EXIT_FAILURE);
    }

    #endif // TEST4
    print_bitmap(discos->bitmap);


    #ifdef TEST5
  
    /* ****TEST 5: Make directory including entries **** */
    retval = MKDIR (PATH_PREFIX "/dir1");
    
    if ( retval < 0 ) {
        fprintf (stderr, "mkdir: Directory 1 creation error! status: %d\n", retval);
        exit(EXIT_FAILURE);
    }

    retval = MKDIR (PATH_PREFIX "/dir1/dir2");
    
    if ( retval < 0 ) {
        fprintf (stderr, "mkdir: Directory 2 creation error! status: %d\n", retval);
        exit(EXIT_FAILURE);
    }

    retval = MKDIR (PATH_PREFIX "/dir1/dir3");
    
    if ( retval < 0 ) {
        fprintf (stderr, "mkdir: Directory 3 creation error! status: %d\n", retval);
        exit(EXIT_FAILURE);
    }
    printf("<1> TEST 5 pass!\n\n");
    
    #endif // TEST5

    
	/* free ramdisk */
	free(discos);
    free(p_fd_table);
	return 0;
}


void print_data_block(int index) {
    printf("Data block %d content: \n", index);
    data_block_struct* block = &discos->data_blocks[index];
    int i;
    for ( i = 0; i < BLOCK_SIZE; i++ ) {
        printf("%c ", block->data[i]);
    }
    printf("\n");
}


void print_block_entries_info(int index) {
    printf("Block %d info: \n", index);
    data_block_struct* block = &discos->data_blocks[index];
    int i;
    for ( i = 0; i < BLOCK_SIZE/16; i++ ) {
        printf("   Entry %d,  entry->name: %s, inode_num: %u\n", i, block->entries[i].name, block->entries[i].inode_num);
    }
}


void print_inode_info(int index) {
    printf("Inode %d info:\n", index);
    inode_struct* inode = &discos->inodes[index];
    printf("     type: %s\n", inode->type);
    printf("     size: %d\n", inode->size);
    printf("     access: %u\n", inode->access); 
    int i;
    for ( i = 0; i < INODE_NUM_DIRECT_PTR; i++ ) {
        if ( inode->pointers[i] == NULL ) {
            printf("Inode->pointers[%d] is NULL \n", i);
        }
        else {
            printf("Inode->pointers[%d]: point to block %d\n", i, inode->pointers[i] - &discos->data_blocks[0]);
        }
    }

    if ( inode->single_indirect_ptrs == NULL ) {
        printf("Single indirect is NULL\n");
    }
    else {
        printf("Single indirect points to block %d\n", inode->single_indirect_ptrs - &discos->data_blocks[0]);
        // iterate the entry
        int j;
        for ( j = 0; j < BLOCK_SIZE/16; j++ ) {
            data_block_struct* index_block = inode->single_indirect_ptrs->index_block[j];
            if ( index_block != NULL ) {
                printf("    index block[%d] point to %d\n", j, index_block - &discos->data_blocks[0]);
            }
            
        }
    }
    printf("\n");
}


void cmd_daemon() {
	char cmd[CMD_MAX_LENGTH] = "Hello Discos!\n";

	printf("%s", cmd);

	while ( 1 ) {
		printf("Discos# ");

		fgets(cmd, CMD_MAX_LENGTH, stdin);	// char* _Buffer, int   _MaxCount, FILE* _Stream
		cmd[strlen(cmd)-1] = '\0';

		if ( !strcmp(cmd, "exit") ) {
			printf("Exit!!!\n");
			break;
		}

	}
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
    memset(discos, 0, sizeof(sizeof(filesys_struct)));
    p_fd_table = malloc(sizeof(process_fd_table)*THERAD_POOL_SIZE);
    memset(p_fd_table, 0, sizeof(process_fd_table)*THERAD_POOL_SIZE);
	if ( !discos ) {
		printf("discos malloc fail.\n");
		return -1;
	}
	discos->superblock.free_blocks = DATA_BLOCKS_NUM;
	discos->superblock.free_inodes = MAX_NUM_FILE;
	retval = rd_mkdir("/");
	return retval;
}

/**
 * mkdir
 * This behaves like rd_creat() but pathname refers to a directory file. 
 * If the file already exists, return -1 else return 0. 
 * Note that you need to update the parent directory file, to include the new entry.
 * 
*/
int rd_mkdir(char* pathname) {
	char* pwd;
	char* filename;
	int len = strlen(pathname);
    // printf("rd_mkdir strlen(pathname): %d\n", len);
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
    int ret = rd_create(pathname, "dir\0", RW);
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
    // printf("deep copy path: %s\n", temp_pathname);
	
    char* split_string = strtok(temp_pathname, delim);
    // printf("delete the first delim temp_pathname: %s\n", temp_pathname);
    // printf("delete the first delim split_string: %s\n", split_string);

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
                // printf("cur_data_block != NULL\n");
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

        printf("Looking for %s in single indirect\n", split_string);
		/* try to find current dir in single indirect block */
        data_block_struct* single_indirect_ptr = discos->inodes[cur_node_num].single_indirect_ptrs;
        // NULL pointer exception
        if ( single_indirect_ptr != NULL ) {
            for( i = 0; i < BLOCK_SIZE/4; i++ ) {
                // this data block contains 64 block pointers (see as index_block[] in the data struct)
                cur_data_block = single_indirect_ptr->index_block[i];   
                if ( cur_data_block != NULL ) {
                    // printf("cur_data_block != NULL\n");
                    int j;
                    for ( j = 0; j < BLOCK_SIZE/16; j++ ) {
                        dir_entry_struct* entry = &cur_data_block->entries[j];
                        if ( entry == NULL ) {
                            printf("entry == NULL\n");
                        }
                        // printf("entry->name: %s    split_string: %s\n", entry->name, split_string);
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
        data_block_struct* double_indirect_ptr = discos->inodes[cur_node_num].double_indirect_ptrs;
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



/** 
 * create file or directory 
 * create a regular file with absolute pathname and mode from the root of the directory tree, 
 * where each directory filename is delimited by a "/" character. 
 * The mode can be read-write (default), read-only, or write-only. 
 * You can assume that any process opening an existing file is restricted by the access rights at creation time. 
 * On success, you should return 0, 
 * else if the file corresponding to pathname already exists you should return -1, indicating an error. 
 * Note that you need to update the parent directory file, to include the new entry.
 * */
int rd_create(char *pathname, char* type, unsigned int mode) {
    
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
        return -1;
    }

    int free_inode_num = get_free_inode_num();
    if ( free_inode_num == -1 ) {
        printf("File/Directory failed: cannot find free inodes.\n");
        return -1;
    }
    printf("get a free inode number: %d\n", free_inode_num);
    inode_struct* free_inode = allocate_inode( free_inode_num );
    
    
    strcpy(free_dir_entry->name, entry_name);
    printf("strcpy free_dir_entry->name: %s    entry_name: %s\n", free_dir_entry->name, entry_name);

    printf("free inode number: %d\n", free_inode_num);
    free_dir_entry->inode_num = (unsigned short)free_inode_num;

    printf("inode type: %s\n", type);
    strcpy(free_inode->type, type);
    free_inode->access = mode;

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
dir_entry_struct* get_next_dir_entry_single(data_block_struct* /*index_block*/single_indirect, int _segment) {
    
    /******************modify***/
    int seg = _segment; // segment has been substracted by 8
    // we start from the first block of single indirect blocks
    // data_block_struct* index_block is the single indirect pointer
    // data_block_struct* block = &index_block[seg];
    data_block_struct* block = single_indirect->index_block[seg];
    printf("<>block seg= %d\n", seg);

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
        single_indirect->index_block[seg] = new_data_block;

        return &single_indirect->index_block[seg]->entries[0];
    }

    // return get_next_dir_entry(block);
    return get_next_dir_entry(single_indirect->index_block[seg]);
}


dir_entry_struct* get_next_dir_entry_double(data_block_struct* double_indirect, int _segment) {

    // locate the single indirect
    int seg = _segment/64;
    data_block_struct* block = double_indirect->index_block[seg];
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
        block = new_index_block;    //////////////////////
        // double_indirect->index_block[seg] = new_index_block;
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
            return &node->pointers[i]->entries[0];
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
        /*************modify****/
        seg = 72;
    }

    /* double indrect block */
    // TODO:
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


/* clear inode, free datablocks */
void clear_inode(inode_struct* inode) {

	data_block_struct* temp_block_ptr;
	int i;
	for ( i = 0; i < INODE_NUM_DIRECT_PTR; i++ ) {
		temp_block_ptr = inode->pointers[i];
		if ( temp_block_ptr != NULL ) {
			// use pointer substraction to get the number of the target block
			clear_bitmap(discos->bitmap, temp_block_ptr - &discos->data_blocks[0]);
			memset(inode->pointers[i], 0, BLOCK_SIZE);
			discos->superblock.free_blocks++;
		}
	}

	if ( inode->single_indirect_ptrs != NULL ) {
		clear_inode_single_indirect(inode->single_indirect_ptrs);
	}

	if ( inode->double_indirect_ptrs != NULL ) {
		clear_inode_double_indirect(inode->double_indirect_ptrs);
	}

}


/* helper function for clear_inode */
void clear_inode_single_indirect(data_block_struct* single_indirect) {
    printf("clear_inode_single_indirect.\n");
	int i;
	for ( i = 0; i < BLOCK_SIZE/4; i++ ) {
		data_block_struct* block = single_indirect->index_block[i];
		if (  block != NULL ) {	
			// clear_bitmap(discos->bitmap, block - &discos->data_blocks[0]);
            printf("clear bit map: %d\n", single_indirect->index_block[i] - &discos->data_blocks[0]);
            clear_bitmap(discos->bitmap, single_indirect->index_block[i] - &discos->data_blocks[0]);
			memset(single_indirect->index_block[i], 0, BLOCK_SIZE);
			discos->superblock.free_blocks++;
		}
	}

	// clear the single indirect index_block[] itself
	clear_bitmap(discos->bitmap, single_indirect - &discos->data_blocks[0]);
	memset(single_indirect, 0, BLOCK_SIZE);
	discos->superblock.free_blocks++;
}


/* helper function for clear_inode */
void clear_inode_double_indirect(data_block_struct* double_indirect) {
	int i;
	for ( i = 0; i < BLOCK_SIZE/4; i++ ) {
		data_block_struct* single_indirect = double_indirect->index_block[i];
		if ( single_indirect != NULL ) {
			clear_inode_single_indirect(single_indirect);
		}
	}
	clear_bitmap(discos->bitmap, double_indirect - &discos->data_blocks[0]);
	memset(double_indirect, 0, BLOCK_SIZE);
	discos->superblock.free_blocks++;
}


/* find the entry name "filename" in the current directory inode */
dir_entry_struct* find_entry_in_current_dir(inode_struct* cur_dir_inode, char* filename) {

	dir_entry_struct* entry;
	int i;
	for ( i = 0; i < INODE_NUM_DIRECT_PTR; i++ ) {
		data_block_struct* block = cur_dir_inode->pointers[i];
		if ( block != NULL ) {
			// iterate the block to find the entry
			int j;
			for ( j = 0; j < BLOCK_SIZE/16; j++ ) {
				entry = &block->entries[j];
                if ( entry != NULL && strcmp(entry->name, filename) == 0 ) {
                    return entry;
                }
			}
		}
	}

    // find entry in single indirect
    data_block_struct* single_indirect = cur_dir_inode->single_indirect_ptrs;
    if ( single_indirect != NULL ) {
        for ( i = 0; i < BLOCK_SIZE/4; i++ ) {
            data_block_struct* block = single_indirect->index_block[i];
            if ( block != NULL ) {
                int j;
                for ( j = 0; j < BLOCK_SIZE/16; j++ ) {
                    entry = &block->entries[j];
                    if ( entry != NULL && strcmp(entry->name, filename) == 0 ) {
                        return entry;
                    }
                }
            }
        }
    }

    // find entry in double indirect
    data_block_struct* double_indirect = cur_dir_inode->double_indirect_ptrs;
    if ( double_indirect != NULL ) {
        // iterate single indirect
        for ( i = 0; i < BLOCK_SIZE/4; i++ ) {
            data_block_struct* single = double_indirect->index_block[i];
            if ( single != NULL ) {
                int j;
                for ( j = 0; j < BLOCK_SIZE/4; j++ ) {
                    data_block_struct* block = single->index_block[j];
                    if ( block != NULL ) {
                        int k;
                        for ( k = 0; k < BLOCK_SIZE/16; k++ ) {
                            entry = &block->entries[k];
                            if ( entry != NULL && strcmp(entry->name, filename) == 0 ) {
                                return entry;
                            }
                        }
                    }
                }
            }
        }
    }

	return NULL;
}


/* find the entry name "filename" in the current directory inode */
int clear_entry_in_current_dir(inode_struct* cur_dir_inode, char* filename) {
    printf("clearing entry in current dir...\n");
	dir_entry_struct* entry;  
    int findEntry = 0; 
	int i;
	for ( i = 0; i < INODE_NUM_DIRECT_PTR; i++ ) {
		data_block_struct* block = cur_dir_inode->pointers[i];
        int nonEmptyEntries = 0;
		if ( block != NULL ) {
			// iterate the block to find the entry
			int j;
			for ( j = 0; j < BLOCK_SIZE/16; j++ ) {
				entry = &block->entries[j];
                if ( entry->inode_num != 0 ) {
                    // printf("  entry->inode_num %d\n", entry->inode_num);
                    nonEmptyEntries += 1;
                }
                if ( entry != NULL && strcmp(entry->name, filename) == 0 ) {
                    
                    memset(entry, 0, sizeof(dir_entry_struct));
                    entry = NULL;
                    findEntry = 1;
                }
			}
		}
        // printf("nonEmptyEntries: %d   findEntry=%d\n", nonEmptyEntries, findEntry);
        if ( findEntry == 1 && nonEmptyEntries == 1 ) {
            clear_bitmap(discos->bitmap, cur_dir_inode->pointers[i] - &discos->data_blocks[0]);
            cur_dir_inode->pointers[i] = NULL;
            print_bitmap(discos->bitmap);
            
        }
	}


    if ( findEntry == 1 ) {
        return 0;
    }

    // find entry in single indirect
    // TODO: clear the single indirect index block
    data_block_struct* single_indirect = cur_dir_inode->single_indirect_ptrs;
    if ( single_indirect != NULL ) {
        int nonEmptyBlocks = 0;
        int lastEntry = 0;
        for ( i = 0; i < BLOCK_SIZE/4; i++ ) {
            data_block_struct* block = single_indirect->index_block[i];
            int nonEmptyEntries = 0;
            if ( block != NULL ) {
                nonEmptyBlocks += 1;
                int j;
                for ( j = 0; j < BLOCK_SIZE/16; j++ ) {
                    entry = &block->entries[j];
                    if ( entry->inode_num != 0 ) {
                        nonEmptyEntries += 1;
                    }
                    if ( entry != NULL && strcmp(entry->name, filename) == 0 ) {
                        // return entry;
                        findEntry = 1;
                        memset(entry, 0, sizeof(dir_entry_struct));
                        entry = NULL;
                        // return 0;
                    }
                }
            }
            // printf("nonEmptyEntries: %d   findEntry=%d\n", nonEmptyEntries, findEntry);
            if ( findEntry == 1 && nonEmptyEntries == 1 ) {
                printf("<><> clear bit %d\n", single_indirect->index_block[i] - &discos->data_blocks[0]);
                clear_bitmap(discos->bitmap, single_indirect->index_block[i] - &discos->data_blocks[0]);
                single_indirect->index_block[i] = NULL;
                print_bitmap(discos->bitmap);
                lastEntry = 1;
                // return 0;
            }
        }
        if ( findEntry == 1 && nonEmptyBlocks == 1 && lastEntry == 1 ) {
            printf("Clearing single indirect block itself.\n");
            printf("<><> clear bit %d\n", cur_dir_inode->single_indirect_ptrs - &discos->data_blocks[0]);
            clear_bitmap(discos->bitmap, cur_dir_inode->single_indirect_ptrs - &discos->data_blocks[0]);
            cur_dir_inode->single_indirect_ptrs = NULL;
            print_bitmap(discos->bitmap);
        }
    }

    if ( findEntry == 1 ) {
        return 0;
    }

    // TODO
    // find entry in double indirect
    data_block_struct* double_indirect = cur_dir_inode->double_indirect_ptrs;
    if ( double_indirect != NULL ) {
        // iterate single indirect
        for ( i = 0; i < BLOCK_SIZE/4; i++ ) {
            data_block_struct* single = double_indirect->index_block[i];
            if ( single != NULL ) {
                int j;
                for ( j = 0; j < BLOCK_SIZE/4; j++ ) {
                    data_block_struct* block = single->index_block[j];
                    if ( block != NULL ) {
                        int k;
                        for ( k = 0; k < BLOCK_SIZE/16; k++ ) {
                            entry = &block->entries[k];
                            if ( entry != NULL && strcmp(entry->name, filename) == 0 ) {
                                // return entry;
                                return 0;
                            }
                        }
                    }
                }
            }
        }
    }

	return 0;
}


/**
 * rd_unlink
 * remove the filename with absolute pathname from the filesystem, 
 * freeing its memory in the ramdisk. 
 * This function returns 0 if successful or -1 if there is an error. 
 * An error can occur if: 
 * 		(1) the pathname does not exist, 
 * 		(2) you attempt to unlink a non-empty directory file, 
 * 		(3) you attempt to unlink an open file, or 
 * 		(4) you attempt to unlink the root directory file.
*/
int rd_unlink(char* _pathname) {
	// deep copy the path name
	char* pathname = malloc(strlen(_pathname));
    strcpy(pathname, _pathname);
    printf("Unlinking %s ...\n", pathname);
	// error occurs if you attempt to unlink the root directory file.
	if ( strcmp(pathname, "/") == 0 ) {
		printf("<1> rd_unlink Error occurs: you attempt to unlink the root dir.\n");
		return -1;
	}

	// error occurs if you attempt to unlink an open file
	// TODO:

	// error occurs if the pathname does not exist
	int file_inode_num = find_inode_number(pathname);
    printf("<1> rd_unlink the file_inode_num: %d\n", file_inode_num);
	if ( file_inode_num == -1 ) {
		printf("<1> rd_unlink Error occurs: the pathname does not exist.\n");
		return -1;
	}
	inode_struct* file_inode = &discos->inodes[file_inode_num];
	
	// error occurs if you attempt to unlink a non-empty directory file
	if ( strcmp(file_inode->type, "dir\0") == 0 ) {
		// check whether this is a non-empty folder
		// 这里要什么复杂的操作吗
		if ( file_inode->size > 0 ) {
			printf("<1> rd_unlink Error occurs: you attempt to unlink the root dir.\n");
			return -1;
		}
	}

	char* current_dir = malloc(strlen(pathname));
	char* filename = malloc(strlen(pathname));
	parse_absolute_path(pathname, current_dir, filename);

	int current_dir_inode_num = find_inode_number(current_dir);
	if ( current_dir_inode_num == -1 ) {
		printf("<1> rd_unlink Error occurs: cannot find current dir.\n");
		return -1;
	}
	inode_struct* cur_dir_inode = &discos->inodes[current_dir_inode_num];
    // printf("<>current dir inode info: \n");
    // print_inode_info(current_dir_inode_num);
	// clear inode, clear blocks, clear bitmap, update free block number in superblock
    printf("Clearing inode...\n");
    // print_inode_info(file_inode_num);
	clear_inode(file_inode);

	// update the info in current dir inode
	// find file entry from the current directory
	
    // dir_entry_struct* entry_in_cur_dir = find_entry_in_current_dir(cur_dir_inode, filename);
    // memset(entry_in_cur_dir, 0, sizeof(dir_entry_struct));
    
    clear_entry_in_current_dir(cur_dir_inode, filename);
	
	
    cur_dir_inode->size -= 16;

	// free inode ++
	memset(file_inode, 0 , sizeof(inode_struct));
	discos->superblock.free_inodes++;


	free(current_dir);
	free(filename);

    printf("Unlink %s success! \n", pathname);
    // free(pathname);
	return 0;

}

/**
 * change the mode (i.e., access rights) of a file identified by the absolute pathname. 
 * Return 0 if successful or a negative value for an error.
*/
int rd_chmod(char *_pathname, unsigned int mode) {

    char* pathname = malloc(strlen(_pathname));
    strcpy(pathname, _pathname);
    printf("chmod %u %s...\n", mode, pathname);
	
	if ( strcmp(pathname, "/") == 0 ) {
		printf("<1> rd_chmod Error occurs: you attempt to chmod the root dir.\n");
		return -1;
	}

	// error occurs if the pathname does not exist
	int file_inode_num = find_inode_number(pathname);
	if ( file_inode_num == -1 ) {
		printf("<1> rd_chmod Error occurs: the pathname does not exist.\n");
		return -1;
	}

    inode_struct* file_inode = &discos->inodes[file_inode_num];
    file_inode->access = mode;

    return 0;

}


/**
 * open an existing file corresponding to pathname (which can be a regular or directory file) or 
 * report an error if file does not exist. 
 * When opening a file, you should return a file descriptor value 
 * that will index into the process' ramdisk file descriptor table. As stated earlier, 
 * this table entry will contain a pointer to a file object. 
 * You can assume the file object has status=flags (unless there is an error), 
 * and the file position is set to 0. 
 * An error can occur when opening a file if the file does not exist 
 * or if the flags value overrides the access rights when the file was created. 
 * For example, a process should not be allowed to open a file for writing if its access rights are read-only. 
 * Finally, you can assume that flags can be any one of READONLY, WRITEONLY, or READWRITE. 
 * Return a value of -1 to indicate an access error, or if the file does not exist.
 * 
*/
int rd_open(char *_pathname, unsigned int flags, int pid) {

    char* pathname = malloc(strlen(_pathname));
    strcpy(pathname, _pathname);
    printf("Opening %u %s...\n", flags, pathname);
	
	if ( strcmp(pathname, "/") == 0 ) {
		printf("<1> rd_open Error occurs: you attempt to open the root dir.\n");
		return -1;
	}

	// error occurs if the pathname does not exist
	int file_inode_num = find_inode_number(pathname);
    printf("<1> rd_open the file_inode_num: %d\n", file_inode_num);
	if ( file_inode_num == -1 ) {
		printf("<1> rd_open Error occurs: the pathname does not exist.\n");
		return -1;
	}
    
    inode_struct* file_inode = &discos->inodes[file_inode_num];
    if ( file_inode == NULL ) {
        printf("<1> rd_open: file_inode == NULL\n");
        return -1;
    }
    
    // check the access right
    if ( flags == RD && file_inode->access == WR ) {
        printf("<1> rd_open Error occurs: No access right. flags=%d, access=%d\n", flags, file_inode->access);
        return -1;
    }
    if ( flags == WR && file_inode->access == RD ) {
        printf("<1> rd_open Error occurs: No access right. flags=%d, access=%d\n", flags, file_inode->access);
        return -1;
    }
    if ( flags == RW && file_inode->access != RW ) {
        printf("<1> rd_open Error occurs: No access right. flags=%d, access=%d\n", flags, file_inode->access);
        return -1;
    }
    
    file_object* f_obj = create_file_object(pid);

    if ( f_obj == NULL ) {
        printf("Error: fd_table has been full.\n");
        return -1;
    }

    f_obj->cursor = 0;
    f_obj->inode_ptr = file_inode;
    f_obj->status = flags;
    free(pathname);
    return f_obj->pos;  // file descriptor value
}


file_object* create_file_object(int pid) {
    
    file_descriptor_table* f_objs = get_fd_table(pid);

    if ( f_objs == NULL ) {
        printf("File descriptor table is NULL.\n");
        return NULL;
    }

    int i;
    for ( i = 0; i < FD_TABLE_SIZE; i++ ) {
        if ( f_objs->file_objects[i].usable == 0 ) {
            f_objs->file_objects[i].pos = i;
            f_objs->file_objects[i].usable = 1;
            return &f_objs->file_objects[i];
        }
    }

    return NULL;
}


file_descriptor_table* get_fd_table(int pid) {
    int i;

    // find the pid
    for ( i = 0; i < THERAD_POOL_SIZE; i++ ) {
        if ( p_fd_table[i].pid == pid ) {
            return &p_fd_table[i].fd_table;
        }
    }

    // find an empty entry
    for ( i = 0; i < THERAD_POOL_SIZE; i++ ) {
        if ( p_fd_table[i].pid == 0 ) {
            return &p_fd_table[i].fd_table;
        }
    }

    return NULL;
}

/**
 * write up to num_bytes from the specified address in the calling process to 
 * a regular file identified by file descriptor, fd. 
 * You should return the actual number of bytes written, 
 * or -1 if there is an error. 
 * An error occurs 
 *      (1) if the value of fd refers either to a non-existent file or a directory file. 
 * If developing DISCOS, you may only have threads within a single shared address space, 
 * in which case you should identify a buffer region from which your data is written.
*/

int rd_write(int fd, int pid, char *data, int num_bytes) {

    // find the fd_table
    file_object* f_obj = NULL;
    
    file_descriptor_table* f_objs = get_fd_table(pid);
    if ( f_objs == NULL ) {
        printf("rd_write: File descriptor table is NULL.\n");
        return -1;  
    }
    
    int i;
    for ( i = 0; i < FD_TABLE_SIZE; i++ ) {
        if ( f_objs->file_objects[i].pos == fd ) {
            f_obj = &f_objs->file_objects[i];
            break;
        }
    }
    
    if ( f_obj == NULL ) {
        printf("rd_write: Cannot find this file object.\n");
        return -1;
    }
    
    inode_struct* inode = f_obj->inode_ptr;
    if ( inode == NULL ) {
        printf("rd_write: inode is null.\n");
        return -1;
    }
    if ( strcmp(inode->type, "dir\0") == 0 ) {
        printf("rd_write: Cannot write a directory!");
        return -1;
    }
    
    // access right check
    // WR RD WR
    if ( f_obj->status == RD || inode->access == RD ) {
        printf("rd_write: Access denied! f_obj->status is %d\n", f_obj->status);
        return -1;
    }
    
    // begin to write
    int written_bytes = 0;
    unsigned int cursor = f_obj->cursor;
    int seg = 0;
    int offset = 0;
    // get the last position
    seg = cursor / BLOCK_SIZE;
    offset = cursor % BLOCK_SIZE;

    /* write in the direct blocks */
    // seg will be less than eight if we are in direct blocks
    for ( i = seg; i < INODE_NUM_DIRECT_PTR; i++ ) {

        if ( inode->pointers[i] == NULL ) {
            // get a new block if this direct block is empty
            int free_block_num = get_free_block_num_from_bitmap(discos->bitmap);
            if ( free_block_num == -1 ) {
                printf("<1> rd_write: no free block found!\n");
                return -1;
            }
            inode->pointers[i] = allocate_data_block(free_block_num);
        }

        data_block_struct* direct_block = inode->pointers[i];
        int j;
        for ( j = offset; j < BLOCK_SIZE; j++ ) {
            if ( data[written_bytes] != '\0' ) {
                direct_block->data[j] = data[written_bytes];
                written_bytes += 1;
                f_obj->cursor += 1;
                inode->size += 1;   // update file size to update the seg and offset
                if ( written_bytes == num_bytes ) {
                    // finish 
                    printf("rd_write: written_byte=%d\n", written_bytes);
                    return written_bytes;
                }
            }
        }
    }

    // written_bytes is not zero here.
    seg = inode->size / BLOCK_SIZE;
    offset = inode->size % BLOCK_SIZE;
    // write in the single indirect blocks
    // single indirect blocks start from the 9th block
    if ( seg >= 8 && seg < (8+64) ) {
        data_block_struct* single_indirect = inode->single_indirect_ptrs;
        if ( single_indirect == NULL ) {
            printf("<1> rd_write: we need a new single_indirect poitner.\n");
            int free_block_num = get_free_block_num_from_bitmap(discos->bitmap);
            if ( free_block_num == -1 ) {
                printf("<1> rd_write: no free block found!\n");
                return -1;
            }
            inode->single_indirect_ptrs = allocate_data_block(free_block_num); 
            single_indirect = inode->single_indirect_ptrs;
        }
        
        // iterate the 64 index block
        seg = seg - 8;      // single indirect blocks start from the 9th block
        for ( i = seg; i < BLOCK_SIZE/4; i++ ) {
            data_block_struct* block = single_indirect->index_block[i];
            if ( block == NULL ) {
                int free_block_num = get_free_block_num_from_bitmap(discos->bitmap);
                if ( free_block_num == -1 ) {
                    printf("<1> rd_write: no free block found!\n");
                    return -1;
                }
                single_indirect->index_block[i] = allocate_data_block(free_block_num);
                block = single_indirect->index_block[i];
            }

            // begin to write
            int j;
            for ( j = offset; j < BLOCK_SIZE; j++ ) {
                if ( data[written_bytes] != '\0' ) {
                    block->data[j] = data[written_bytes];
                    written_bytes += 1;
                    f_obj->cursor += 1;
                    inode->size += 1;   // update file size to update the seg and offset
                    if ( written_bytes == num_bytes ) {
                        // finish 
                        printf("rd_write: written_byte=%d\n", written_bytes);
                        return written_bytes;
                    }
                }
            }
        }
    }

    seg = inode->size / BLOCK_SIZE;
    offset = inode->size % BLOCK_SIZE;
    // write in double indirect block
    if ( seg >= (8 + 64) && seg < (8 + 64 + 64*64) ) {

        data_block_struct* double_indirect = inode->double_indirect_ptrs;
        if ( double_indirect == NULL ) {
            printf("<1> rd_write: we need a new double_indirect pointer.\n");
            int free_block_num = get_free_block_num_from_bitmap(discos->bitmap);
            if ( free_block_num == -1 ) {
                printf("<1> rd_write: no free block found!\n");
                return -1;
            }
            inode->double_indirect_ptrs = allocate_data_block(free_block_num);
            double_indirect = inode->double_indirect_ptrs;
        }

        seg = seg - 8 - 64;

        // iterate the single indirect index blocks
        for ( i = 0; i < BLOCK_SIZE/4; i++ ) {
            data_block_struct* single_indirect = double_indirect->index_block[i];
            if ( single_indirect == NULL ) {
                printf("<1> rd_write: we need a new single_indirect poitner after double_indirect.\n");
                int free_block_num = get_free_block_num_from_bitmap(discos->bitmap);
                if ( free_block_num == -1 ) {
                    printf("<1> rd_write: no free block found!\n");
                    return -1;
                }
                inode->double_indirect_ptrs->index_block[i] = allocate_data_block(free_block_num);
                single_indirect = inode->double_indirect_ptrs->index_block[i];
            }
            
            seg = seg/64;
            int j;
            for ( j = seg; j < BLOCK_SIZE/4; j++ ) {
                data_block_struct* block = single_indirect->index_block[j];
                if ( block == NULL ) {
                    int free_block_num = get_free_block_num_from_bitmap(discos->bitmap);
                    if ( free_block_num == -1 ) {
                        printf("<1> rd_write: no free block found!\n");
                        return -1;
                    }
                    single_indirect->index_block[j] = allocate_data_block(free_block_num);
                    block = single_indirect->index_block[j];
                }

                // begin to write
                int k;
                for ( k = offset; k < BLOCK_SIZE; k++ ) {
                    if ( data[written_bytes] != '\0' ) {
                        block->data[k] = data[written_bytes];
                        written_bytes += 1;
                        f_obj->cursor += 1;
                        inode->size += 1;
                        if ( written_bytes == num_bytes ) {
                            printf("rd_write: written_byte=%d\n", written_bytes);
                            return written_bytes;
                        }
                    }
                }
            }
        }
    }

    if ( seg >= (8 + 64 + 64*64) ) {
        printf("<1> rd_write: MAXIMUM FILE LIMIT!\n");
        return written_bytes;
    }

    return 0;   // the actual number of byte written

}


/**
 * set the file object's file position identified by file descriptor, fd, to offset, 
 * returning 
 *      the new position, 
 *      or the end of the file position if the offset is beyond the file's current size. 
 *      -1 to indicate an error, if applied to directory files, 
 *      else 0 to indicate success.
*/
int rd_lseek(int _fd, int offset, int pid) {

    // find the fd_table
    file_object* f_obj = NULL;
    
    file_descriptor_table* f_objs = get_fd_table(pid);
    if ( f_objs == NULL ) {
        printf("rd_lseek: File descriptor table is NULL.\n");
        return -1;  
    }
    
    int i;
    for ( i = 0; i < FD_TABLE_SIZE; i++ ) {
        if ( f_objs->file_objects[i].pos == _fd ) {
            f_obj = &f_objs->file_objects[i];
            break;
        }
    }
    
    if ( f_obj == NULL ) {
        printf("rd_lseek: Cannot find this file object.\n");
        return -1;
    }
    
    inode_struct* inode = f_obj->inode_ptr;
    if ( inode == NULL ) {
        printf("rd_lseek: inode is null.\n");
        return -1;
    }

    if ( f_obj->usable != 1 ) {
        printf("rd_lseek: Cnnot find file object. usable != 1 \n");
        return -1;
    }

    if ( strcmp(inode->type, "dir\0") == 0 ) {
        printf("rd_lseek: Cannot lseek a directory!");
        return -1;
    }

    if ( offset < 0 ) {
        printf("rd_lseek: offset < 0 \n");
        return -1;
    }

    unsigned int file_size = inode->size; 
    if ( offset >= file_size ) {
        // return end of file
        printf("rd_lseek: return end of file.\n");
        f_obj->cursor = file_size - 1;
        return file_size - 1;
    }

    f_obj->cursor = offset;
    printf("rd_lseek: change position to %d\n", f_obj->cursor);
    return f_obj->cursor;
}


/**
 * read up to num_bytes from a regular file identified by file descriptor, fd, into a process' location at address. 
 * You should return 
 *      the number of bytes actually read, 
 *      else -1 if there is an error. 
 * An error occurs 
 *      if the value of fd refers either to a non-existent file or a directory file. 
 * If developing DISCOS, you may only have threads within a single shared address space, 
 * in which case you should identify a buffer region into which your data is read.
*/
int rd_read(int _fd, char *data, int num_bytes, int pid) {
    
    /* preprocess: find this file object first */

    // find the fd_table
    file_object* f_obj = NULL;
    
    file_descriptor_table* f_objs = get_fd_table(pid);
    if ( f_objs == NULL ) {
        printf("rd_read: File descriptor table is NULL.\n");
        return -1;  
    }
    
    int i;
    for ( i = 0; i < FD_TABLE_SIZE; i++ ) {
        if ( f_objs->file_objects[i].pos == _fd ) {
            f_obj = &f_objs->file_objects[i];
            break;
        }
    }
    
    if ( f_obj == NULL ) {
        printf("rd_read: Cannot find this file object.\n");
        return -1;
    }
    
    inode_struct* inode = f_obj->inode_ptr;
    if ( inode == NULL ) {
        printf("rd_read: inode is null.\n");
        return -1;
    }

    if ( f_obj->usable != 1 ) {
        printf("rd_read: Cnnot find file object. usable != 1 \n");
        return -1;
    }

    if ( strcmp(inode->type, "dir\0") == 0 ) {
        printf("rd_read: Cannot read a directory!");
        return -1;
    }


    /* preprocess finish */

    // TODO: read



    return 0;   // return the number of bytes actually read

}


/**
 * close the corresponding file descriptor 
 * and release the file object matching the value returned from a previous rd_open(). 
 * Return 0 on success and -1 on error. 
 * An error occurs if fd refers to a non-existent file.
*/
int rd_close(int _fd, int pid) {

    /* preprocess: find this file object first */

    // find the fd_table
    file_object* f_obj = NULL;
    
    file_descriptor_table* f_objs = get_fd_table(pid);
    if ( f_objs == NULL ) {
        printf("rd_close: File descriptor table is NULL.\n");
        return -1;  
    }
    
    int i;
    for ( i = 0; i < FD_TABLE_SIZE; i++ ) {
        if ( f_objs->file_objects[i].pos == _fd ) {
            f_obj = &f_objs->file_objects[i];
            break;
        }
    }
    
    if ( f_obj == NULL ) {
        printf("rd_close: Cannot find this file object.\n");
        return -1;
    }
    
    inode_struct* inode = f_obj->inode_ptr;
    if ( inode == NULL ) {
        printf("rd_close: inode is null.\n");
        return -1;
    }

    if ( f_obj->usable != 1 ) {
        printf("rd_close: Cnnot find file object. usable != 1 \n");
        return -1;
    }

    if ( strcmp(inode->type, "dir\0") == 0 ) {
        printf("rd_close: Cannot close a directory!");
        return -1;
    }


    /* preprocess finish */


    // release the file object
    if ( f_obj->usable == 1 ) {
        f_obj->inode_ptr = NULL;
        memset(f_obj, 0, sizeof(file_object));
        printf("rd_close: release object.\n");
        return 0;
    }

    printf("<1> rd_close: ERROR!\n");
    return -1;
}

