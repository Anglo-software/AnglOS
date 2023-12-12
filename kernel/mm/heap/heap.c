#include "heap.h"
#include "libc/kernel/list.h"
#include "libc/string.h"
#include "memlib.h"
#include "threads/spinlock.h"

static spinlock lock;

struct tag {
    uint64_t size  : 63;
    uint64_t inuse : 1;
};

const struct tag FENCE = {.inuse = 1, .size = 0};

struct block {
    struct tag header;
    char payload[0];
};

#define WSIZE                sizeof(struct tag)
#define MIN_BLOCK_SIZE_WORDS 8
#define CHUNKSIZE            (36)
#define NUM_LISTS            12

static struct list free_lists[NUM_LISTS];

static int get_seg_index(size_t words) {
    switch (words) {
    case 0 ... 8: return 0;
    case 9 ... 16: return 1;
    case 17 ... 32: return 2;
    case 33 ... 64: return 3;
    case 65 ... 128: return 4;
    case 129 ... 256: return 5;
    case 257 ... 512: return 6;
    case 513 ... 1024: return 7;
    case 1025 ... 2048: return 8;
    case 2049 ... 4096: return 9;
    case 4097 ... 8192: return 10;
    default: return 11;
    }
}

static inline size_t max(size_t x, size_t y) { return x > y ? x : y; }

static size_t align(size_t size) {
    return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

static struct block* heap_listp = 0;

static struct block* extend_heap(size_t words);
static void place(struct block* bp, size_t asize);
static struct block* find_fit(size_t asize);
static struct block* coalesce(struct block* bp);
static struct list* get_seg_list(size_t words);

static struct tag* prev_blk_footer(struct block* blk) {
    return &blk->header - 1;
}

static bool blk_free(struct block* blk) { return !blk->header.inuse; }

static size_t blk_size(struct block* blk) { return blk->header.size; }

static struct block* prev_blk(struct block* blk) {
    struct tag* prevfooter = prev_blk_footer(blk);
    return (struct block*)((void*)blk - WSIZE * prevfooter->size);
}

static struct block* next_blk(struct block* blk) {
    return (struct block*)((void*)blk + WSIZE * blk->header.size);
}

static struct tag* get_footer(void* blk) {
    return (blk + WSIZE * ((struct block*)blk)->header.size) -
           sizeof(struct tag);
}

static void set_header_and_footer(struct block* blk, int size, int inuse) {
    blk->header.inuse = inuse;
    blk->header.size  = size;
    *get_footer(blk)  = blk->header;
}

static void mark_block_used(struct block* blk, int size) {
    set_header_and_footer(blk, size, 1);
}

static void mark_block_free(struct block* blk, int size) {
    set_header_and_footer(blk, size, 0);
    struct list_elem tmp = {.next = NULL, .prev = NULL};
    memmove(&blk->payload, &tmp, sizeof(tmp));
}

int initKernelHeap(void) {
    struct tag* initial = mem_sbrk(4 * sizeof(struct tag));
    if (initial == NULL)
        return -1;
    for (int i = 0; i < NUM_LISTS; i++) {
        list_init(&free_lists[i]);
    }

    spinInit(&lock);

    initial[2] = FENCE;
    heap_listp = (struct block*)&initial[3];
    initial[3] = FENCE;

    if (extend_heap(CHUNKSIZE) == NULL)
        return -1;
    return 0;
}

static void* _kmalloc(size_t size) {
    struct block* bp;

    if (size == 0)
        return NULL;

    size_t bsize = align(size + 2 * sizeof(struct tag));
    if (bsize < size)
        return NULL;

    size_t awords = max(MIN_BLOCK_SIZE_WORDS, bsize / WSIZE);

    if ((bp = find_fit(awords)) != NULL) {
        place(bp, awords);
        return bp->payload;
    }

    size_t extendwords = max(awords, CHUNKSIZE);
    if ((bp = extend_heap(extendwords)) == NULL) {
        return NULL;
    }

    place(bp, awords);
    return bp->payload;
}

static void* _kcalloc(size_t size) {
    void* ptr = _kmalloc(size);
    return memset(ptr, 0, size);
}

static void _kfree(void* bp) {
    if (bp == 0)
        return;

    struct block* blk = bp - offsetof(struct block, payload);

    mark_block_free(blk, blk_size(blk));
    list_push_back(get_seg_list(blk->header.size),
                   (struct list_elem*)(&blk->payload));
    coalesce(blk);
}

static void* _krealloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return _kmalloc(size);
    }
    if (size == 0) {
        _kfree(ptr);
        return (void*)0;
    }
    struct block* blk = (struct block*)(ptr - offsetof(struct block, payload));

    size_t oldsize    = blk_size(blk);

    size_t bsize      = align(size + 2 * sizeof(struct tag));
    if (bsize < size)
        return NULL;

    size_t awords   = max(MIN_BLOCK_SIZE_WORDS, bsize / WSIZE);

    bool prev_alloc = prev_blk_footer(blk)->inuse;
    bool next_alloc = !blk_free(next_blk(blk));

    if (!memcmp((void*)(&next_blk(blk)->header), (void*)(&FENCE),
                sizeof(FENCE)) &&
        prev_alloc && awords > oldsize) {
        size_t extendwords = max(awords - oldsize, CHUNKSIZE);
        extend_heap(extendwords);
    }

    if (awords == oldsize) {
        return ptr;
    }

    else if (awords < oldsize && (oldsize - awords) >= MIN_BLOCK_SIZE_WORDS) {
        mark_block_used(blk, awords);
        blk = next_blk(blk);
        mark_block_free(blk, oldsize - awords);
        list_push_back(get_seg_list(blk->header.size),
                       (struct list_elem*)(&blk->payload));
        coalesce(blk);
        return ptr;
    }

    if (!next_alloc && prev_alloc) {
        if (awords < (blk_size(next_blk(blk)) + oldsize) &&
            (oldsize + blk_size(next_blk(blk)) - awords) >=
                MIN_BLOCK_SIZE_WORDS) {
            size_t tmp = blk_size(next_blk(blk));
            list_remove((struct list_elem*)(&next_blk(blk)->payload));
            mark_block_used(blk, awords);
            blk = next_blk(blk);
            mark_block_free(blk, oldsize + tmp - awords);
            list_push_back(get_seg_list(blk->header.size),
                           (struct list_elem*)(&blk->payload));
            return ptr;
        }
    }

    if (!prev_alloc) {
        if (awords <
                (blk_size(prev_blk(blk)) + oldsize + blk_size(next_blk(blk))) &&
            (oldsize + blk_size(prev_blk(blk)) + blk_size(next_blk(blk)) -
             awords) >= MIN_BLOCK_SIZE_WORDS &&
            !next_alloc) {
            blk         = prev_blk(blk);
            size_t tmp1 = blk_size(blk);
            size_t tmp2 = blk_size(next_blk(next_blk(blk)));
            list_remove((struct list_elem*)(&blk->payload));
            list_remove(
                (struct list_elem*)(&(next_blk(next_blk(blk))->payload)));
            memmove(blk->payload, next_blk(blk)->payload,
                    (oldsize - 2) * WSIZE);
            mark_block_used(blk, awords);
            ptr = blk->payload;
            blk = next_blk(blk);
            mark_block_free(blk, oldsize + tmp1 + tmp2 - awords);
            list_push_back(get_seg_list(blk->header.size),
                           (struct list_elem*)(&blk->payload));
            return ptr;
        }
        if (awords < (blk_size(prev_blk(blk)) + oldsize) &&
            (oldsize + blk_size(prev_blk(blk)) - awords) >=
                MIN_BLOCK_SIZE_WORDS) {
            blk        = prev_blk(blk);
            size_t tmp = blk_size(blk);
            list_remove((struct list_elem*)(&blk->payload));
            memmove(blk->payload, next_blk(blk)->payload,
                    (oldsize - 2) * WSIZE);
            mark_block_used(blk, awords);
            ptr = blk->payload;
            blk = next_blk(blk);
            mark_block_free(blk, oldsize + tmp - awords);
            list_push_back(get_seg_list(blk->header.size),
                           (struct list_elem*)(&blk->payload));
            coalesce(blk);
            return ptr;
        }
    }

    void* newptr = _kmalloc(size);

    if (!newptr) {
        return 0;
    }

    struct block* oldblock = ptr - offsetof(struct block, payload);
    size_t oldpayloadsize = blk_size(oldblock) * WSIZE - 2 * sizeof(struct tag);
    if (size < oldpayloadsize)
        oldpayloadsize = size;
    memmove(newptr, ptr, oldpayloadsize);

    _kfree(ptr);

    return newptr;
}

static struct block* extend_heap(size_t words) {
    void* bp = mem_sbrk(words * WSIZE);

    if (bp == NULL)
        return NULL;

    struct block* blk = bp - sizeof(FENCE);
    mark_block_free(blk, words);
    list_push_back(get_seg_list(blk->header.size),
                   (struct list_elem*)(&blk->payload));
    next_blk(blk)->header = FENCE;

    return coalesce(blk);
}

static void place(struct block* bp, size_t asize) {
    size_t csize = blk_size(bp);

    list_remove((struct list_elem*)(&bp->payload));

    if ((csize - asize) >= MIN_BLOCK_SIZE_WORDS) {
        mark_block_used(bp, asize);
        bp = next_blk(bp);
        mark_block_free(bp, csize - asize);
        list_push_back(get_seg_list(bp->header.size),
                       (struct list_elem*)(&bp->payload));
    }
    else {
        mark_block_used(bp, csize);
    }
}

static struct block* find_fit(size_t asize) {
    size_t idx = get_seg_index(asize);
    for (size_t i = idx; i < NUM_LISTS; i++) {
        size_t j       = 0;
        struct list* l = &free_lists[i];
        for (struct list_elem* elem = list_begin(l); elem != list_end(l);
             elem                   = list_next(elem)) {
            struct block* blk =
                (struct block*)((void*)elem - offsetof(struct block, payload));
            if (blk_size(blk) >= asize) {
                return blk;
            }
            if (j > 11 - (i - idx)) {
                break;
            }
            j++;
        }
    }
    return NULL;
}

static struct block* coalesce(struct block* bp) {
    bool prev_alloc = prev_blk_footer(bp)->inuse;
    bool next_alloc = !blk_free(next_blk(bp));
    size_t size     = blk_size(bp);

    if (prev_alloc && next_alloc) {
        return bp;
    }

    else if (prev_alloc && !next_alloc) {
        list_remove((struct list_elem*)(&bp->payload));
        list_remove((struct list_elem*)(&next_blk(bp)->payload));
        mark_block_free(bp, size + blk_size(next_blk(bp)));
    }

    else if (!prev_alloc && next_alloc) {
        list_remove((struct list_elem*)(&bp->payload));
        list_remove((struct list_elem*)(&prev_blk(bp)->payload));
        bp = prev_blk(bp);
        mark_block_free(bp, size + blk_size(bp));
    }

    else {
        list_remove((struct list_elem*)(&bp->payload));
        list_remove((struct list_elem*)(&prev_blk(bp)->payload));
        list_remove((struct list_elem*)(&next_blk(bp)->payload));
        mark_block_free(prev_blk(bp),
                        size + blk_size(next_blk(bp)) + blk_size(prev_blk(bp)));
        bp = prev_blk(bp);
    }

    list_push_back(get_seg_list(bp->header.size),
                   (struct list_elem*)(&bp->payload));
    return bp;
}

static struct list* get_seg_list(size_t words) {
    return &free_lists[get_seg_index(words)];
}

void* kmalloc(size_t size) {
    spinLock(&lock);
    void* ptr = _kmalloc(size);
    spinUnlock(&lock);
    return ptr;
}

void* kcalloc(size_t size) {
    spinLock(&lock);
    void* ptr = _kcalloc(size);
    spinUnlock(&lock);
    return ptr;
}

void kfree(void* ptr) {
    spinLock(&lock);
    _kfree(ptr);
    spinUnlock(&lock);
}

void* krealloc(void* ptr, size_t size) {
    spinLock(&lock);
    void* rptr = _krealloc(ptr, size);
    spinUnlock(&lock);
    return rptr;
}