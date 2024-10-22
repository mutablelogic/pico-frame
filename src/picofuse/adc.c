#include "adc.h"
#include "printf.h"
#include <hardware/adc.h>

///////////////////////////////////////////////////////////////////////////////
// GLOBALS

static fuse_adc_t *fuse_adc_instance = NULL;

///////////////////////////////////////////////////////////////////////////////
// LIFECYCLE

/** @brief Register type for ADC
 */
void fuse_register_value_adc(fuse_t *self)
{
    assert(self);

    fuse_value_desc_t fuse_adc_type = {
        .size = sizeof(struct adc_context),
        .name = "ADC",
        .init = fuse_adc_init,
        .destroy = fuse_adc_destroy,
        .str = fuse_adc_str,
    };
    fuse_register_value_type(self, FUSE_MAGIC_ADC, fuse_adc_type);
}

///////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS

/** @brief Initialize a adc instance
 */
static bool fuse_adc_init(fuse_t *self, fuse_value_t *value, const void *user_data)
{
    assert(self);
    assert(value);
    assert(user_data);

    // Check parameters
    fuse_adc_t *adc = (fuse_adc_t *)value;
    fuse_adc_config_t *data = (fuse_adc_config_t *)user_data;
    if (data->channel_mask == 0)
    {
        fuse_debugf(self, "fuse_adc_init: No ADC channels selected\n");
        return false;
    }

    // Zero
    for (uint8_t i = 0; i < 8; i++)
    {
        adc->gpio[i] = NULL;
    }
    adc->temp = 0;

    // Check for existing ADC - only one allowed
    if (fuse_adc_instance)
    {
        fuse_debugf(self, "fuse_adc_init: ADC already initialized\n");
        return false;
    }
    else
    {
        adc_init();
    }

    // Initialize the ADC channels
    for (uint8_t i = 0; i < NUM_ADC_CHANNELS; i++)
    {
        if (data->channel_mask & (1 << i) == 0)
        {
            continue;
        }

        // Initialize the temp sensor
        if (fuse_adc_is_temp_sensor(i))
        {
            adc_set_temp_sensor_enabled(true);
            adc->temp = i;
            fuse_debugf(self, "fuse_adc_init: init temp sensor\n");
        }
        else
        {
            // Get the GPIO pin for the channel
            uint pin = fuse_adc_gpio(i);
            if (pin == 0)
            {
                fuse_debugf(self, "fuse_adc_init: invalid channel %d\n", i);
                return false;
            }

            // Initialize the GPIO
            fuse_gpio_t *gpio = fuse_new_gpio_ex(self, pin, FUSE_GPIO_ADC, 0, 0);
            if (!gpio)
            {
                fuse_debugf(self, "fuse_adc_init: cannot initialize GPIO%d\n", pin);
                return false;
            }
        }
    }

    // Set the ADC instance
    fuse_adc_instance = adc;

    // Retain GPIO
    for (uint8_t i = 0; i < 8; i++)
    {
        fuse_retain(self, adc->gpio[i]);
    }

    // Return true
    return true;
}

/** @brief Destroy the ADC
 */
static void fuse_adc_destroy(fuse_t *self, fuse_value_t *value)
{
    assert(self);
    assert(value);
    fuse_adc_t *adc = (fuse_adc_t *)value;

    // Disable the temp sensor
    adc_set_temp_sensor_enabled(false);

    // Release GPIO
    for (uint8_t i = 0; i < 8; i++)
    {
        fuse_release(self, adc->gpio[i]);
    }

    // Remove instance
    fuse_adc_instance = NULL;
}

/** @brief Append a JSON representation of a watchdog
 */
static size_t fuse_adc_str(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *value, bool json)
{
    assert(self);
    assert(buf == NULL || sz > 0);
    assert(value);
    fuse_adc_t *adc = (fuse_adc_t *)value;

    // Add prefix
    i = chtostr_internal(buf, sz, i, '{');

    // Add suffix
    i = chtostr_internal(buf, sz, i, '}');

    return i;
}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS

/** @brief Initialize a ADC interface
 *
 * @param self The fuse application
 * @param data The ADC configuration
 * @return The ADC context, or NULL if it could not be initialized
 */
fuse_adc_t *fuse_new_adc_ex(fuse_t *self, fuse_adc_config_t data, const char *file, const int line)
{
    assert(self);

    // Register type
    if (!fuse_is_registered_value(self, FUSE_MAGIC_ADC))
    {
        fuse_register_value_adc(self);
    }

    return (fuse_adc_t *)fuse_new_value_ex(self, FUSE_MAGIC_ADC, &data, file, line);
}

///////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS

/** @brief Returns true if a channel number is the temperature sensor
 *
 */
static inline bool fuse_adc_is_temp_sensor(uint8_t ch)
{
    return ch == ADC_TEMPERATURE_CHANNEL_NUM;
}

/** @brief Returns the GPIO pin for an ADC channel, or 0 if the channel is invalid
 *
 */
static inline uint8_t fuse_adc_gpio(uint8_t ch)
{
    if (ch >= NUM_ADC_CHANNELS)
    {
        return 0;
    }
    if (fuse_adc_is_temp_sensor(ch))
    {
        return 0;
    }
    if (NUM_ADC_CHANNELS == 5)
    {
        return ch + 26;
    }
    if (NUM_ADC_CHANNELS == 8)
    {
        return ch + 40;
    }
    return 0;
}


/*

uint32_t fuse_adc_get(uint8_t channel, uint32_t *mask)
{
    assert(channel < fuse_adc_count());
    adc_select_input(channel);
    if (mask != NULL)
    {
        *mask = ADC_MASK;
    }
    return adc_read() & ADC_MASK;
}

float fuse_adc_get_voltage(uint8_t channel)
{
    assert(channel < fuse_adc_count());
    uint32_t value = fuse_adc_get(channel,NULL);
    return (float)value *  ADC_VREF / (float)ADC_MASK;
}

float fuse_adc_get_temp(uint8_t channel)
{
    float value = fuse_adc_get_voltage(channel);
    return 27. - (value - 0.706f) / 0.001721f;
}
*/