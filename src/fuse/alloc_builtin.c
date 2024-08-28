#include <stdbool.h>
#include <string.h>

// Includes
#include <fuse/fuse.h>
#include "alloc.h"

void free(void *ptr);
void *malloc(size_t size);

///////////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS

fuse_allocator_t *fuse_allocator_builtin_new()
{
    // Allocate memory for the allocator
    fuse_allocator_t *allocator = malloc(sizeof(fuse_allocator_t));
    if (allocator == NULL)
    {
        return NULL;
    }

    // Zero all data structures
    memset(allocator, 0, sizeof(fuse_allocator_t));

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
    if (block->next != NULL)
    {
        block->next->prev = block->prev;
    }
    if (ctx->head == block)
    {
        ctx->head = block->next;
    }
    if (ctx->tail == block)
    {
        ctx->tail = block->prev;
    }

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
    if (--block->ref == 0)
    {
        fuse_allocator_builtin_free(ctx, ptr);
        return true;
    }
    return false;
}

void** fuse_allocator_builtin_headptr(void *ptr) {
    assert(ptr);

    // Get the header
    struct fuse_allocator_header *block = ptr - sizeof(struct fuse_allocator_header);
    assert(block->ptr == ptr);

    // Return the head pointer
    return &block->head;
}

void** fuse_allocator_builtin_tailptr(void *ptr) {
    assert(ptr);

    // Get the header
    struct fuse_allocator_header *block = ptr - sizeof(struct fuse_allocator_header);
    assert(block->ptr == ptr);

    // Return the tail pointer
    return &block->tail;
}
