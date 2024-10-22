#include <picofuse/picofuse.h>

/* @brief Callback when there is a rising or falling edge on GPIO pin
 */
void gpio_callback(fuse_t *self, fuse_event_t *evt, void *user_data)
{
    assert(self);
    assert(evt);
    assert(user_data);

    // Print the event
    fuse_printf(self, "Event: evt=%v user_data=%p\n", evt, user_data);
}

int run(fuse_t *self)
{
    // GPIO callbacks for A, B, C buttons
    assert(fuse_retain(self, fuse_new_gpio(self, 12, FUSE_GPIO_PULLUP)));
    assert(fuse_retain(self, fuse_new_gpio(self, 13, FUSE_GPIO_PULLUP)));
    assert(fuse_retain(self, fuse_new_gpio(self, 14, FUSE_GPIO_PULLUP)));

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
