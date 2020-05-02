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


int main ()
{
    char str[] ="- This, a sample string.";
    char * pch;
    printf ("Splitting string \"%s\" into tokens:\n",str);
    pch = strtok (str," ,.-");
    while (pch != NULL)
    {
        printf ("%s\n",pch);
        pch = strtok (NULL, " ,.-");
    }
    return 0;
}


data_block_struct* get_free_block_from_bitmap(char* map, filesys_struct* filesys);
char* get_pwd();

/* get the free block number from bitmap 
    return the number of the free data block.
    If there is no data block, return -1.
*/
int get_free_block_num_from_bitmap(char* map) {
    // map: char bitmap[BITMAP_SIZE*BLOCK_SIZE]; 8192 bits in all
    // the first 7931 blocks are usable

    // find the first 0 bit in bit map
    int i;
    int free_byte_pos;
    char free_byte;
    // iterate bit map byte by byte
    for ( i = 0; i < BITMAP_SIZE*BLOCK_SIZE; i++ ) {
        char ch = map[i];
        if ( ch != 255 )  {
            free_byte_pos = i;
            free_byte = map[i];
            break;
        }
    }

    // no free data block
    if ( i == BITMAP_SIZE*BLOCK_SIZE ) {
        return -1;
    }

    // iterate a byte bit by bit
    int free_bit;
    for ( i = 0, free_bit = free_byte_pos * 8; i < 8; i++ ) {
        if ( (free_byte & 1 ) == 0 ) {
            free_bit += i;
            break;
        }
    }

    // check it out whether it is greater than 7931-1
    if ( free_bit >= 7931 ) {
        return -1;
    }

    return free_bit;
}
