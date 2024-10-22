/** @file adc.h
 *  @brief Private function prototypes and structure definitions for ADC.
 */
#ifndef FUSE_PRIVATE_ADC_H
#define FUSE_PRIVATE_ADC_H
#include <picofuse/picofuse.h>

/** @brief ADC context
 */
struct adc_context
{
    fuse_gpio_t *gpio[8]; ///< The GPIO pins for the ADC channels
    uint8_t temp;         ///< The temperature sensor channel, or zero if not enabled
};

///////////////////////////////////////////////////////////////////////////////
// DECLARATIONS

/** @brief Initialize a ADC instance
 */
static bool fuse_adc_init(fuse_t *self, fuse_value_t *value, const void *user_data);

/** @brief Destroy the ADC instance
 */
static void fuse_adc_destroy(fuse_t *self, fuse_value_t *value);

/** @brief Append a string representation of the ADC instance
 */
static size_t fuse_adc_str(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v, bool json);

/** @brief Returns true if a channel number is the temperature sensor
 */
static bool fuse_adc_is_temp_sensor(uint8_t ch);

/** @brief Returns the GPIO pin for an ADC channel, or 0 if the channel is invalid
 */
static uint8_t fuse_adc_gpio(uint8_t ch);

#endif
