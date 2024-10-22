#include <picofuse/picofuse.h>

fuse_gpio_t* button_a = NULL;
fuse_gpio_t* button_b = NULL;
fuse_gpio_t* button_c = NULL;

const char* gpio_button(fuse_gpio_t* source)
{
    if (source == button_a)
    {
        return "A";
    }
    if (source == button_b)
    {
        return "B";
    }
    if (source == button_c)
    {
        return "C";
    }
    return NULL;
}

/* @brief Callback when there is a rising or falling edge on GPIO pin
 */
void gpio_callback(fuse_t *self, fuse_event_t *evt, void *user_data)
{
    assert(self);
    assert(evt);
    assert(user_data);

    const char* button = gpio_button((fuse_gpio_t* )fuse_event_source(self, evt));
    if(button == NULL)
    {
        return;
    }

    uint8_t action = (uintptr_t)user_data;
    if (action & FUSE_GPIO_RISING)
    {
        fuse_printf(self, "Button %s: Up\n", button);
    }
    if (action & FUSE_GPIO_FALLING)
    {
        fuse_printf(self, "Button %s: Down\n", button);
    }
}

int run(fuse_t *self)
{
    // GPIO callbacks for A, B, C buttons
    button_a = (fuse_gpio_t* )fuse_retain(self, fuse_new_gpio(self, 12, FUSE_GPIO_PULLUP));
    button_b = (fuse_gpio_t* )fuse_retain(self, fuse_new_gpio(self, 13, FUSE_GPIO_PULLUP));
    button_c = (fuse_gpio_t* )fuse_retain(self, fuse_new_gpio(self, 14, FUSE_GPIO_PULLUP));
    assert(button_a);
    assert(button_b);
    assert(button_c);

    // Register a callback
    fuse_register_callback(self, FUSE_EVENT_GPIO, 0, gpio_callback);

    // Create an SPI interface
    fuse_spi_data_t config = {
        .rx = 16,
        .cs = 17,
        .ck = 18,
        .tx = 19,
        .baudrate = 5 * 1000 * 1000,
        .xfr_delayms = 10,
    };

    // Create a new UC8151
    fuse_uc8151_config_t uc8151_config = {
        .spi = fuse_new_spi(self, config),
        .dc = 20,
        .reset = 21,
        .busy = 26,
        .width = 296,
        .height = 128
    };
    fuse_uc8151_t *uc8151 = fuse_new_uc8151(self, uc8151_config);
    assert(uc8151);

    fuse_debugf(self, "uc8151 initialised: %v\n", uc8151);

    return 0;
}
