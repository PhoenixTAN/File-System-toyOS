#include "discos.h"

/*
在根目录创建文件，找到根目录inode，根目录inode直接指针指向一个free_block，
占用这个free_block, name[14]装名字，inode_num指向一个新的inode，
新inode指向一个新的free data block，并占用这个data block，
新inode文件size设置成1.

pwd
ls
cd 
cd ..
*/

filesys_struct* discos;


int main(void) {

    char str[] ="- This, a sample string.";
    char * pch;
    printf ("Splitting string \"%s\" into tokens:\n",str);
    pch = strtok (str," ,.-");
    while (pch != NULL)
    {
        printf ("%s\n",pch);
        pch = strtok (NULL, " ,.-");
    }

    char bitmap[1024];
    int i;
    int pos;
    printf("bitmap size: %d\n", sizeof(bitmap));
    // output 0
    for ( i = 0; i < 1024; i++ ) {
        bitmap[i] = 255;
    }
    // bitmap[2023] = 247;
    pos = get_free_block_num_from_bitmap(bitmap);
    printf("pos: %d\n", pos);   // 1111 0111


    char s[8];
    printf("str size: %d\n", sizeof(s));
    printf("strlen: %d\n", strlen(s));
    for ( i = 0; i < 8; i++ ) {
        s[i] = 255;
    }
    for ( i = 0; i < strlen(s); i++ ) {
        printf("%d ", s[i]);
    }

	printf("\n\nDiscos ##########\n");

	/* Data structure size check */
    printf("Data structure size check:\n");
	printf("  superblock %d\n", sizeof(superblock_struct));
	printf("  dir entry %d\n", sizeof(dir_entry_struct));
	printf("  data block %d\n", sizeof(data_block_struct));
	printf("  single_indirect %d\n", sizeof(single_indirect_struct));
	printf("  double_indirect %d\n", sizeof(double_indirect_struct));
	printf("  inode %d\n", sizeof(inode_struct));
	printf("  filesys %d\n", sizeof(filesys_struct));

    /* initialize file system */
	init_file_sys();

    /* test mkdir */
    int ret;
    ret = rd_mkdir("/");    // output root exits
    ret = rd_mkdir("/folder1"); 
    ret = rd_mkdir("/folderA/folderB"); 
    ret = rd_mkdir("/folderA/folderB/folderC"); 

	free(discos);

	return 0;
}

/* get the free block number from bitmap 
    return the number of the free data block.
    If there is no data block, return -1.
*/
int get_free_block_num_from_bitmap(unsigned char* map) {
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
		return 0;
	}
	if(strcmp(pathname, "/") == 0 && discos->superblock.free_inodes != MAX_NUM_FILE) {
		printf("root dir exist!\n");
		return -1;
	}
	parse_absolute_path(pathname, pwd, filename);
	printf("after parse absolute path: pwd:%s   filename: %s\n", pwd, filename);
	
    int cur_dir_node;   // 初始化习惯？为啥输出26，我人傻了
	printf("Cur_dir_node: %d\n", cur_dir_node);
	
    cur_dir_node = find_inode_number(pwd);
	printf("Cur_dir_node: %d\n", cur_dir_node);

    if( cur_dir_node == -1 ) {
		printf("No such file.\n");
		free(pwd);
		free(filename);
		return -1;
	}

    // call create_file()

	// printf("cur_node: %d\n", cur_dir_node);
	free(pwd);
	free(filename);
	return 0;
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
		printf("%s\n", split_string);
		
        int i;
		// directory entry
		for ( i = 0; i < INODE_NUM_DIRECT_PTR; i++ ) {
		    cur_data_block = discos->inodes[cur_node_num].pointers[i];
			// if cur_data_block is null, return cur_node_number, pathname = pathname
            if ( cur_data_block != NULL ) {
                int j;
                for ( j = 0; j < BLOCK_SIZE/sizeof(dir_entry_struct); j++ ) {
                    dir_entry_struct* entry = &cur_data_block->entries[j];
                    if ( entry != NULL && strcmp(entry->name, split_string) == 0 ) {
                        cur_node_num = cur_data_block->entries[j].inode_num;
                        break;
                    }
                }
            }

            if ( cur_node_num != pre_node_num ) {
                // which means we have found the current dir_entry
                break;
            }
		}

		if ( cur_node_num != pre_node_num ) {
			pre_node_num = cur_node_num;
            split_string = strtok(NULL, delim);
			continue;   // to find the next level directory
		}

        // we cannot find the dir_entry in the first 8 direct entry
        // directory does not exits
        return -1;

        /*
		//single directory entry
		for(i = 0; i < BLOCK_SIZE/4; i++) {
			int j;
			cur_data_block = discos->inodes[cur_node_num].single_indirect_ptrs;
			if(cur_data_block == NULL) {
				return cur_node_num;
			}
			for(j = 0; j < BLOCK_SIZE/4; j++) {
				int k;
				cur_data_block = discos->inodes[cur_node_num].single_indirect_ptrs->index_block[j];
				if(cur_data_block == NULL) {
					return cur_node_num;
				}
				for(k = 0; k < BLOCK_SIZE/sizeof(dir_entry_struct); k++) {
					if(strcmp(cur_data_block->index_block[j]->entries[k].name, split_string) == 0) {
						cur_node_num = cur_data_block->index_block[j]->entries[k].inode_num;
						break;
					}
				}
				if(cur_node_num) {

                }
			}
		}
        */

        // split_string = strtok(NULL, delim);
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

    map[seg] = map[seg] & ( (unsigned char)1 << offset );
    
    return 0;
}   

// get free inode
int get_free_inode() {
    int i;
    for(i = 0; i < MAX_NUM_FILE; i++) {
        inode_struct temp_inode = discos->inodes[i];
        if(strcmp(temp_inode.type, "")==0) {
            return i;
        }
    }
    return -1;
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

    int free_inode_num = get_free_inode();
    if ( free_inode_num == -1 ) {
        printf("File/Directory failed: cannot find free inodes.\n");
        return -1;
    }
    inode_struct* free_inode = &discos->inodes[free_inode_num];
/*
    int free_block_num = get_free_block_num_from_bitmap(discos->bitmap);
    if ( free_block_num == -1 ) {
        printf("File/Directory failed: cannot find free blocks.\n");
        return -1;
    }
    dir_entry_struct* free_dir_entry = &discos->data_blocks[free_block_num];
*/
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

    inode_struct* current_path_inode = &discos->inodes[cur_inode_num];

    // find the next free entry in current directory

    // if there is no more free entry, get a new block.



    return 0;
}

dir_entry_struct* get_next_free_dir_entry(inode_struct* node) {
    int i;
    for(i = 0; i < INODE_NUM_DIRECT_PTR; i++) {
        data_block_struct* dir_ptr = node->pointers[i];
        int j;
        // directory entry
        for(j = 0; j < BLOCK_SIZE/sizeof(dir_entry_struct); j++) {
            if(strcmp(dir_ptr->entries[j].name, "") == 0) {
                return &(dir_ptr->entries[j]);
            }
        }
    }
     return NULL;
}