#include "uc8151.h"
#include "../../../fuse/printf.h"

///////////////////////////////////////////////////////////////////////////////
// DECLARATIONS

static void fuse_uc8151_register(fuse_t *self);
static bool fuse_uc8151_init(fuse_t *self, fuse_value_t *value, const void *user_data);
static void fuse_uc8151_destroy(fuse_t *self, fuse_value_t *value);
static size_t fuse_uc8151_str(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v, bool json);

///////////////////////////////////////////////////////////////////////////////
// LIFECYCLE

/** @brief Register value type for UC8151
 */
static void fuse_uc8151_register(fuse_t *self)
{
    assert(self);

    // Register UC8151
    fuse_value_desc_t fuse_uc8151_type = {
        .size = sizeof(struct uc8151_context),
        .name = "UC8151",
        .init = fuse_uc8151_init,
        .destroy = fuse_uc8151_destroy,
        .str = fuse_uc8151_str};
    fuse_register_value_type(self, FUSE_MAGIC_UC8151, fuse_uc8151_type);
}

/** @brief Create a new UC8151 driver with SPI interface
 */
fuse_uc8151_t *fuse_new_uc8151_ex(fuse_t *self, fuse_uc8151_config_t config, const char *file, const int line)
{
    assert(self);

    // Register type
    if (!fuse_is_registered_value(self, FUSE_MAGIC_UC8151))
    {
        fuse_uc8151_register(self);
    }

    // Return a new UC8151 context
    return (fuse_uc8151_t *)fuse_new_value_ex(self, FUSE_MAGIC_UC8151, &config, file, line);
}

///////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS

/** @brief Initialize a new UC8151 instance
 */
static bool fuse_uc8151_init(fuse_t *self, fuse_value_t *value, const void *user_data)
{
    assert(self);
    assert(value);
    assert(user_data);

    fuse_uc8151_config_t *config = (fuse_uc8151_config_t *)user_data;
    fuse_uc8151_t *ctx = (fuse_uc8151_t *)value;

    // Retain the SPI interface
    if (config->spi == NULL)
    {
        fuse_debugf(self, "fuse_uc8151_init: spi interface is required");
        return false;
    }
    else
    {
        ctx->spi = config->spi;
    }

    // Data/Command DC GPIO
    assert(config->dc < fuse_gpio_count());
    if (!config->dc)
    {
        fuse_debugf(self, "fuse_uc8151_init: data/command GPIO is required");
        return false;
    }
    else
    {
        ctx->dc = fuse_new_gpio_ex(self, config->dc, FUSE_GPIO_OUT, 0, 0);
        if (!ctx->dc)
        {
            fuse_debugf(self, "fuse_uc8151_init: cannot initialize DC (GPIO%d)\n", config->dc);
            return false;
        }
    }

    // Reset GPIO
    assert(config->reset < fuse_gpio_count());
    if (config->reset)
    {
        ctx->reset = fuse_new_gpio_ex(self, config->reset, FUSE_GPIO_OUT, 0, 0);
        if (!ctx->reset)
        {
            fuse_debugf(self, "fuse_uc8151_init: cannot initialize RESET (GPIO%d)\n", config->reset);
            return false;
        } else {
            // Set reset high
            fuse_gpio_set(ctx->reset, true);
        }
    }

    // Busy GPIO - set as pull-up
    assert(config->busy < fuse_gpio_count());
    if (config->busy)
    {
        ctx->busy = fuse_new_gpio_ex(self, config->busy, FUSE_GPIO_PULLUP, 0, 0);
        if (!ctx->busy)
        {
            fuse_debugf(self, "fuse_uc8151_init: cannot initialize BUSY (GPIO%d)\n", config->busy);
            return false;
        }
    }

    /*
    fuse_uc8151_update_t update; ///< The update speed. Can be zero for default.
    uint16_t width; ///< The width of the display in pixels. Required.
    uint16_t height; ///< The height of the display in pixels. Required.
    */

    // Reset
    if(ctx->reset && ctx->busy && !fuse_uc8151_reset(self,ctx))  {
        fuse_debugf(self, "fuse_uc8151_init: cannot reset UC8151\n");
        return false;
    }

    // Power on
    if(!fuse_uc8151_power(self, ctx, true)) {
        fuse_debugf(self, "fuse_uc8151_init: cannot power on UC8151\n");
        return false;
    }

    // Refresh the display
    if(!fuse_uc8151_refresh(self, ctx)) {
        fuse_debugf(self, "fuse_uc8151_init: cannot refresh UC8151\n");
        return false;
    }

    // Read the revision
    uint8_t rev = fuse_uc8151_read_revision(self, ctx);
    fuse_debugf(self, "fuse_uc8151_init: revision %d\n", rev);
    
    // Power off
    if(!fuse_uc8151_power(self, ctx, false)) {
        fuse_debugf(self, "fuse_uc8151_init: cannot power off UC8151\n");
        return false;
    }

    // Retain
    fuse_retain(self, ctx->spi);
    fuse_retain(self, ctx->dc);
    fuse_retain(self, ctx->reset);
    fuse_retain(self, ctx->busy);

    // Return success
    return true;
}

/** @brief Release the UC8151 instance
 */
static void fuse_uc8151_destroy(fuse_t *self, fuse_value_t *value)
{
    assert(self);
    assert(value);

    // Context
    fuse_uc8151_t *ctx = (fuse_uc8151_t *)value;

    // Release SPI and GPIO
    fuse_release(self, ctx->spi);
    fuse_release(self, ctx->dc);
    fuse_release(self, ctx->reset);
    fuse_release(self, ctx->busy);
}

/** @brief Append a JSON representation of the UC8151 instance
 */
static size_t fuse_uc8151_str(fuse_t *self, char *buf, size_t sz, size_t i, fuse_value_t *v, bool json)
{
    assert(self);
    assert(buf == NULL || sz > 0);
    assert(v);


    // Context
    fuse_uc8151_t *ctx = (fuse_uc8151_t *)v;

    // Add prefix
    i = chtostr_internal(buf, sz, i, '{');

    // Add spi
    i = qstrtostr_internal(buf, sz, i, "spi");
    i = chtostr_internal(buf, sz, i, ':');
    i = vtostr_internal(self, buf, sz, i, (fuse_value_t *)ctx->spi, true);
    i = chtostr_internal(buf, sz, i, ',');


    // Add DC
    if (ctx->dc) {
        i = qstrtostr_internal(buf, sz, i, "dc");
        i = chtostr_internal(buf, sz, i, ':');
        i = vtostr_internal(self, buf, sz, i, (fuse_value_t *)ctx->dc, true);
        i = chtostr_internal(buf, sz, i, ',');
    }

    // Add Reset
    if (ctx->reset) {
        i = qstrtostr_internal(buf, sz, i, "reset");
        i = chtostr_internal(buf, sz, i, ':');
        i = vtostr_internal(self, buf, sz, i, (fuse_value_t *)ctx->reset, true);
        i = chtostr_internal(buf, sz, i, ',');
    }

    // Add Busy
    if (ctx->busy) {
        i = qstrtostr_internal(buf, sz, i, "busy");
        i = chtostr_internal(buf, sz, i, ':');
        i = vtostr_internal(self, buf, sz, i, (fuse_value_t *)ctx->busy, true);
        i = chtostr_internal(buf, sz, i, ',');
    }

    // Add suffix
    i = chtostr_internal(buf, sz, i, '}');

    // Return the index
    return i;
}
