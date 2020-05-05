/* 
   -- template test file for RAMDISK Filesystem Assignment.
   -- include a case for:
   -- two processes (LINUX) or threads in DISCOS
   -- largest number of files (should be 1024 max -- 1023 discounting "/")
   -- largest single file (start with direct blocks [2048 bytes max], 
   then single-indirect [18432 bytes max] and finally double 
   indirect [1067008 bytes max])
   -- creating and unlinking files to avoid memory leaks
   -- each file operation
   -- error checking on invalid inputs
*/

#include "userspace.h"

// #define's to control what tests are performed,
// comment out a test if you do not wish to perform it

#define USE_RAMDISK
#define TEST1
#define TEST2
// #define TEST3
#define TEST4
#define TEST5
// #define TEST6

// File modes
#define RD  (S_IRUSR | S_IRGRP | S_IROTH)
#define RW  (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define WR  (S_IWUSR | S_IRGRP | S_IROTH)

// File open flags
#define READONLY   O_RDONLY
#define READWRITE  O_RDWR
#define WRITEONLY  O_WRONLY

// Insert a string for the pathname prefix here. For the ramdisk, it should be
// NULL
#define PATH_PREFIX "/"

#ifdef USE_RAMDISK
#define CREAT   rd_creat
#define OPEN    rd_open
#define WRITE   rd_write
#define READ    rd_read
#define UNLINK  rd_unlink
#define MKDIR   rd_mkdir
#define CLOSE   rd_close
#define LSEEK   rd_lseek
#define CHMOD   rd_chmod

#else
#define CREAT   creat
#define OPEN    open
#define WRITE   write
#define READ    read
#define UNLINK  unlink
#define MKDIR(path)   mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define CLOSE   close
#define LSEEK(fd, offset)   lseek(fd, offset, SEEK_SET)
#define CHMOD  chmod
#endif


// #define's to control whether single indirect or
// double indirect block pointers are tested

#define TEST_SINGLE_INDIRECT
#define TEST_DOUBLE_INDIRECT


#define MAX_FILES 4
#define BLK_SZ 256		/* Block size */
#define DIRECT 8		/* Direct pointers in location attribute */
#define PTR_SZ 4		/* 32-bit [relative] addressing */
#define PTRS_PB  (BLK_SZ / PTR_SZ) /* Pointers per index block */

static char pathname[80];

static char data1[DIRECT*BLK_SZ]; /* Largest data directly accessible */
static char data2[PTRS_PB*BLK_SZ];     /* Single indirect data size */
static char data3[PTRS_PB*PTRS_PB*BLK_SZ]; /* Double indirect data size */
static char addr[PTRS_PB*PTRS_PB*BLK_SZ+1]; /* Scratchpad memory */

int main () {
    
  int retval, i;
  int fd;
  int index_node_number;

  /* Some arbitrary data for our files */
  memset (data1, '1', sizeof (data1));
  memset (data2, '2', sizeof (data2));
  memset (data3, '3', sizeof (data3));

  rd_init();

#ifdef TEST1

  /* ****TEST 1: MAXIMUM file creation**** */

  /* Generate MAXIMUM regular files */
  for (i = 0; i < MAX_FILES; i++) { 
    sprintf (pathname, PATH_PREFIX "file%d", i);
    
    retval = CREAT (pathname, RD);
    
    if (retval < 0) {
      fprintf (stderr, "creat: File creation error! status: %d (%s)\n",
	       retval, pathname);
      perror("Error!");
      
      if (i != MAX_FILES)
	exit(EXIT_FAILURE);
    }
    
    memset (pathname, 0, 80);
  }   

  /* Delete all the files created */
  for (i = 0; i < MAX_FILES; i++) { 
    sprintf (pathname, PATH_PREFIX "file%d", i);
    
    retval = UNLINK (pathname);
    
    if (retval < 0) {
      fprintf (stderr, "unlink: File deletion error! status: %d\n",
	       retval);
      
      exit(EXIT_FAILURE);
    }
    
    memset (pathname, 0, 80);
  }

  if(retval == 0) {
    printf("Success!!!!");
  }

#endif // TEST1
  
#ifdef TEST2

  /* ****TEST 2: LARGEST file size**** */

  
  /* Generate one LARGEST file */
  retval = CREAT (PATH_PREFIX "bigfile", RW);

  if (retval < 0) {
    fprintf (stderr, "creat: File creation error! status: %d\n",
	     retval);

    exit(EXIT_FAILURE);
  }

  retval =  OPEN (PATH_PREFIX "bigfile", RW); /* Open file to write to it */
  
  if (retval < 0) {
    fprintf (stderr, "open: File open error! status: %d\n",
	     retval);

    exit(EXIT_FAILURE);
  }

  if(retval >= 0) {
    printf("Current cursor: %d\n", retval);
  }

  fd = retval;			/* Assign valid fd */

  /* Try writing to all direct data blocks */
  retval = WRITE (fd, data1, sizeof(data1));
  
  if (retval < 0) {
    fprintf (stderr, "write: File write STAGE1 error! status: %d\n",
	     retval);

    exit(EXIT_FAILURE);
  }

  printf("Write success!!!");

#ifdef TEST_SINGLE_INDIRECT
  
  /* Try writing to all single-indirect data blocks */
  retval = WRITE (fd, data2, sizeof(data2));
  
  if (retval < 0) {
    fprintf (stderr, "write: File write STAGE2 error! status: %d\n",
	     retval);

    exit(EXIT_FAILURE);
  }

#ifdef TEST_DOUBLE_INDIRECT

  /* Try writing to all double-indirect data blocks */
  retval = WRITE (fd, data3, sizeof(data3));
  
  if (retval < 0) {
    fprintf (stderr, "write: File write STAGE3 error! status: %d\n",
	     retval);

    exit(EXIT_FAILURE);
  }

#endif // TEST_DOUBLE_INDIRECT

#endif // TEST_SINGLE_INDIRECT

#endif // TEST2

#ifdef TEST3

  /* ****TEST 3: Seek and Read file test**** */
  retval = LSEEK (fd, 0);	/* Go back to the beginning of your file */

  if (retval < 0) {
    fprintf (stderr, "lseek: File seek error! status: %d\n",
	     retval);

    exit(EXIT_FAILURE);
  }

  /* Try reading from all direct data blocks */
  retval = READ (fd, addr, sizeof(data1));
  
  if (retval < 0) {
    fprintf (stderr, "read: File read STAGE1 error! status: %d\n",
	     retval);

    exit(EXIT_FAILURE);
  }
  /* Should be all 1s here... */
  printf ("Data at addr: %s\n", addr);
  printf ("Press a key to continue\n");
  getchar(); // Wait for keypress

#ifdef TEST_SINGLE_INDIRECT

  /* Try reading from all single-indirect data blocks */
  retval = READ (fd, addr, sizeof(data2));
  
  if (retval < 0) {
    fprintf (stderr, "read: File read STAGE2 error! status: %d\n",
	     retval);

    exit(EXIT_FAILURE);
  }
  /* Should be all 2s here... */
  printf ("Data at addr: %s\n", addr);
  printf ("Press a key to continue\n");
  getchar(); // Wait for keypress

#ifdef TEST_DOUBLE_INDIRECT

  /* Try reading from all double-indirect data blocks */
  retval = READ (fd, addr, sizeof(data3));
  
  if (retval < 0) {
    fprintf (stderr, "read: File read STAGE3 error! status: %d\n",
	     retval);

    exit(EXIT_FAILURE);
  }
  /* Should be all 3s here... */
  printf ("Data at addr: %s\n", addr);
  printf ("Press a key to continue\n");
  getchar(); // Wait for keypress

#endif // TEST_DOUBLE_INDIRECT

#endif // TEST_SINGLE_INDIRECT

  /* Close the bigfile */
  retval = CLOSE(fd);
  
  if (retval < 0) {
    fprintf (stderr, "close: File close error! status: %d\n",
	     retval);

    exit(EXIT_FAILURE);
  }

#endif // TEST3


#ifdef TEST4

  /* ****TEST 4: Check permissions**** */
  retval = CHMOD(PATH_PREFIX "bigfile", RD); // Change bigfile to read-only
  
  if (retval < 0) {
    fprintf (stderr, "chmod: Failed to change mode! status: %d\n",
	     retval);

    exit(EXIT_FAILURE);
  }

  /* Now try to write to bigfile again, but with read-only permissions! */
  retval = WRITE (fd, data1, sizeof(data1));
  if (retval < 0) {
    fprintf (stderr, "chmod: Tried to write to read-only file!\n");
    printf ("Press a key to continue\n");
    getchar(); // Wait for keypress
  }
  
  /* Remove the biggest file */

  retval = UNLINK (PATH_PREFIX "bigfile");
	
  if (retval < 0) {
    fprintf (stderr, "unlink: /bigfile file deletion error! status: %d\n",
	     retval);
    
    exit(EXIT_FAILURE);
  }

#endif // TEST4

  
#ifdef TEST5
  
  /* ****TEST 5: Make directory including entries **** */
  retval = MKDIR (PATH_PREFIX "dir1");
    
  if (retval < 0) {
    fprintf (stderr, "mkdir: Directory 1 creation error! status: %d\n",
	     retval);

    exit(EXIT_FAILURE);
  }

  retval = MKDIR (PATH_PREFIX "dir1/dir2");
    
  if (retval < 0) {
    fprintf (stderr, "mkdir: Directory 2 creation error! status: %d\n",
	     retval);

    exit(EXIT_FAILURE);
  }

  retval = MKDIR (PATH_PREFIX "dir1/dir3");
    
  if (retval < 0) {
    fprintf (stderr, "mkdir: Directory 3 creation error! status: %d\n",
	     retval);

    exit(EXIT_FAILURE);
  }

#endif // TEST5

  
#ifdef TEST6

  /* ****TEST 6: 2 process test**** */
  
  if((retval = fork())) {

    if(retval == -1) {
      fprintf(stderr, "Failed to fork\n");
      exit(EXIT_FAILURE);
    }

    /* Generate 300 regular files */
    for (i = 0; i < 300; i++) { 
      sprintf (pathname, PATH_PREFIX "/file_p_%d", i);
      
      retval = CREAT (pathname, RD);
      
      if (retval < 0) {
	fprintf (stderr, "(Parent) create: File creation error! status: %d\n", 
		 retval);
	exit(EXIT_FAILURE);
      }
    
      memset (pathname, 0, 80);
    }  
    
  }
  else {
    /* Generate 300 regular files */
    for (i = 0; i < 300; i++) { 
      sprintf (pathname, PATH_PREFIX "/file_c_%d", i);
      
      retval = CREAT (pathname, RD);
      
      if (retval < 0) {
	fprintf (stderr, "(Child) create: File creation error! status: %d\n", 
		 retval);

	exit(EXIT_FAILURE);
      }
      
      memset (pathname, 0, 80);
    }
  }

#endif // TEST6
  
  printf("Congratulations, you have passed all tests!!\n");
  
  return 0;
}
