/** @file adc.h
 *  @brief Function prototypes for ADC instance
 *
 *  This file contains the function prototypes for accessing the ADC hardware
 */
#ifndef PICOFUSE_ADC_H
#define PICOFUSE_ADC_H
#include <stdint.h>

#ifdef DEBUG
#define fuse_new_adc(self, data) \
    ((fuse_adc_t *)fuse_new_adc_ex((self), (data), __FILE__, __LINE__))
#else
#define fuse_new_adc(self, data) \
    ((fuse_adc_t *)fuse_new_adc_ex((self), (data), 0, 0))
#endif

/** @brief An opaque ADC object
 */
typedef struct adc_context fuse_adc_t;

/** @brief configuration used to initialize the ADC
 *
 *  This structure is used to initialize the ADC hardware
 */
typedef struct
{
    uint8_t channel_mask; ///< The ADC channels to sample (bitmask) in round-robin
} fuse_adc_config_t;

/** @brief Initialize a ADC interface
 *
 * @param self The fuse application
 * @param data The ADC configuration
 * @return The ADC context, or NULL if it could not be initialized
 */
fuse_adc_t *fuse_new_adc_ex(fuse_t *self, fuse_adc_config_t data, const char *file, const int line);

#endif
