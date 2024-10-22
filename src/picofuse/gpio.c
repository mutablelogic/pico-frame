#include <picofuse/picofuse.h>
#include <hardware/gpio.h>
#include "gpio.h"
#include "fuse.h"
#include "printf.h"

///////////////////////////////////////////////////////////////////////////////
// DECLARATIONS

/** @brief Set the function of a GPIO pin
 */
static void fuse_gpio_setfunc(uint8_t pin, fuse_gpio_func_t func);

/** @brief Callback for interrupt
 */
static void fuse_gpio_callback(uint pin, uint32_t events);

/** @brief Initialize a GPIO pin and set the interrupt
 */
static bool fuse_gpio_init(fuse_t *self, fuse_value_t *value, const void *user_data);

/** @brief Disable the interrupt on a GPIO pin
 */
static void fuse_gpio_destroy(fuse_t *self, fuse_value_t *value);

/** @brief Append a string representation of a GPIO pin
 */
static size_t fuse_gpio_str(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v, bool json);

///////////////////////////////////////////////////////////////////////////////
// GLOBALS

static fuse_gpio_t *fuse_gpio_pin[NUM_BANK0_GPIOS];
static fuse_t *fuse_gpio_instance = NULL;

///////////////////////////////////////////////////////////////////////////////
// LIFECYCLE

/** @brief Register value type for gpio
 */
void fuse_register_value_gpio(fuse_t *self)
{
    assert(self);

    // Register gpio type
    fuse_value_desc_t fuse_gpio_type = {
        .size = sizeof(struct gpio_context),
        .name = "GPIO",
        .init = fuse_gpio_init,
        .destroy = fuse_gpio_destroy,
        .str = fuse_gpio_str,
    };
    fuse_register_value_type(self, FUSE_MAGIC_GPIO, fuse_gpio_type);

    // Reset the GPIO pins
    for (size_t i = 0; i < NUM_BANK0_GPIOS; i++)
    {
        fuse_gpio_pin[i] = NULL;
    }

    // Set the callback instance
    fuse_gpio_instance = self;

    // Set the irq callback on this core
    gpio_set_irq_callback(&fuse_gpio_callback);
    irq_set_enabled(IO_IRQ_BANK0, true);
}

//////////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS

/** @brief Initialize a GPIO pin with a function
 *
 *  @param pin The GPIO pin number
 *  @param func The function of the GPIO pin
 */
fuse_gpio_t *fuse_new_gpio_ex(fuse_t *self, uint8_t pin, fuse_gpio_func_t func, const char *file, const int line)
{
    assert(self);
    assert(pin < fuse_gpio_count());
    assert(func < FUSE_GPIO_FUNC_COUNT);

    // Register type
    if (!fuse_is_registered_value(self, FUSE_MAGIC_GPIO))
    {
        fuse_register_value_gpio(self);
    }

    // Create a new GPIO context
    fuse_gpio_t *ctx = (fuse_gpio_t *)fuse_new_value_ex(self, FUSE_MAGIC_GPIO, (void *)(uintptr_t)pin, file, line);
    if (ctx == NULL)
    {
        return NULL;
    }

    // Set the GPIO function
    fuse_gpio_setfunc(pin, func);

    // Return the GPIO context
    return ctx;
}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS

/** @brief Return the GPIO cound (GPIO[0]...GPIO[count-1])
 */
inline uint8_t fuse_gpio_count()
{
    return NUM_BANK0_GPIOS;
}

/** @brief Set the GPIO pin to be low or high
 */
inline void fuse_gpio_set(fuse_gpio_t *pin, bool value)
{
    assert(pin);
    assert(pin->pin < fuse_gpio_count());
    gpio_put(pin->pin, value);
}

/** @brief Get the binary value of a GPIO pin
 */
inline bool fuse_gpio_get(fuse_gpio_t *pin)
{
    assert(pin);
    assert(pin->pin < fuse_gpio_count());
    return gpio_get(pin->pin);
}

///////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS

/** @brief Initialize a GPIO pin and set the interrupt
 */
static bool fuse_gpio_init(fuse_t *self, fuse_value_t *value, const void *user_data)
{
    assert(self);
    assert(value);
    assert(user_data);

    // If pin already initialized, return
    fuse_gpio_t *ctx = (fuse_gpio_t *)value;
    size_t pin = (size_t)user_data;
    if (fuse_gpio_pin[pin] != NULL)
    {
        return false;
    }
    else
    {
        fuse_gpio_pin[pin] = ctx;
        ctx->pin = pin;
    }

    // Return success
    return true;
}

/** @brief Disable the interrupt on a GPIO pin
 */
static void fuse_gpio_destroy(fuse_t *self, fuse_value_t *value)
{
    assert(self);
    assert(value);

    fuse_gpio_t *ctx = (fuse_gpio_t *)value;
    assert(ctx);
    assert(ctx->pin < fuse_gpio_count());

    // Cancel the interrupt
    gpio_set_irq_enabled(ctx->pin, 0xFF, false);
    gpio_deinit(ctx->pin);
}

#include <stdio.h>

/** @brief Set the function of a GPIO pin
 */
static void fuse_gpio_setfunc(uint8_t pin, fuse_gpio_func_t func)
{
    assert(pin < fuse_gpio_count());
    assert(func < FUSE_GPIO_FUNC_COUNT);

    // Set GPIO pin to SIO
    gpio_init(pin);

    // Cancel the interrupt
    gpio_set_irq_enabled(pin, 0xFF, false);

    // Set function based on function
    switch (func)
    {
    case FUSE_GPIO_IN:
        gpio_set_dir(pin, GPIO_IN);
        gpio_set_pulls(pin, false, false);
        gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
        break;
    case FUSE_GPIO_PULLDN:
        gpio_set_dir(pin, GPIO_IN);
        gpio_set_pulls(pin, false, true);
        gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
        break;
    case FUSE_GPIO_PULLUP:
        gpio_set_dir(pin, GPIO_IN);
        gpio_set_pulls(pin, true, false);
        gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
        break;
    case FUSE_GPIO_OUT:
        gpio_set_dir(pin, GPIO_OUT);
        break;
    case FUSE_GPIO_PWM:
        gpio_set_function(pin, GPIO_FUNC_PWM);
        break;
    case FUSE_GPIO_SPI:
        gpio_set_function(pin, GPIO_FUNC_SPI);
        break;
    case FUSE_GPIO_ADC:
        gpio_set_function(pin, FUSE_GPIO_ADC);
        break;
    case FUSE_GPIO_I2C:
        gpio_set_function(pin, FUSE_GPIO_I2C);
        break;
    case FUSE_GPIO_UART:
        gpio_set_function(pin, FUSE_GPIO_UART);
        break;
    default:
        assert(false);
    }
}

/** @brief Append a string representation of a GPIO pin
 */
static size_t fuse_gpio_str(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v, bool json)
{
    assert(self);
    assert(buf == NULL || sz > 0);
    assert(v);

    if(json) {
        // Add prefix
        i = chtostr_internal(buf, sz, i, '"');
    }

    i = cstrtostr_internal(buf, sz, i, self->desc[FUSE_MAGIC_GPIO].name);
    i = utostr_internal(buf, sz, i, ((fuse_gpio_t *)v)->pin, 0);

    if(json) {
        // Add suffix
        i = chtostr_internal(buf, sz, i, '"');
    }

    return i;
}

/** @brief GPIO interrupt callback - place event on the queues
 */
static void fuse_gpio_callback(uint pin, uint32_t events)
{
    fuse_t *self = fuse_gpio_instance;
    fuse_gpio_t *source = fuse_gpio_pin[pin];
    if (self && pin)
    {
        fuse_event_t *evt = fuse_new_event(self, (fuse_value_t *)source, FUSE_EVENT_GPIO, (void *)events);
        assert(evt);
    }
}
