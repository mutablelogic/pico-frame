/** @file timer.h
 *  @brief Private function prototypes and structure definitions for timers.
 */
#ifndef FUSE_PRIVATE_TIMER_H
#define FUSE_PRIVATE_TIMER_H

#include <fuse/fuse.h>

#ifdef DEBUG
#define fuse_new_timer(self, data) \
    (fuse_new_value_ex((self), (FUSE_MAGIC_TIMER), (data), __FILE__, __LINE__))
#else
#define fuse_new_timer(self, data) \
    (fuse_new_value_ex((self), (FUSE_MAGIC_TIMER), (data), 0, 0))
#endif

///////////////////////////////////////////////////////////////////////////////
// DEFINITIONS

void fuse_register_value_timer(fuse_t *self);

#endif
