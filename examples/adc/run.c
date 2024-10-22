#include <picofuse/picofuse.h>

int run(fuse_t *self)
{
    // ADC instance
    fuse_adc_config_t config = {
        .channel_mask = 0xFF,
    };
    fuse_adc_t *adc = (fuse_adc_t *)fuse_retain(self, fuse_new_adc(self, config));
    assert(adc);
    fuse_printf(self, "ADC: %v\n", adc);

    return 0;
}
