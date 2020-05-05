# File-System
For this project you are required to implement a filesystem. The basic filesystem will be implemented in RAM and, if you get that far, you should work towards supporting a filesystem in persistent (secondary) storage (e.g., an IDE/SATA disk or USB flash drive).

## How to run?
```
cd Jiaqian-Ramdisk-Final

cd kernelspace

make

cd ../userspace

make
```


## First Step: A RAM Disk File system

To begin, you should start out with a **RAM-only** filesystem. You can choose one of two approaches: 
- (1) one approach is to implement your own filesystem using FIFOS/MEMOS-2 as the starting point, or 
- (2) you can build a filesystem in the Linux kernel. 

If you choose the **first approach**, you should create a new system called **DISCOS**, as a play on words for a RAM DISK OS. For DISCOS, you will **not be expected to implement system calls from user-space to kernel services**. Instead, you can implement all your file operations as function calls, with all your code working in the **kernel protection domain**. 

If you choose the **second approach**, you will need to implement user-level code in Linux to test your kernel-based RAM disk, and you will have to use the ioctl() approach from the first primer assignment to pass control from user-space to kernel-level.

We will provide a **user-space test routine**, which will work with a Linux filesystem, **but you will need to convert it to work with DISCOS**. You must be able to demonstrate your filesystem with the provided test routine. This routine will be posted at a later date.

## Approach 1 (DISCOS) * Worth more points *
In this approach, you will need to implement a RAM memory area in your DISCOS system that is initialized to 2MB using a constant defined in your code. Care must be taken to make sure the 2MB RAM disk "partition" is **in contiguous RAM**, so you will need to walk the memory map (from MEMOS-2) to find where free memory is available. You can assume your OS will run on **a virtual machine with more than 2MB of RAM available**.

## Approach 2 (Linux module)
Here, you will need to **implement a Linux kernel loadable module** for your RAM disk. However,  you will not have to modify the core kernel.

- Ramdisk initialization:

The first step is to leverage the ioctl() code from the primer to implement your own routine that creates a RAM disk. 

First, you need to create **an entry in the /proc** directory called /proc/ramdisk. You will be able to see the "ramdisk" in /proc when your module is loaded, but it should be removed from /proc when your module is unloaded. To actually create a memory area for your ramdisk, you can open /proc/ramdisk and issue a corresponding ioctl() command. That is, you need to create an ioctl file operation on /proc/ramdisk that invokes **vmalloc()** to establish a ramdisk memory area. Note that **kmalloc()** used to restrict memory allocations to a contiguous physical range up to 128KB (at least for Linux 2.4/2.6 kernels) while vmalloc() allows larger contiguous linear memory areas to be specified, although these areas may need paging support to map to non-contiguous page frames.

As with Approach 1, you can assume that your ramdisk memory area is initialized to 2MB using a constant defined in your code. That way, we can adjust this value if we recompile your code.


## Common to Both Approaches

### adding filesystem data structures:
Once your ramdisk memory area is established, you can treat it like **a raw partition**, comprised of a sequence of blocks. You can assume that a ramdisk **block is of size 256 bytes**, and the first block of the partition is a **superblock** containing filesystem-specific information that we will describe later. 


**The next 256 blocks** contain an array of **"index node"** structures that contain attributes for a given file. **Each index node is 64 bytes in size**. After the index node array the partition will contain four blocks for a "block bitmap" used to keep track of free and allocated blocks in the rest of the partition. Each bit of the block bitmap will be set to one or zero, depending upon whether or not the corresponding block is free or allocated.

The rest of the partition contains the contents of two types of files: **directories and regular files**. 

A regular file can hold arbitrary data, while a directory file holds directory entries for its position in a filesystem tree. Each **directory entry** is a (filename, index_node_number) pair, where the filename is a string, padded as necessary, to be 14 bytes including the null terminator, and the index_node_number is a two-byte integer used to index into the index node array of your partition to find information about a given file. Observe that implicitly the directory structure is a tree of directory entries where each entry may be a regular or directory file. The tree is rooted with a directory file whose name is "/" and whose index_node_number is 0. That is, the root of the directory tree has an index node stored in the first entry of the index node array.

The following attributes are to be maintained for each index node (where there is one index node per file):
- type -- either "dir" for directory or "reg" for regular files.
- size -- the current size of the corresponding file in bytes. For directories, this will be the size of all entries in the corresponding directory file, where each entry's size is the number of bytes needed to record a (filename, index_node_number) pair.
- location -- this attribute member identifies the blocks (potentially) scattered within the ramdisk that store the file's contents.
- access rights -- by default, a file can be read and written. However, a file's access rights can be modified to be: read-only, write-only, or read-write.

A file's storage space must occupy an integer number of ramdisk blocks. The location attribute of a file contains 10 block pointers of 4-bytes each: the first 8 pointers are direct block pointers, the 9th pointer is a single-indrect block pointer, and the 10th pointer is a double-indirect block pointer. This means that the maximum size of a single file will be: 256*8+64*256+64^2*256=1067008 bytes. Make sure you define constants for all these parameters!

To keep track of open files for each process using your filesystem, you need a table of open file descriptors on a per-process (Linux) or per-thread (FIFOS) basis. You can think of this table as a per-process/thread ramdisk file descriptor table. Each entry in this table will reference a file object that maintains three values: a set of status flags (described below, when a file is opened), the current read/write file position , and a pointer to the index node in the ramdisk partition for the corresponding file.

**NOTE: You can implement each ramdisk file descriptor table in memory outside the ramdisk partition.**

### The superblock:
The superblock at the beginning of the filesystem partition contains information about:

- the number of free blocks that can be allocated for storing directory and regular file contents;
- the number of free index nodes that can be associated with newly created files. Note that each file in the system will have its own index node;

The additional space in the superblock can be used to record any additional information that you wish to use for convenience e.g., the first data block in your partition.

### File operations:
You need to create the following file operations (preferably in a library if implemented in Linux) that can be used by normal processes wishing to use your ramdisk filesystem:

- int rd_creat(char *pathname, mode_t mode)

    create a regular file with absolute pathname and mode from the root of the directory tree, where each directory filename is delimited by a "/" character.  The mode can be read-write (default), read-only, or write-only . You can assume that any process opening an existing file is restricted by the access rights at creation time. On success, you should return 0, else if the file corresponding to pathname already exists you should return -1, indicating an error. Note that you need to update the parent directory file, to include the new entry. 

- int rd_mkdir(char *pathname) 

    this behaves like rd_creat() but pathname refers to a directory file. If the file already exists, return -1 else return 0. Note that you need to update the parent directory file, to include the new entry.

- int rd_open(char *pathname, int flags)

    open an existing file corresponding to pathname (which can be a regular or directory file) or report an error if file does not exist. When opening a file, you should return a file descriptor value that will index into the process'  ramdisk file descriptor table. As stated earlier, this table entry will contain a pointer to a file object. You can assume the file object has status=flags (unless there is an error), and the file position is set to 0. An error can occur when opening a file if the file does not exist or if the flags value overrides the access rights when the file was created. For example, a process should not be allowed to open a file for writing if its access rights are read-only. Finally, you can assume that flags can be any one of READONLY, WRITEONLY, or READWRITE. Return a value of -1 to indicate an access error, or if the file does not exist.

- int rd_close(int fd) 

    close the corresponding file descriptor and release the file object matching the value returned from a previous rd_open(). Return 0 on success and -1 on error. An error occurs if fd refers to a non-existent file.

- int rd_read(int fd, char *address, int num_bytes)

    read up to num_bytes from a regular file identified by file descriptor, fd, into a process' location at address. You should return the number of bytes actually read, else -1 if there is an error. An error occurs if the value of fd refers either to a non-existent file or a directory file. If developing DISCOS, you may only have threads within a single shared address space, in which case you should identify a buffer region into which your data is read.

- int rd_write(int fd, char *address, int num_bytes) 

    write up to num_bytes from the specified address in the calling process to a regular file identified by file descriptor, fd. You should return the actual number of bytes written, or -1 if there is an error. An error occurs if the value of fd refers either to a non-existent file or a directory file. If developing DISCOS, you may only have threads within a single shared address space, in which case you should identify a buffer region from which your data is written.

- int rd_lseek(int fd, int offset) 

    set the file object's file position identified by file descriptor, fd, to offset, returning the new position, or the end of the file position if the offset is beyond the file's current size. This call should return -1 to indicate an error, if applied to directory files, else 0 to indicate success.

- int rd_unlink(char *pathname) 

    remove the filename with absolute pathname from the filesystem, freeing its memory in the ramdisk. This function returns 0 if successful or -1 if there is an error.  An error can occur if: (1) the pathname does not exist, (2) you attempt to unlink a non-empty directory file, (3) you attempt to unlink an open file, or (4) you attempt to unlink the root directory file.

- int rd_chmod(char *pathname, mode_t mode) 
    
    change the mode (i.e., access rights) of a file identified by the absolute pathname. Return 0 if successful or a negative value for an error.

### Additional error checking:
Observe that all the above operations should return -1 to indicate additional error conditions not stated. In the cases of rd_creat(), rd_mkdir() and rd_write() you need to return  -1 if the file system has insufficient space and/or free index nodes to satisfy the requests. For rd_creat() and rd_mkdir() you should also report an error if you attempt to create a file whose pathname prefix refers to a non-existent directory. For example, if "home" was the name of a valid directory file with complete pathname "/home" but "/home/cs552" was non-existent, it would be an error to create "/home/cs552/project" but you could create "/home/cs552" first and then "/home/cs552/project".


## Testing
You should test your ramdisk filesystem using at least two processes (in Linux) or two threads (in DISCOS), where each process/thread creates and manipulates several ramdisk files. In your test routine(s) you need to non-trivially exercise each of the above file operations to show their correct functionality. Irrespective of Linux or DISCOS, you should use your file operations to create a file system tree of files, as well as special cases such as the largest single file or the maximum number of files your RAM disk will hold. Each file should then be written, as necessary, to store valid data that can be read correctly.

## Grading
A small bonus of up to 15% will be given to a DISCOS implementation built on FIFOS.

