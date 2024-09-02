#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////
// DEFINITIONS

// Public
#include <fuse/fuse.h>

// Private
#include "alloc.h"
#include "alloc_builtin.h"
#include "event.h"
#include "fuse.h"
#include "list.h"
#include "map.h"
#include "mutex.h"
#include "printf.h"
#include "timer.h"

///////////////////////////////////////////////////////////////////////////////
// DECLARATIONS

static bool fuse_init_null(fuse_t *self, fuse_value_t *value, const void *user_data);
static bool fuse_init_number(fuse_t *self, fuse_value_t *value, const void *user_data);
static bool fuse_init_memcpy(fuse_t *self, fuse_value_t *value, const void *user_data);
static bool fuse_init_cstr(fuse_t *self, fuse_value_t *value, const void *user_data);
static size_t fuse_cstr_null(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v);
static size_t fuse_qstr_null(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v);
static size_t fuse_qstr_bool(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v);
static size_t fuse_qstr_number(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v);
static size_t fuse_cstr_cstr(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v);
static size_t fuse_qstr_cstr(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v);
static size_t fuse_cstr_data(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v);
static size_t fuse_qstr_data(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v);

///////////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS

fuse_t *fuse_new()
{
    // Create an allocator
    struct fuse_allocator *allocator = fuse_allocator_builtin_new();
    if (allocator == NULL)
    {
        return NULL;
    }

    // Allocate a fuse application
    fuse_t *fuse = fuse_allocator_malloc(allocator, sizeof(fuse_t), (uint16_t)(FUSE_MAGIC_APP), NULL, 0);
    if (fuse == NULL)
    {
        fuse_allocator_destroy(allocator);
        return NULL;
    }
    else
    {
        fuse->allocator = allocator;
        fuse->exit_code = FUSE_EXIT_SUCCESS;
    }

    // Retain the application so it isn't autoreleased
    fuse_allocator_retain(allocator, fuse);

    // Register primitive types
    fuse->desc[FUSE_MAGIC_NULL] = (struct fuse_value_desc){
        .size = 0,
        .name = "NULL",
        .cstr = fuse_cstr_null,
        .qstr = fuse_qstr_null,
    };
    fuse->desc[FUSE_MAGIC_APP] = (struct fuse_value_desc){
        .size = 0,
        .name = "APP",
    };
    fuse->desc[FUSE_MAGIC_DATA] = (struct fuse_value_desc){
        .size = 0,
        .name = "DATA",
        .init = fuse_init_memcpy,
        .cstr = fuse_cstr_data,
        .qstr = fuse_qstr_data,
    };
    fuse->desc[FUSE_MAGIC_U8] = (struct fuse_value_desc){
        .size = sizeof(uint8_t),
        .name = "U8",
        .init = fuse_init_number,
        .cstr = fuse_qstr_number,
        .qstr = fuse_qstr_number,
    };
    fuse->desc[FUSE_MAGIC_U16] = (struct fuse_value_desc){
        .size = sizeof(uint16_t),
        .name = "U16",
        .init = fuse_init_number,
        .cstr = fuse_qstr_number,
        .qstr = fuse_qstr_number,
    };
    fuse->desc[FUSE_MAGIC_U32] = (struct fuse_value_desc){
        .size = sizeof(uint32_t),
        .name = "U32",
        .init = fuse_init_number,
        .cstr = fuse_qstr_number,
        .qstr = fuse_qstr_number,
    };
    fuse->desc[FUSE_MAGIC_U64] = (struct fuse_value_desc){
        .size = sizeof(uint64_t),
        .name = "U64",
        .init = fuse_init_number,
        .cstr = fuse_qstr_number,
        .qstr = fuse_qstr_number,
    };
    fuse->desc[FUSE_MAGIC_S8] = (struct fuse_value_desc){
        .size = sizeof(int8_t),
        .name = "S8",
        .init = fuse_init_number,
        .cstr = fuse_qstr_number,
        .qstr = fuse_qstr_number,
    };
    fuse->desc[FUSE_MAGIC_S16] = (struct fuse_value_desc){
        .size = sizeof(int16_t),
        .name = "S16",
        .init = fuse_init_number,
        .cstr = fuse_qstr_number,
        .qstr = fuse_qstr_number,
    };
    fuse->desc[FUSE_MAGIC_S32] = (struct fuse_value_desc){
        .size = sizeof(int32_t),
        .name = "S32",
        .init = fuse_init_number,
        .cstr = fuse_qstr_number,
        .qstr = fuse_qstr_number,
    };
    fuse->desc[FUSE_MAGIC_S64] = (struct fuse_value_desc){
        .size = sizeof(int64_t),
        .name = "S64",
        .init = fuse_init_number,
        .cstr = fuse_qstr_number,
        .qstr = fuse_qstr_number,
    };
    fuse->desc[FUSE_MAGIC_F32] = (struct fuse_value_desc){
        .size = sizeof(float),
        .name = "F32",
        .init = fuse_init_memcpy,
        .cstr = fuse_qstr_number,
        .qstr = fuse_qstr_number,
    };
    fuse->desc[FUSE_MAGIC_F64] = (struct fuse_value_desc){
        .size = sizeof(double),
        .name = "F64",
        .init = fuse_init_memcpy,
        .cstr = fuse_qstr_number,
        .qstr = fuse_qstr_number,
    };
    fuse->desc[FUSE_MAGIC_BOOL] = (struct fuse_value_desc){
        .size = sizeof(bool),
        .name = "BOOL",
        .init = fuse_init_number,
        .cstr = fuse_qstr_bool,
        .qstr = fuse_qstr_bool,
    };
    fuse->desc[FUSE_MAGIC_CSTR] = (struct fuse_value_desc){
        .size = sizeof(const char *),
        .name = "CSTR",
        .init = fuse_init_cstr,
        .cstr = fuse_cstr_cstr,
        .qstr = fuse_qstr_cstr,
    };
    fuse->desc[FUSE_MAGIC_LIST] = (struct fuse_value_desc){
        .size = sizeof(struct fuse_list),
        .name = "LIST",
        .init = fuse_init_list,
        .destroy = fuse_destroy_list,
        .cstr = fuse_qstr_list,
        .qstr = fuse_qstr_list,
    };
    fuse->desc[FUSE_MAGIC_MAP] = (struct fuse_value_desc){
        .size = sizeof(fuse_map_t),
        .name = "MAP",
    };

    // Register types
    fuse_register_value_event(fuse);
    fuse_register_value_mutex(fuse);
    fuse_register_value_timer(fuse);

    // Create the event queue for Core 0 
    fuse->core0 = (struct fuse_list *)fuse_retain(fuse, (fuse_value_t* )fuse_new_list(fuse));
    if (fuse->core0 == NULL)
    {
        fuse_allocator_free(allocator, fuse);
        fuse_allocator_destroy(allocator);
        return NULL;
    }

    // TODO: Don't create an event queue for Core 1
    fuse->core1 = NULL;

    // Return the fuse application
    return fuse;
}

int fuse_destroy(fuse_t *fuse)
{
    assert(fuse);

    // Release event queues
    fuse_release(fuse, (fuse_value_t* )fuse->core0);
    fuse_release(fuse, (fuse_value_t* )fuse->core1);

    // Store the exit code
    int exit_code = fuse->exit_code;
    struct fuse_allocator *allocator = fuse->allocator;

    // Repeatedly drain the allocator pool until no new memory blocks are freed
    // It will call fuse_destroy_callback for each memory block that is freed,
    // releasing resources
    size_t drained = 0;
    do
    {
        drained = fuse_drain(fuse, 0);
    } while (drained > 0);

    // Free the application
    fuse_allocator_free(allocator, fuse);

    // Walk through any remaining memory blocks
#ifdef DEBUG
    struct fuse_allocator_header *hdr = allocator->head;
    size_t count = 0;
    while (hdr != NULL)
    {
        // Print any memory leaks
        fuse_debugf("LEAK: %p %s (%d bytes)", hdr->ptr, fuse->desc[hdr->magic].name, hdr->size);
        if (hdr->file != NULL)
        {
            fuse_debugf(" [allocated at %s:%d]", hdr->file, hdr->line);
        }
        fuse_debugf("\n");
        count++;
        hdr = hdr->next;
    }

    // If the count is greater than zero, then there are memory leaks
    if (count > 0)
    {
        exit_code = FUSE_EXIT_MEMORYLEAKS;
    }
#endif

    // Free the allocator
    fuse_allocator_destroy(allocator);

    // Return the exit code
    return exit_code == FUSE_EXIT_SUCCESS ? 0 : exit_code;
}

/** @brief Drain the memory allocation pool
 */
size_t fuse_drain(fuse_t *self, size_t cap)
{
    assert(self);

    size_t count = 0;
    struct fuse_allocator_header *hdr = self->allocator->head;
    while (hdr != NULL && (cap == 0 || count < cap))
    {
        struct fuse_allocator_header *next = hdr->next;
        if (hdr->ref == 0)
        {
            fuse_free(self, hdr->ptr);
            count++;
        }
        hdr = next;
    }
    return count;
}

void *fuse_alloc_ex(fuse_t *self, const uint16_t magic, const void *user_data, const char *file, const int line)
{
    assert(self);
    assert(magic < FUSE_MAGIC_COUNT);

    // Determine the size of the memory block
    size_t size = self->desc[magic].size;
    switch (magic)
    {
    case FUSE_MAGIC_DATA:
        size = (size_t)user_data;
        break;
    default:
        size = self->desc[magic].size;
        break;
    }

    // Allocate the memory
    void *ptr = fuse_allocator_malloc(self->allocator, size, magic, file, line);
    if (ptr == NULL)
    {
#ifdef DEBUG
        fuse_debugf("fuse_alloc_ex: %s: Could not allocate %lu bytes", self->desc[magic].size);
        if (file != NULL)
        {
            fuse_debugf(" [allocated at %s:%d]", file, line);
        }
        fuse_debugf("\n");
#endif
        return NULL;
    }

    // Initialise the value
    if (self->desc[magic].init)
    {
        if (!self->desc[magic].init((struct fuse_application *)self, ptr, user_data))
        {
#ifdef DEBUG
            fuse_debugf("fuse_alloc_ex: %s: initialise failed", self->desc[magic].name);
            if (file != NULL)
            {
                fuse_debugf(" [allocated at %s:%d]", file, line);
            }
            fuse_debugf("\n");
#endif
            fuse_allocator_free(self->allocator, ptr);
            return NULL;
        }
    }

    // Return success
    return ptr;
}

void fuse_free(fuse_t *self, void *ptr)
{
    assert(self);
    assert(ptr);

    // Get the magic number
    uint16_t magic = fuse_allocator_magic(self->allocator, (void *)ptr);
    assert(magic < FUSE_MAGIC_COUNT);

    // Destroy the value
    if (self->desc[magic].destroy)
    {
        self->desc[magic].destroy((struct fuse_application *)self, (void *)ptr);
    }

    // Free the memory
    fuse_allocator_free(self->allocator, ptr);
}

void fuse_run(fuse_t *self, int (*callback)(fuse_t *))
{
    assert(self);
    assert(callback);

    // TODO: Register the timer device

    // Call the callback, and exit if it returns a non-zero value
    int exit_code = callback(self);
    if (exit_code)
    {
        fuse_exit(self, exit_code);
        return;
    }

    // Run the loop
    while (!self->exit_code)
    {
        sleep_ms(100);
    }
}

inline void fuse_exit(fuse_t *self, int exit_code)
{
    assert(self);
    self->exit_code = exit_code ? exit_code : FUSE_EXIT_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS

/* @brief Initialise a number value
 */
static bool fuse_init_number(fuse_t *self, fuse_value_t *value, const void *user_data)
{
    assert(self);
    assert(value);

    switch (fuse_allocator_magic(self->allocator, value))
    {
    case FUSE_MAGIC_U8:
        *(uint8_t *)value = (uint8_t)(uintptr_t)user_data;
        break;
    case FUSE_MAGIC_U16:
        *(uint16_t *)value = (uint16_t)(uintptr_t)user_data;
        break;
    case FUSE_MAGIC_U32:
        *(uint32_t *)value = (uint32_t)(uintptr_t)user_data;
        break;
    case FUSE_MAGIC_U64:
        *(uint64_t *)value = (uint64_t)(uintptr_t)user_data;
        break;
    case FUSE_MAGIC_S8:
        *(int8_t *)value = (int8_t)(intptr_t)user_data;
        break;
    case FUSE_MAGIC_S16:
        *(int16_t *)value = (int16_t)(intptr_t)user_data;
        break;
    case FUSE_MAGIC_S32:
        *(int32_t *)value = (int32_t)(intptr_t)user_data;
        break;
    case FUSE_MAGIC_S64:
        *(int64_t *)value = (int64_t)(intptr_t)user_data;
        break;
    case FUSE_MAGIC_BOOL:
        *(bool *)value = (bool)user_data;
        break;
    default:
        return false;
    }

    // Return success
    return true;
}

/* @brief Initialise by copying memory
 */
static bool fuse_init_memcpy(fuse_t *self, fuse_value_t *value, const void *user_data)
{
    assert(self);
    assert(value);

    uint16_t magic = fuse_allocator_magic(self->allocator, value);
    assert(magic < FUSE_MAGIC_COUNT);
    size_t size = self->desc[magic].size;

    // Copy or zero the memory
    if (user_data != NULL)
    {
        memcpy(value, user_data, size);
    }
    else
    {
        memset(value, 0, size);
    }

    // Return success
    return true;
}

/* @brief Initialise by setting the string pointer
 */
static bool fuse_init_cstr(fuse_t *self, fuse_value_t *value, const void *user_data)
{
    assert(self);
    assert(value);

    // Set the string pointer
    *(const char **)value = (const char *)user_data;

    // Return success
    return true;
}

/* @brief Output a null as a cstr
 */
static size_t fuse_cstr_null(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v)
{
    return cstrtostr_internal(buf, sz, i, NULL);
}

/* @brief Output a null as a qstr
 */
static size_t fuse_qstr_null(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v)
{
    return cstrtostr_internal(buf, sz, i, FUSE_PRINTF_NULL_JSON);
}

/* @brief Output a bool as a cstr
 */
static size_t fuse_qstr_bool(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v)
{
    assert(self);
    assert(v);
    assert(fuse_allocator_magic(self->allocator, v) == FUSE_MAGIC_BOOL);

    bool b = *(bool *)v;
    if (b)
    {
        return cstrtostr_internal(buf, sz, i, FUSE_PRINTF_TRUE);
    }
    else
    {
        return cstrtostr_internal(buf, sz, i, FUSE_PRINTF_FALSE);
    }
}

/* @brief Output an integer as a cstr
 */
static size_t fuse_qstr_number(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v)
{
    assert(self);
    assert(v);

    switch (fuse_allocator_magic(self->allocator, v))
    {
    case FUSE_MAGIC_U8:
        return utostr_internal(buf, sz, i, *(uint8_t *)v, 0);
    case FUSE_MAGIC_U16:
        return utostr_internal(buf, sz, i, *(uint16_t *)v, 0);
    case FUSE_MAGIC_U32:
        return utostr_internal(buf, sz, i, *(uint32_t *)v, 0);
    case FUSE_MAGIC_U64:
        return utostr_internal(buf, sz, i, *(uint64_t *)v, 0);
    case FUSE_MAGIC_S8:
        return itostr_internal(buf, sz, i, *(int8_t *)v, 0);
    case FUSE_MAGIC_S16:
        return itostr_internal(buf, sz, i, *(int16_t *)v, 0);
    case FUSE_MAGIC_S32:
        return itostr_internal(buf, sz, i, *(int32_t *)v, 0);
    case FUSE_MAGIC_S64:
        return itostr_internal(buf, sz, i, *(int64_t *)v, 0);
    case FUSE_MAGIC_F32:
        return ftostr_internal(buf, sz, i, *(float *)v, 0);
    case FUSE_MAGIC_F64:
        return ftostr_internal(buf, sz, i, *(double *)v, 0);
    default:
        assert(false);
    }
    return 0;
}

/* @brief Output a pointer to a null-terminated string as a cstr
 */
static size_t fuse_cstr_cstr(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v)
{
    assert(self);
    assert(v);
    assert(fuse_allocator_magic(self->allocator, v) == FUSE_MAGIC_CSTR);

    const char *vp = *(const char **)v;
    return cstrtostr_internal(buf, sz, i, vp);
}

/* @brief Output a pointer to a null-terminated string as a quoted cstr
 */
static size_t fuse_qstr_cstr(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v)
{
    assert(self);
    assert(v);
    assert(fuse_allocator_magic(self->allocator, v) == FUSE_MAGIC_CSTR);

    const char *vp = *(const char **)v;
    return qstrtostr_internal(buf, sz, i, vp);
}

/* @brief Output a data block as hex
 */
static size_t fuse_cstr_data(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v)
{
    assert(self);
    assert(v);
    assert(fuse_allocator_magic(self->allocator, v) == FUSE_MAGIC_DATA);

    // If data size is zero, then return empty string
    size_t datasz = fuse_allocator_size(self->allocator, v);
    if (datasz == 0)
    {
        return i;
    }

    // Write hex data
    size_t j = 0;
    while (j < datasz)
    {
        i = utostr_internal(buf, sz, i, ((uint8_t *)v)[j++], FUSE_PRINTF_FLAG_HEX | FUSE_PRINTF_FLAG_UPPER | 2);
    }

    // Return the new index
    return i;
}

/* @brief Output a data block as base64
 */
static size_t fuse_qstr_data(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v)
{
    assert(self);
    assert(v);
    assert(fuse_allocator_magic(self->allocator, v) == FUSE_MAGIC_DATA);

    return b64tostr_internal(buf, sz, i, v, fuse_allocator_size(self->allocator, v));
}
