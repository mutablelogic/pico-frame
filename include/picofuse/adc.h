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

/** @brief Set free-running mode for the ADC
 *
 *  This function sets the ADC to free-running mode, where it will sample the channels in round-robin
 *  at the specified frequency, and emit the event FUSE_EVENT_ADC when samples are ready.
 * 
 * @param self The fuse application
 * @param adc The ADC instance
 * @param freq The frequency for sampling, or zero to disable sampling
 */
void fuse_adc_enabled(fuse_t *self, fuse_adc_t *adc, uint32_t freq);

#endif
