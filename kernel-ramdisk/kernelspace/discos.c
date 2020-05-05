#include "discos.h"

/* global variables */
filesys_struct* discos;

void print_block_info(int index) {
    printk("Block %d info: \n", index);
    data_block_struct* block = &discos->data_blocks[index];
    int i;
    for ( i = 0; i < BLOCK_SIZE/16; i++ ) {
        printk("   Entry %d,  entry->name: %s, inode_num: %u\n", i, block->entries[i].name, block->entries[i].inode_num);
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
    // printk("byte i=%d\n", free_byte);
    // printk("free_byte_pos=%d\n", free_byte_pos);
    // iterate a byte bit by bit
    int free_bit;
    for ( i = 0, free_bit = free_byte_pos * 8; i < 8; i++ ) {
        if ( (free_byte & (unsigned char)1 ) == 0 ) {
            free_bit += i;
            break;
        }
        free_byte = free_byte >> 1;
    }
    // printk("free_bit: %d\n", free_bit);
    // check it out whether it is greater than 7931-1
    if ( free_bit >= 7931 ) {
        return -1;
    }

    return free_bit;
}


int init_file_sys() {
	int retval;
	discos = (filesys_struct*)vmalloc(sizeof(filesys_struct));
	if ( !discos ) {
		printk("discos malloc fail.\n");
		return -1;
	}
    memset(discos, 0, RAMDISK_SIZE);
    printk("\n\nDiscos ##########\n");

	// Data structure size check
    printk("<1> Data structure size check:\n");
	printk("   superblock %d\n", sizeof(superblock_struct));
	printk("   dir entry %d\n", sizeof(dir_entry_struct));
	printk("   data block %d\n", sizeof(data_block_struct));
	printk("   inode %d\n", sizeof(inode_struct));
	printk("   filesys %d\n", sizeof(filesys_struct));
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
    printk("rd_mkdir strlen(pathname): %d\n", len);
	pwd = vmalloc(len);
    memset(pwd, 0, len);
	filename = vmalloc(len);
    memset(filename, 0, len);
	if(strcmp(pathname, "/") == 0 && discos->superblock.free_inodes == MAX_NUM_FILE) {
		printk("init root dir\n");
		discos->superblock.free_inodes--;
        strcpy(discos->inodes[0].type, "dir\0");
		return 0;
	}
	if(strcmp(pathname, "/") == 0 && discos->superblock.free_inodes != MAX_NUM_FILE) {
		printk("root dir exist!\n");
		return -1;
	}

    /* modify 23:22 */
    int ret = rd_create(pathname, "dir\0", 1);
	vfree(pwd);
	vfree(filename);

    if ( ret == 0 ) {
        printk("<1> file/directory created successfully !!!!\n");
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
int my_find_inode_number(char* pathname) {
    
    const char* delim = "/";

    char* temp_pathname, *found;
 	temp_pathname = vmalloc(strlen(pathname));
    memset(temp_pathname, 0, strlen(pathname));
	strcpy(temp_pathname, pathname);
	// temp_pathname = vmalloc(strlen(pathname));
	// strcpy(temp_pathname, pathname);
    // printk("deep copy path: %s\n", temp_pathname);

    // char* split_string = strsep(&temp_pathname, delim);
    // printk("delete the first delim temp_pathname: %s\n", temp_pathname);
    // printk("delete the first delim split_string: %s\n", split_string);

    if(strcmp(temp_pathname, delim) == 0) {
        return 0;
    }

    temp_pathname++;


    if ( temp_pathname == NULL ) {
        return 0;
    }

	int pre_node_num = 0;
	int cur_node_num = 0;

	data_block_struct* cur_data_block;
    // ret = rd_mkdir("/folderA/folderB/folderC"); 
	while((found = strsep(&temp_pathname,delim)) != NULL) {
		printk("looking for dir: %s\n", found);
		
        int i;
		// directory entry
		for ( i = 0; i < INODE_NUM_DIRECT_PTR; i++ ) {
		    cur_data_block = discos->inodes[cur_node_num].pointers[i];
			// if cur_data_block is null, return cur_node_number, pathname = pathname
            if ( cur_data_block != NULL ) {
                // printk("cur_data_block != NULL\n");
                int j;
                for ( j = 0; j < BLOCK_SIZE/sizeof(dir_entry_struct); j++ ) {
                    dir_entry_struct* entry = &cur_data_block->entries[j];
                    if ( entry != NULL && strcmp(entry->name, found) == 0 ) {
                        cur_node_num = entry->inode_num;
                        break;
                    }
                }
            }

            if ( cur_node_num != pre_node_num ) {
                // which means we have found the current dir_entry
                printk("which means we have found the current dir_entry in direct block.\n");
                break;
            }
		}

        // you find the current dir in the direct block
		if ( cur_node_num != pre_node_num ) {
			pre_node_num = cur_node_num;
            // split_string = strsep(NULL, delim);
            printk("To be continued 1... \n");
			continue;   // to find the next level directory
		}

        // we cannot find the dir_entry in the first 8 direct entry
        // directory does not exits

        printk("Looking for %s in single indirect\n", found);
		/* try to find current dir in single indirect block */
        data_block_struct* single_indirect_ptr = discos->inodes[cur_node_num].single_indirect_ptrs;
        // NULL pointer exception
        if ( single_indirect_ptr != NULL ) {
            for( i = 0; i < BLOCK_SIZE/4; i++ ) {
                // this data block contains 64 block pointers (see as index_block[] in the data struct)
                cur_data_block = single_indirect_ptr->index_block[i];   
                if ( cur_data_block != NULL ) {
                    // printk("cur_data_block != NULL\n");
                    int j;
                    for ( j = 0; j < BLOCK_SIZE/16; j++ ) {
                        dir_entry_struct* entry = &cur_data_block->entries[j];
                        if ( entry == NULL ) {
                            printk("entry == NULL\n");
                        }
                        // printk("entry->name: %s    split_string: %s\n", entry->name, split_string);
                        if ( entry != NULL && strcmp(entry->name, found) == 0 ) {
                            cur_node_num = entry->inode_num;
                            printk("Single indirect: i=%dth pointer, j=%dth entry\n", i, j);
                            break;
                        }
                    }
                }
                if ( cur_node_num != pre_node_num ) {
                    // which means we have found the current dir_entry
                    printk("which means we have found the current dir_entry in the single indrect block.\n");
                    break;
                }
            }
        }		

        if ( cur_node_num != pre_node_num ) {
			pre_node_num = cur_node_num;
            // split_string = strsep(NULL, delim);
            printk("To be continued 2... \n");
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
                                if ( entry != NULL && strcmp(entry->name, found) == 0 ) {
                                    cur_node_num = entry->inode_num;
                                    printk("Double indirect: k=%dth double ptr, i=%dth single ptr, j=%dth entry\n", k, i, j);
                                    break;
                                }
                            }
                        }
                        if ( cur_node_num != pre_node_num ) {
                            // which means we have found the current dir_entry
                            printk("which means we have found the current dir_entry in the single indrect block.\n");
                            break;
                        }
                    }
                }
                if ( cur_node_num != pre_node_num ) {
                    // which means we have found the current dir_entry
                    printk("which means we have found the current dir_entry in the single indrect block.\n");
                    break;
                }
            }
        }

        if ( cur_node_num != pre_node_num ) {
			pre_node_num = cur_node_num;
            // split_string = strsep(&temp_pathname, delim);
            printk("To be continued 3... \n");
			continue;   // to find the next level directory
		}
        
        // which mean you cannot find the dir
        return -1;

	}

	vfree(temp_pathname);

    return cur_node_num;
}


/* clear the bit of loc to be zero in the bitmap */
int clear_bitmap(unsigned char* map, int index) {
    // location should be in range [0, 7931)
    if( index < 0 || index > 7931 ) {
        printk("location illeagal!\n");
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
        printk("location illeagal!\n");
        return -1;
    }

    int seg = index / 8;
    int offset = index % 8;

    map[seg] = map[seg] | ( (unsigned char)1 << offset );
    
    return 0;
}   


void print_bitmap (unsigned char* map) {
    printk("Printing bit map: \n");
    int i;
    for ( i = 0; i < 1024; i++ ) {
        printk("%u ", map[i]);
    }
    printk("\n\n");
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
    printk("setting bitmap: free_block_number: %d\n", free_block_number);
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
int rd_create(char *pathname, char* type, int mode) {
    
    printk("Creating file/directory ...\n");

    if ( discos->superblock.free_inodes <= 0 ) {
        printk("File/Directory failed: not enough free inodes.\n");
        return -1;
    }

    if ( discos->superblock.free_blocks <= 0 ) {
        printk("File/Directory failed: not enough free blocks.\n");
        return -1;
    }
    
    // find the current directory
    // get the file/directory name you want to create
    char *current_path = vmalloc(strlen(pathname));
    char *entry_name = vmalloc(strlen(pathname));
    memset(current_path, 0, strlen(pathname));
    memset(entry_name, 0, strlen(pathname));

    parse_absolute_path(pathname, current_path, entry_name);
    printk("parse: current path: %s    entry_name: %s\n", current_path, entry_name);

    // find the inode of the current directory
    int cur_inode_num = my_find_inode_number(current_path);
    if ( cur_inode_num == -1 ) {
        printk("Cannot find such a directory.\n");
        return -1;
    }
    printk("find the inode number of current dir: %d\n", cur_inode_num);
    inode_struct* current_path_inode = &discos->inodes[cur_inode_num];

    // find the next free entry in current directory
    dir_entry_struct* free_dir_entry = get_next_entry(current_path_inode);
    if ( free_dir_entry == NULL ) {
        printk("No free entry!\n");
        return -1;
    }

    int free_inode_num = get_free_inode_num();
    if ( free_inode_num == -1 ) {
        printk("File/Directory failed: cannot find free inodes.\n");
        return -1;
    }
    printk("get a free inode number: %d\n", free_inode_num);
    inode_struct* free_inode = allocate_inode( free_inode_num );
    
    
    strcpy(free_dir_entry->name, entry_name);
    printk("strcpy free_dir_entry->name: %s    entry_name: %s\n", free_dir_entry->name, entry_name);

    printk("free inode number: %d\n", free_inode_num);
    free_dir_entry->inode_num = (unsigned short)free_inode_num;

    printk("inode type: %s\n", type);
    strcpy(free_inode->type, type);

    /* update size */
    current_path_inode->size += 16;

    vfree(current_path);
    vfree(entry_name);
    
    return 0;
}


/* get_next_dir_entry in direct block*/
dir_entry_struct* get_next_dir_entry(data_block_struct* block) {
    int j;
    for ( j = 0; j < BLOCK_SIZE/16; j++ ) {
        if ( block->entries[j].inode_num == 0 ) {
            printk("get next dir entry: return block->entries[%d]\n", j);
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
    printk("<>block seg= %d\n", seg);

    if ( block == NULL ) {
        int free_block_num = get_free_block_num_from_bitmap(discos->bitmap);
        if ( free_block_num == -1 ) {
                printk("Cannot find free blocks.\n");
                return NULL;
        }
        // we find a new block
        printk("free block number: %d, single index_block[%d]\n", free_block_num, seg);

        data_block_struct* new_data_block = allocate_data_block( free_block_num );
        printk("After allocate datablock: \n");
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
                printk("Cannot find free blocks.\n");
                return NULL;
        }
        // we find a new block
        printk("free block number: %d, double index_block[%d]\n", free_block_num, seg);
        data_block_struct* new_index_block = allocate_data_block( free_block_num );
        printk("After allocate datablock: \n");
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
    printk("last block that is not full: node->size %d / BLOCK_SIZE = %d\n", node->size, seg);
    
    /* find the block that has not been full in direct block */
    int i;
    for ( i = seg; i < INODE_NUM_DIRECT_PTR; i++ ) {
        data_block_struct* dir_ptr = node->pointers[i];
        // this block is empty
        if ( dir_ptr == NULL ) {
            printk("We need a new block.\n");
            // print_bitmap(discos->bitmap);
            int free_block_num = get_free_block_num_from_bitmap(discos->bitmap);
            if ( free_block_num == -1 ) {
                 printk("Cannot find free blocks.\n");
                 return NULL;
            }
            // we find a new block
            printk("free block number: %d, pointers[%d]\n", free_block_num, i);

            data_block_struct* new_data_block = allocate_data_block( free_block_num );
            printk("After allocate datablock: \n");
            // print_bitmap(discos->bitmap);
            /* !! update ptr */
            node->pointers[i] = new_data_block;
            return &node->pointers[i]->entries[0];
        }

        // this block has not been full yet
        // then we get the next free entry in this block
        dir_entry_struct* free_dir_entry = get_next_dir_entry(dir_ptr);
        if ( free_dir_entry != NULL ) {
            printk("return free_dir_entry\n");
            return free_dir_entry;
        }
    }

    /* single indirect block */
    // seg should be in range [8,72)
    if ( seg >= 8 && seg < (8 + 64) ) {
        if ( node->single_indirect_ptrs == NULL ) {
            printk("Single indirect (NULL). We need a new block.\n");
            
            // print_bitmap(discos->bitmap);
            int free_block_num = get_free_block_num_from_bitmap(discos->bitmap);
            if ( free_block_num == -1 ) {
                 printk("Cannot find free blocks.\n");
                 return NULL;
            }
            // we find a new block
            printk("free block number: %d\n", free_block_num);

            data_block_struct* new_index_block = allocate_data_block( free_block_num );
            printk("After allocate datablock: \n");
            // print_bitmap(discos->bitmap);
            /* !! update ptr */
            node->single_indirect_ptrs = new_index_block;
        }

        // this block has not been full yet
        // then we get the next free entry in this block
        dir_entry_struct* free_dir_entry = get_next_dir_entry_single(node->single_indirect_ptrs, seg - 8);
        if ( free_dir_entry != NULL ) {
            printk("return free_dir_entry\n");
            return free_dir_entry;
        }
        /*************modify****/
        seg = 72;
    }

    /* double indrect block */
    if ( seg >= (8 + 64) && seg < (8 + 64 + 64*64) ) {
        if ( node ->double_indirect_ptrs ==  NULL ) {
            printk("Double indirect (NULL). We need a new block.\n");
            int free_block_num = get_free_block_num_from_bitmap(discos->bitmap);
            if ( free_block_num == -1 ) {
                 printk("Cannot find free blocks.\n");
                 return NULL;
            }
            // we find a new block
            printk("free block number: %d\n", free_block_num);

            data_block_struct* new_index_block = allocate_data_block( free_block_num );
            printk("After allocate datablock: \n");
            // print_bitmap(discos->bitmap);
            /* !! update ptr */
            node->double_indirect_ptrs = new_index_block;
        }

        dir_entry_struct* free_dir_entry = get_next_dir_entry_double(node->double_indirect_ptrs, seg - 72);
        if ( free_dir_entry != NULL ) {
            printk("return free_dir_entry\n");
            return free_dir_entry;
        }

    }

    if ( seg >= (8 + 64 + 64*64) ) {
        printk("Over 8 + 64 + 64*64\n");
    }

    return NULL;
}


/* clear inode, free datablocks */
void my_clear_inode(inode_struct* inode) {

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

    printk("Here!\n");

	if ( inode->single_indirect_ptrs != NULL ) {
		clear_inode_single_indirect(inode->single_indirect_ptrs);
	}

	if ( inode->double_indirect_ptrs != NULL ) {
		clear_inode_double_indirect(inode->double_indirect_ptrs);
	}

}


/* helper function for clear_inode */
void clear_inode_single_indirect(data_block_struct* single_indirect) {
    printk("clear_inode_single_indirect.\n");
	int i;
	for ( i = 0; i < BLOCK_SIZE/4; i++ ) {
		data_block_struct* block = single_indirect->index_block[i];
		if (  block != NULL ) {	
			clear_bitmap(discos->bitmap, block - &discos->data_blocks[0]);
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
	char* pathname = vmalloc(strlen(_pathname));
    memset(pathname, 0, strlen(_pathname));
    strcpy(pathname, _pathname);
    printk("Unlinking %s ...\n", pathname);
	// error occurs if you attempt to unlink the root directory file.
	if ( strcmp(pathname, "/") == 0 ) {
		printk("<1> rd_unlink Error occurs: you attempt to unlink the root dir.\n");
		return -1;
	}

	// error occurs if you attempt to unlink an open file
	// TODO:

	// error occurs if the pathname does not exist
	int file_inode_num = my_find_inode_number(pathname);
	if ( file_inode_num == -1 ) {
		printk("<1> rd_unlink Error occurs: the pathname does not exist.\n");
		return -1;
	}
	inode_struct* file_inode = &discos->inodes[file_inode_num];
	
	// error occurs if you attempt to unlink a non-empty directory file
	if ( strcmp(file_inode->type, "dir\0") == 0 ) {
		// check whether this is a non-empty folder
		// 这里要什么复杂的操作吗
		if ( file_inode->size > 0 ) {
			printk("<1> rd_unlink Error occurs: you attempt to unlink the root dir.\n");
			return -1;
		}
	}

	char* current_dir = vmalloc(strlen(pathname));
	char* filename = vmalloc(strlen(pathname));
    memset(current_dir, 0, strlen(pathname));
    memset(filename, 0, strlen(pathname));
	parse_absolute_path(pathname, current_dir, filename);

	int current_dir_inode_num = my_find_inode_number(current_dir);
	if ( current_dir_inode_num == -1 ) {
		printk("<1> rd_unlink Error occurs: cannot find current dir.\n");
		return -1;
	}
	inode_struct* cur_dir_inode = &discos->inodes[current_dir_inode_num];

	// clear inode, clear blocks, clear bitmap, update free block number in superblock
    printk("Clearing inode...\n");
	my_clear_inode(file_inode);

	// update the info in current dir inode
	// find file entry from the current directory
	dir_entry_struct* entry_in_cur_dir = find_entry_in_current_dir(cur_dir_inode, filename);
	memset(entry_in_cur_dir, 0, sizeof(dir_entry_struct));
	cur_dir_inode->size -= 16;

	// free inode ++
	memset(file_inode, 0 , sizeof(inode_struct));
	discos->superblock.free_inodes++;

    printk("Unlink %s success! \n", pathname);

    vfree(pathname);
	vfree(current_dir);
	vfree(filename);

	return 0;

}

void cleanup_fs() {
    vfree(discos);
}

