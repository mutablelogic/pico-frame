#include <stdbool.h>
#include <string.h>
#include <fuse/fuse.h>
#include "alloc.h"

///////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS

void free(void *ptr);
void *malloc(size_t size);
void *fuse_allocator_builtin_malloc(struct fuse_allocator *ctx, size_t size, uint16_t magic, const char *file, int line);
void fuse_allocator_builtin_free(struct fuse_allocator *ctx, void *ptr);
void fuse_allocator_builtin_destroy(struct fuse_allocator *ctx);
uint16_t fuse_allocator_builtin_magic(struct fuse_allocator *ctx, void *ptr);
size_t fuse_allocator_builtin_size(struct fuse_allocator *ctx, void *ptr);
void fuse_allocator_builtin_retain(struct fuse_allocator *ctx, void *ptr);
bool fuse_allocator_builtin_release(struct fuse_allocator *ctx, void *ptr);
void **fuse_allocator_builtin_headptr(void *ptr);
void **fuse_allocator_builtin_tailptr(void *ptr);

///////////////////////////////////////////////////////////////////////////////
// LIFECYCLE

struct fuse_allocator *fuse_allocator_builtin_new()
{
    // Allocate memory for the allocator
    struct fuse_allocator *allocator = malloc(sizeof(struct fuse_allocator));
    if (allocator == NULL)
    {
        return NULL;
    }

    // Zero all data structures
    memset(allocator, 0, sizeof(struct fuse_allocator));

    // Set the allocator properties
    allocator->malloc = fuse_allocator_builtin_malloc;
    allocator->free = fuse_allocator_builtin_free;
    allocator->destroy = fuse_allocator_builtin_destroy;
    allocator->magic = fuse_allocator_builtin_magic;
    allocator->size = fuse_allocator_builtin_size;
    allocator->retain = fuse_allocator_builtin_retain;
    allocator->release = fuse_allocator_builtin_release;
    allocator->headptr = fuse_allocator_builtin_headptr;
    allocator->tailptr = fuse_allocator_builtin_tailptr;
    allocator->cur = sizeof(struct fuse_allocator);
    allocator->max = allocator->cur;

    // Return the allocator
    return allocator;
}

///////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS

void *fuse_allocator_builtin_malloc(struct fuse_allocator *ctx, size_t size, uint16_t magic, const char *file, int line)
{
    assert(ctx);

    // Create a header
    struct fuse_allocator_header *block = malloc(sizeof(struct fuse_allocator_header) + size);
    if (block == NULL)
    {
        return NULL;
    }

    // Set stats
    ctx->cur += size;
    if (ctx->cur > ctx->max)
    {
        ctx->max = ctx->cur;
    }

    // Zero all data structures
    memset(block, 0, sizeof(struct fuse_allocator_header));
    block->ptr = (void *)block + sizeof(struct fuse_allocator_header);
    block->size = size;
    block->magic = magic;
#ifdef DEBUG
    block->file = file;
    block->line = line;
#endif

    // Link into the list
    if (ctx->head == NULL)
    {
        ctx->head = block;
    }
    if (ctx->tail != NULL)
    {
        ctx->tail->next = block;
    }
    block->prev = ctx->tail;
    ctx->tail = block;

    // Return pointer to the memory block
    return block->ptr;
}

void fuse_allocator_builtin_free(struct fuse_allocator *ctx, void *ptr)
{
    assert(ctx);
    assert(ptr);

    // Get the header
    struct fuse_allocator_header *block = ptr - sizeof(struct fuse_allocator_header);
    assert(block->ptr == ptr);

    // Unlink from the list
    if (block->prev != NULL)
    {
        block->prev->next = block->next;
    }
    else
    { // Update head if this block was at the head
        ctx->head = block->next;
    }
    if (block->next != NULL)
    {
        block->next->prev = block->prev;
    } // Update tail if this block was at the tail
    else
    {
        ctx->tail = block->prev;
    }

    // Set stats
    ctx->cur -= block->size;

    // Free the memory block
    free(block);
}

void fuse_allocator_builtin_destroy(struct fuse_allocator *ctx)
{
    assert(ctx);
    struct fuse_allocator_header *block = ctx->head;
    while (block != NULL)
    {
        struct fuse_allocator_header *next = block->next;
        free(block);
        block = next;
    }

    // Free the allocator
    free(ctx);
}

uint16_t fuse_allocator_builtin_magic(struct fuse_allocator *ctx, void *ptr)
{
    assert(ctx);
    assert(ptr);

    // Get the header
    struct fuse_allocator_header *block = ptr - sizeof(struct fuse_allocator_header);
    assert(block->ptr == ptr);

    // Return the magic number
    return block->magic;
}

size_t fuse_allocator_builtin_size(struct fuse_allocator *ctx, void *ptr)
{
    assert(ctx);
    assert(ptr);

    // Get the header
    struct fuse_allocator_header *block = ptr - sizeof(struct fuse_allocator_header);
    assert(block->ptr == ptr);

    // Return the size
    return block->size;
}

void fuse_allocator_builtin_retain(struct fuse_allocator *ctx, void *ptr)
{
    assert(ctx);
    assert(ptr);

    // Get the header
    struct fuse_allocator_header *block = ptr - sizeof(struct fuse_allocator_header);
    assert(block->ptr == ptr);
    assert(block->ref < UINT16_MAX);

    // Increment the reference count
    // TODO: Do this atomically
    block->ref++;
}

bool fuse_allocator_builtin_release(struct fuse_allocator *ctx, void *ptr)
{
    assert(ctx);
    assert(ptr);

    // Get the header
    struct fuse_allocator_header *block = ptr - sizeof(struct fuse_allocator_header);
    assert(block->ptr == ptr);
    assert(block->ref > 0);

    // Decrement the reference count
    // TODO: Do this atomically
    return (--block->ref == 0);
}

void **fuse_allocator_builtin_headptr(void *ptr)
{
    assert(ptr);

    // Get the header
    struct fuse_allocator_header *block = ptr - sizeof(struct fuse_allocator_header);
    assert(block->ptr == ptr);

    // Return the head pointer
    return &block->head;
}

void **fuse_allocator_builtin_tailptr(void *ptr)
{
    assert(ptr);

    // Get the header
    struct fuse_allocator_header *block = ptr - sizeof(struct fuse_allocator_header);
    assert(block->ptr == ptr);

    // Return the tail pointer
    return &block->tail;
}
