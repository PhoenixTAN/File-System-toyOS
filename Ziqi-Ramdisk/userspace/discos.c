#include "discos.h"

filesys_struct* discos;

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


int main(void) {

	// cmd_daemon();
	printf("\n\n\n Discos ##########\n");
	// kmalloc()

	/* Data structure size check */
	printf("superblock %d\n", sizeof(superblock_struct));
	printf("dir entry %d\n", sizeof(dir_entry_struct));
	printf("data block %d\n", sizeof(data_block_struct));
	printf("single_indirect %d\n", sizeof(single_indirect_struct));
	printf("double_indirect %d\n", sizeof(double_indirect_struct));
	printf("inode %d\n", sizeof(inode_struct));
	printf("filesys %d\n", sizeof(filesys_struct));

	// /* malloc for Discos 2MB */
	// filesys_struct *discos = (filesys_struct*)malloc(sizeof(filesys_struct));
	// if ( !discos ) {
	// 	printf("discos malloc fail.\n");
	// }
	// else {
	// 	printf("discos 2MB created! Enjoy!\n");
	// }
	init_file_sys();
	free(discos);

	return 0;
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
	retval = rd_mkdir("/");
	return retval;
}

int rd_mkdir(char* pathname) {
	if(strcmp(pathname, "/") == 0 && discos->superblock.free_inodes == MAX_NUM_FILE) {
		printf("init root dir\n");
		discos->superblock.free_inodes--;
		return 0;
	}
	if(strcmp(pathname, "/") == 0 && discos->superblock.free_inodes != MAX_NUM_FILE) {
		printf("root dir exist!");
		return -1;
	}
	
}
/**
 * int rd_creat(char *pathname, mode_t mode)
 * 		create a regular file with absolute pathname and mode 
 * 		from the root of the directory tree, 
 * 		where each directory filename is delimited by a "/" character. 
 * 		The mode can be read-write (default), read-only, or write-only . 
 * 		You can assume that any process opening an existing file 
 * 		is restricted by the access rights at creation time. 
 * 		On success, you should return 0, 
 * 		else if the file corresponding to pathname already exists 
 * 		you should return -1, indicating an error. 
 * 		Note that you need to update the parent directory file, 
 * 		to include the new entry.
 * 
 * 		step 1.
 * 			从i-node array找到一个free inode.
 * 			如何判断是不是free?
 * 			
 * 		step 2. 对于这个inode
 * 			遍历block bitmap,找到第一个free data blocks
 * 			update type
 * 			size = 256 byte (1 block)
 * 			update the direct pointers. 
 * 			access right
 * 		step 3.
 * 			判断同一层是否有同名文件或文件夹。
 * 		
 * 
 * 		return 0;
 * 
*/


/**
 * 
 * 
 * write
 * 
 * 看看是不是满了：1.使用single-indirect
 * 								   2. 使用
*/

/**
 * mkdir:
 * 创建根目录：
 * 解析：
 * 1. 得到要创建的dir name：从后往前找到第一个“/”，将字符串截取两节，第二节是dir name
 * 2. 处理第一节字符串：默认从根目录开始找，遍历根目录node array的第一个entry（包括一级二级指针）直到找到目标name
 * 
 1. 找到一个inode，占用inode，不占用datablock，也不需要记录根目录的名称，因为根目录在全局是唯一的。
 2. 在根目录下mkdir, 找到根目录的inode，在inode中将data block ptr分配给free的data block
 ，这个datablock用dir_entry装，name[14]装名字



在根目录创建文件，找到根目录inode，根目录inode直接指针指向一个free_block，
占用这个free_block, name[14]装名字，inode_num指向一个新的inode，
新inode指向一个新的free data block，并占用这个data block，
新inode文件size设置成1.

pwd
ls
cd 
cd ..
 */