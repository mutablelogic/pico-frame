#include <picofuse/picofuse.h>

void adc_callback(fuse_t *self, fuse_event_t *evt, void *user_data)
{
    assert(self);
    assert(evt);

    uint16_t value = (uint16_t)user_data;
    fuse_printf(self, "%X", value);
}

int run(fuse_t *self)
{
    // ADC instance
    fuse_adc_config_t config = {
        .channel_mask = 0xFF,
    };
    fuse_adc_t *adc = (fuse_adc_t *)fuse_retain(self, fuse_new_adc(self, config));
    assert(adc);
    fuse_printf(self, "ADC: %v\n", adc);

    // Register callback
    fuse_register_callback(self, FUSE_EVENT_ADC, 0, adc_callback);

    // Sample at 1Hz
    fuse_adc_enabled(self, adc, 1);

    return 0;
}
