#include "heap.h"
#include "memlib.h"
#include "list.h"
#include "libc/string.h"

struct tag {
    uint64_t size:63; // size of block, in words
    uint64_t inuse:1; // inuse bit
};

/* FENCE is used for heap prologue/epilogue. */
const struct tag FENCE = {
    .inuse = 1,
    .size = 0
};

/* 
 * A C struct describing the beginning of each inuse block.
 *
 * If each block is aligned at 12 mod 16, each payload will
 * be aligned at 0 mod 16.
 */
struct block {
    struct tag header; /* offset 0, at address 12 mod 16 */
    char payload[0];   /* offset 4, at address 0 mod 16 */
};

/* Basic constants and macros */
#define WSIZE                sizeof(struct tag) /* Word and header/footer size (bytes) */
#define MIN_BLOCK_SIZE_WORDS 8                  /* Minimum block size in words */
#define CHUNKSIZE            (36)               /* Extend heap by this amount (words) */
#define NUM_LISTS            12                 /* Number of segregated lists */

/*
 * Segregated free lists, separated by size in words that the 
 * block can be
 */
static struct list free_lists[NUM_LISTS];

static int get_seg_index(size_t words) {
    switch(words) {
        case 0    ... 8:    return 0; 
        case 9    ... 16:    return 1;
        case 17   ... 32:    return 2;
        case 33   ... 64:    return 3;
        case 65   ... 128:   return 4;
        case 129  ... 256:   return 5;
        case 257  ... 512:   return 6;
        case 513  ... 1024:  return 7;
        case 1025 ... 2048:  return 8;
        case 2049 ... 4096:  return 9;
        case 4097 ... 8192:  return 10;
        default:             return 11;
    }
}

static inline size_t max(size_t x, size_t y) {
    return x > y ? x : y;
}

static size_t align(size_t size) {
  return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

/* Global variables */
static struct block* heap_listp = 0;  /* Pointer to first block */

/* Function prototypes for internal helper routines */
static struct block* extend_heap(size_t words);
static void place(struct block* bp, size_t asize);
static struct block* find_fit(size_t asize);
static struct block* coalesce(struct block* bp);
static struct list* get_seg_list(size_t words);
static void checkheap(void);

/* 
 * Given a block, obtain previous's block footer.
 * Works for left-most block also. 
 */
static struct tag* prev_blk_footer(struct block* blk) {
    return &blk->header - 1;
}

/* Return if block is free */
static bool blk_free(struct block* blk) { 
    return !blk->header.inuse; 
}

/* Return size of payload of block */
static size_t blk_size(struct block* blk) { 
    return blk->header.size; 
}

/* 
 * Given a block, obtain pointer to previous block.
 * Not meaningful for left-most block. 
 */
static struct block* prev_blk(struct block* blk) {
    struct tag* prevfooter = prev_blk_footer(blk);
    return (struct block*)((void*)blk - WSIZE * prevfooter->size);
}

/* 
 * Given a block, obtain pointer to next block.
 * Not meaningful for right-most block. 
 */
static struct block* next_blk(struct block *blk) {
    return (struct block*)((void*)blk + WSIZE * blk->header.size);
}

/* Given a block, obtain its footer boundary tag */
static struct tag* get_footer(void* blk) {
    return (blk + WSIZE * ((struct block*)blk)->header.size)
                   - sizeof(struct tag);
}

/* Set a block's size and inuse bit in header and footer */
static void set_header_and_footer(struct block* blk, int size, int inuse) {
    blk->header.inuse = inuse;
    blk->header.size = size;
    *get_footer(blk) = blk->header; /* Copy header to footer */
}

/* Mark a block as used and sets its size. */
static void mark_block_used(struct block* blk, int size) {
    set_header_and_footer(blk, size, 1);
}

/* Mark a block as free, sets its size, and gives it an empty list_elem. */
static void mark_block_free(struct block* blk, int size) {
    set_header_and_footer(blk, size, 0);
    struct list_elem tmp = { .next = NULL, .prev = NULL };
    memmove(&blk->payload, &tmp, sizeof(tmp));
}

/* 
 * mm_init - Initialize the memory manager 
 */
int mm_init (void) {
    /* Create the initial empty heap */
    struct tag* initial = mem_sbrk(4 * sizeof(struct tag));
    if (initial == NULL)
        return -1;
    for (int i = 0; i < NUM_LISTS; i++) {
        list_init(&free_lists[i]);
    }

    /* We use a slightly different strategy than suggested in the book.
     * Rather than placing a min-sized prologue block at the beginning
     * of the heap, we simply place two fences.
     * The consequence is that coalesce() must call prev_blk_footer()
     * and not prev_blk() because prev_blk() cannot be called on the
     * left-most block.
     */
    initial[2] = FENCE;                     /* Prologue footer */
    heap_listp = (struct block*)&initial[3];
    initial[3] = FENCE;                     /* Epilogue header */

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE) == NULL) 
        return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block with at least size bytes of payload 
 */
void* mm_malloc (size_t size) {
    struct block *bp;      

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    size_t bsize = align(size + 2 * sizeof(struct tag));    /* account for tags */
    if (bsize < size)
        return NULL;    /* integer overflow */
    
    /* Adjusted block size in words */
    size_t awords = max(MIN_BLOCK_SIZE_WORDS, bsize/WSIZE); /* respect minimum size */

    /* Search the free list for a fit */
    if ((bp = find_fit(awords)) != NULL) {
        place(bp, awords);
        return bp->payload;
    }

    /* No fit found. Get more memory and place the block */
    size_t extendwords = max(awords,CHUNKSIZE); /* Amount to extend heap if no fit */
    if ((bp = extend_heap(extendwords)) == NULL) {
        return NULL;
    }

    place(bp, awords);
    return bp->payload;
}

/* 
 * mm_free - Free a block 
 */
void mm_free (void *bp) {
    if (bp == 0) 
        return;

    /* Find block from user pointer */
    struct block *blk = bp - offsetof(struct block, payload);

    mark_block_free(blk, blk_size(blk));
    list_push_back(get_seg_list(blk->header.size), (struct list_elem*)(&blk->payload));
    coalesce(blk);
}

/*
 * mm_realloc - Naive implementation of realloc where a block is resized to size
 */
void* mm_realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return mm_malloc(size);
    }
    if (size == 0) {
        mm_free(ptr);
        return (void*)0;
    }
    struct block* blk = (struct block*)(ptr - offsetof(struct block, payload));

    size_t oldsize = blk_size(blk);

    /* Adjust block size to include overhead and alignment reqs. */
    size_t bsize = align(size + 2 * sizeof(struct tag));    /* account for tags */
    if (bsize < size)
        return NULL;    /* integer overflow */
    
    /* Adjusted block size in words */
    size_t awords = max(MIN_BLOCK_SIZE_WORDS, bsize/WSIZE); /* respect minimum size */
    
    bool prev_alloc = prev_blk_footer(blk)->inuse; /* Prev block inuse? */
    bool next_alloc = !blk_free(next_blk(blk)); /* Next block inuse? */

    if (!memcmp((void*)(&next_blk(blk)->header), (void*)(&FENCE), sizeof(FENCE)) &&
        prev_alloc && awords > oldsize) {
        size_t extendwords = max(awords - oldsize, CHUNKSIZE); /* Amount to extend heap if no fit */
        extend_heap(extendwords);
    }

    if (awords == oldsize) {
        return ptr;
    }
    else if (awords < oldsize &&
            (oldsize - awords) >= MIN_BLOCK_SIZE_WORDS) { /* New block smaller than old */
        mark_block_used(blk, awords);
        blk = next_blk(blk);
        mark_block_free(blk, oldsize - awords);
        list_push_back(get_seg_list(blk->header.size), (struct list_elem*)(&blk->payload));
        coalesce(blk);
        return ptr;
    }
    /*
     * Block to be resized is in the following configuration:
     * [ Allocated ][ Current ][ Free ]
     */
    if (!next_alloc && prev_alloc) {
        if (awords < (blk_size(next_blk(blk)) + oldsize) &&
            (oldsize + blk_size(next_blk(blk)) - awords) >= MIN_BLOCK_SIZE_WORDS) {
            size_t tmp = blk_size(next_blk(blk));
            list_remove((struct list_elem*)(&next_blk(blk)->payload));
            mark_block_used(blk, awords);
            blk = next_blk(blk);
            mark_block_free(blk, oldsize + tmp - awords);
            list_push_back(get_seg_list(blk->header.size), (struct list_elem*)(&blk->payload));
            return ptr;
        }
    }
    /*
     * Block to be resized is in the following configuration:
     * [ Free ][ Current ][ Allocated ]
     * [ Free ][ Current ][ Free ]
     * And if the current resizes to larger than it and the last block
     * but smaller than it + last block + next block
     */
    if (!prev_alloc) {
        if (awords < (blk_size(prev_blk(blk)) + oldsize + blk_size(next_blk(blk))) &&
            (oldsize + blk_size(prev_blk(blk)) + blk_size(next_blk(blk)) - awords) >= MIN_BLOCK_SIZE_WORDS &&
            !next_alloc) {
            blk = prev_blk(blk);
            size_t tmp1 = blk_size(blk);
            size_t tmp2 = blk_size(next_blk(next_blk(blk)));
            list_remove((struct list_elem*)(&blk->payload));
            list_remove((struct list_elem*)(&(next_blk(next_blk(blk))->payload)));
            memmove(blk->payload, next_blk(blk)->payload, (oldsize - 2) * WSIZE);
            mark_block_used(blk, awords);
            ptr = blk->payload;
            blk = next_blk(blk);
            mark_block_free(blk, oldsize + tmp1 + tmp2 - awords);
            list_push_back(get_seg_list(blk->header.size), (struct list_elem*)(&blk->payload));
            return ptr;
        }
        if (awords < (blk_size(prev_blk(blk)) + oldsize) &&
            (oldsize + blk_size(prev_blk(blk)) - awords) >= MIN_BLOCK_SIZE_WORDS) {
            blk = prev_blk(blk);
            size_t tmp = blk_size(blk);
            list_remove((struct list_elem*)(&blk->payload));
            memmove(blk->payload, next_blk(blk)->payload, (oldsize - 2) * WSIZE);
            mark_block_used(blk, awords);
            ptr = blk->payload;
            blk = next_blk(blk);
            mark_block_free(blk, oldsize + tmp - awords);
            list_push_back(get_seg_list(blk->header.size), (struct list_elem*)(&blk->payload));
            coalesce(blk);
            return ptr;
        }
    }

    /* All other cases fall here (naive approach) */
    void *newptr = mm_malloc(size);

    /* If realloc() fails the original block is left untouched  */
    if (!newptr) {
        return 0;
    }

    /* Copy the old data. */
    struct block *oldblock = ptr - offsetof(struct block, payload);
    size_t oldpayloadsize = blk_size(oldblock) * WSIZE - 2 * sizeof(struct tag);
    if (size < oldpayloadsize) oldpayloadsize = size;
    memmove(newptr, ptr, oldpayloadsize);

    /* Free the old block. */
    mm_free(ptr);

    return newptr;
}

/* 
 * The remaining routines are internal helper routines 
 */

/* 
 * extend_heap - Extend heap with free block and return its block pointer
 */
static struct block* extend_heap(size_t words) 
{
    void* bp = mem_sbrk(words * WSIZE);

    if (bp == NULL)
        return NULL;

    /* Initialize free block header/footer and the epilogue header.
     * Note that we overwrite the previous epilogue here. */
    struct block* blk = bp - sizeof(FENCE);
    mark_block_free(blk, words);
    list_push_back(get_seg_list(blk->header.size), (struct list_elem*)(&blk->payload));
    next_blk(blk)->header = FENCE;

    /* Coalesce if the previous block was free */
    return coalesce(blk);
}

/* 
 * place - Place block of asize words at start of free block bp 
 *         and split if remainder would be at least minimum block size
 */
static void place(struct block *bp, size_t asize)
{
    size_t csize = blk_size(bp);

    /* Makes sure to remove the free block from list */
    list_remove((struct list_elem*)(&bp->payload));

    if ((csize - asize) >= MIN_BLOCK_SIZE_WORDS) { 
        /* Splits into [ Allocated ][ Free ] */
        mark_block_used(bp, asize);
        bp = next_blk(bp);
        mark_block_free(bp, csize-asize);
        list_push_back(get_seg_list(bp->header.size), (struct list_elem*)(&bp->payload));
    }
    else {
        /* Does not split */
        mark_block_used(bp, csize);
    }
}

/* 
 * find_fit - Find a fit for a block with asize words 
 */
static struct block* find_fit(size_t asize)
{
    /* Gets the smallest seg list that can fit block */
    int idx = get_seg_index(asize);
    /* Iterates through seg list entries until the first small enough one is found */
    for (size_t i = idx; i < NUM_LISTS; i++) {
        int j = 0;
        struct list* l = &free_lists[i];
        for (struct list_elem* elem = list_begin(l); 
             elem != list_end(l); 
             elem = list_next(elem)) {
            struct block* blk = (struct block*)((void*)elem - offsetof(struct block, payload));
            if (blk_size(blk) >= asize) {
                return blk;
            }
            /* Number of iterations goes down as size class increases */
            if (j > 11 - (i - idx)) {
                break;
            }
            j++;
        }
    }
    return NULL; /* No fit */
}

/*
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block
 */
static struct block* coalesce(struct block *bp) 
{
    bool prev_alloc = prev_blk_footer(bp)->inuse;   /* is previous block allocated? */
    bool next_alloc = !blk_free(next_blk(bp));     /* is next block allocated? */
    size_t size = blk_size(bp);

    if (prev_alloc && next_alloc) {            /* Case 1 */
        // both are allocated, nothing to coalesce
        return bp;
    }

    else if (prev_alloc && !next_alloc) {      /* Case 2 */
        // combine this block and next block by extending it
        list_remove((struct list_elem*)(&bp->payload));
        list_remove((struct list_elem*)(&next_blk(bp)->payload));
        mark_block_free(bp, size + blk_size(next_blk(bp)));
    }

    else if (!prev_alloc && next_alloc) {      /* Case 3 */
        // combine previous and this block by extending previous
        list_remove((struct list_elem*)(&bp->payload));
        list_remove((struct list_elem*)(&prev_blk(bp)->payload));
        bp = prev_blk(bp);
        mark_block_free(bp, size + blk_size(bp));
    }

    else {                                     /* Case 4 */
        // combine all previous, this, and next block into one
        list_remove((struct list_elem*)(&bp->payload));
        list_remove((struct list_elem*)(&prev_blk(bp)->payload));
        list_remove((struct list_elem*)(&next_blk(bp)->payload));
        mark_block_free(prev_blk(bp), 
                        size + blk_size(next_blk(bp)) + blk_size(prev_blk(bp)));
        bp = prev_blk(bp);
    }
    list_push_back(get_seg_list(bp->header.size), (struct list_elem*)(&bp->payload));
    return bp;
}

/*
 * get_seg_list - Gets the list that this block size belongs to
 */
static struct list* get_seg_list(size_t words) {
    /* Gets the number of words in payload */
    return &free_lists[get_seg_index(words)];
}
