#include "heap.h"

#define WSIZE 4
#define DSIZE 8

#define GETW(p) (*(unsigned int *)(p))
#define PUTW(p, val) (GETW(p) = (val))

#define GET_SIZE(p) (GETW(p) & ~0x7)
#define GET_ALLOC(p) (GETW(p) & 0x1)

#define NEXT_BLKP(p) (((p) < LASTPTR) ? ((p) + GET_SIZE(p)) : NULL)
#define PREV_BLKP(p) (((p) > baseptr) ? ((p)-GETW((p) + WSIZE)) : NULL)

#define PACK(size, alloc) ((size) | (alloc))

#define LASTPTR (*(char **)((char *)baseptr - DSIZE))

#define DEFAULT_MEM_SIZE (1 << 20)

char buffer[DEFAULT_MEM_SIZE];
char *baseptr;

void init_heap(void)
{
    baseptr = buffer;
    baseptr += DSIZE;
    LASTPTR = baseptr;
    PUTW(baseptr, DEFAULT_MEM_SIZE - DSIZE); // 8 bytes for LASTPTR
    PUTW((char *)baseptr + WSIZE, 0);        // no previous block
}

void *malloc(size_t size)
{
    if (!size) // nothing to allocate
    {
        return NULL;
    }
    size = (size + 7) >> 3 << 3;             // round to 8
    const unsigned blocksize = size + DSIZE; // plus 8-byte header
    unsigned freesize;
    char *p = baseptr;
    // first fit
    while (p && (GET_ALLOC(p) || (freesize = GET_SIZE(p)) < blocksize))
    {
        p = NEXT_BLKP(p);
    }
    if (!p) // no enough space
    {
        return NULL;
    }
    unsigned newblocksize = freesize - blocksize;
    if (newblocksize <= DSIZE) // no enough space for a new free block
    {
        PUTW(p, PACK(freesize, 1));
    }
    else // split the free block
    {
        char *const next = NEXT_BLKP(p); // pointer to the address after the free block, NULL if p is the last block
        PUTW(p, PACK(blocksize, 1));
        char *const newfree = p + blocksize; // pointer to the new free block
        PUTW(newfree, newblocksize);
        PUTW(newfree + WSIZE, blocksize);
        if (next)
        {
            PUTW(next + WSIZE, newblocksize);
        }
        else
        {
            LASTPTR = newfree;
        }
    }
    return p + DSIZE;
}

void Combine(char *const p, char *const q) // combine a block p and its next q (must be free) to a free block
{
    const unsigned blocksize = GET_SIZE(p) + GETW(q);
    char *const next = NEXT_BLKP(q);
    PUTW(p, blocksize);
    if (next)
    {
        PUTW(next + WSIZE, blocksize);
    }
    else
    {
        LASTPTR = p;
    }
}

void free(void *addr)
{
    if (!addr) // nothing to free
    {
        return;
    }
    char *const p = (char *)addr - DSIZE;
    // first test next block
    char *next = NEXT_BLKP(p);
    if (next && !GET_ALLOC(next))
    {
        Combine(p, next);
    }
    else
    {
        unsigned blocksize = GET_SIZE(p);
        PUTW(p, blocksize);
    }
    // then test previous block
    char *prev = PREV_BLKP(p);
    if (prev && !GET_ALLOC(prev))
    {
        Combine(prev, p);
    }
}
