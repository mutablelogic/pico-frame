/** @file watchdog.h
 *  @brief Function prototypes for watchdog interface
 *
 *  This file contains the function prototypes for accessing the watchdog timer hardware
 */
#ifndef PICOFUSE_WATCHDOG_H
#define PICOFUSE_WATCHDOG_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef DEBUG
#define fuse_new_watchdog(self, data) \
    ((fuse_pwm_t *)fuse_new_watchdog_ex((self), (data), __FILE__, __LINE__))
#else
#define fuse_new_watchdog(self, data) \
    ((fuse_pwm_t *)fuse_new_watchdog_ex((self), (data), 0, 0))
#endif

/** @brief An opaque watchdog object
 */
typedef struct watchdog_context fuse_watchdog_t;

/** @brief watchdog data used to initialize
 *
 *  This structure is used to initialize a watchdog timer.
 */
typedef struct
{
    uint16_t period_ms; ///< Requested period before watchdog updates. If zero, the default period is used.
} fuse_watchdog_config_t;


/** @brief Create a new watchdog instance
 *
 * @param self The fuse application
 * @param data The watchdog configuration
 * @return The watchdog context, or NULL if it could not be initialized
 */
fuse_watchdog_t *fuse_new_watchdog_ex(fuse_t *self, fuse_watchdog_config_t data, const char *file, const int line);

#endif
