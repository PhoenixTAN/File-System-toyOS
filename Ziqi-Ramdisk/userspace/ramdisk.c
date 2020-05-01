#include "ramdisk.h"


void cmd_daemon() {
	char cmd[CMD_MAX_LENGTH] = "Hello Ramdisk!\n";

	printf("%s", cmd);

	while ( 1 ) {
		printf("Ramdisk# ");

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
	
	// kmalloc()

	/* Data structure check */
	printf("");
	printf("dir entry t %d\n", sizeof(dir_entry_t));
	printf("block %d\n", sizeof(block));
	return 0;
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

