#include "stm_lcd_axs15231b.h"
#include "stm_lcd_impl.h"
#include <stdlib.h>

/* AXS15231B init data adapted from Espressif (Apache-2.0). */
typedef struct
{
    uint8_t cmd, size;
    uint16_t delay_ms;
    uint8_t data[31];
} init_command_t;

// 模组专用参数表；
// 逻辑尺寸配置不能替代其他面板的时序参数。
static const init_command_t init_commands[] = {
    {0xBB, 8, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5A, 0xA5}},
    {0xA0,
     17,
     0,
     {0x00, 0x10, 0x00, 0x02, 0x00, 0x00, 0x64, 0x3F, 0x20, 0x05, 0x3F, 0x3F, 0x00, 0x00, 0x00,
      0x00, 0x00}},
    {0xA2, 31, 0, {0x30, 0x04, 0x0A, 0x3C, 0xEC, 0x54, 0xC4, 0x30, 0xAC, 0x28, 0x7F,
                   0x7F, 0x7F, 0x20, 0xF8, 0x10, 0x02, 0xFF, 0xFF, 0xF0, 0x90, 0x01,
                   0x32, 0xA0, 0x91, 0xC0, 0x20, 0x7F, 0xFF, 0x00, 0x54}},
    {0xD0, 30, 0, {0x30, 0xAC, 0x21, 0x24, 0x08, 0x09, 0x10, 0x01, 0xAA, 0x14,
                   0xC2, 0x00, 0x22, 0x22, 0xAA, 0x03, 0x10, 0x12, 0x40, 0x14,
                   0x1E, 0x51, 0x15, 0x00, 0x40, 0x10, 0x00, 0x03, 0x3D, 0x12}},
    {0xA3, 22, 0, {0xA0, 0x06, 0xAA, 0x08, 0x08, 0x02, 0x0A, 0x04, 0x04, 0x04, 0x04,
                   0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x55, 0x55}},
    {0xC1, 30, 0, {0x33, 0x04, 0x02, 0x02, 0x71, 0x05, 0x24, 0x55, 0x02, 0x00,
                   0x41, 0x00, 0x53, 0xFF, 0xFF, 0xFF, 0x4F, 0x52, 0x00, 0x4F,
                   0x52, 0x00, 0x45, 0x3B, 0x0B, 0x02, 0x0D, 0x00, 0xFF, 0x40}},
    {0xC3, 11, 0, {0x00, 0x00, 0x00, 0x50, 0x03, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01}},
    {0xC4, 29, 0, {0x00, 0x24, 0x33, 0x90, 0x50, 0xea, 0x64, 0x32, 0xC8, 0x64,
                   0xC8, 0x32, 0x90, 0x90, 0x11, 0x06, 0xDC, 0xFA, 0x04, 0x03,
                   0x80, 0xFE, 0x10, 0x10, 0x00, 0x0A, 0x0A, 0x44, 0x50}},
    {0xC5, 23, 0, {0x18, 0x00, 0x00, 0x03, 0xFE, 0x78, 0x33, 0x20, 0x30, 0x10, 0x88, 0xDE,
                   0x0D, 0x08, 0x0F, 0x0F, 0x01, 0x78, 0x33, 0x20, 0x10, 0x10, 0x80}},
    {0xC6, 20, 0, {0x05, 0x0A, 0x05, 0x0A, 0x00, 0xE0, 0x2E, 0x0B, 0x12, 0x22,
                   0x12, 0x22, 0x01, 0x00, 0x00, 0x3F, 0x6A, 0x18, 0xC8, 0x22}},
    {0xC7, 20, 0, {0x50, 0x32, 0x28, 0x00, 0xa2, 0x80, 0x8f, 0x00, 0x80, 0xff,
                   0x07, 0x11, 0x9F, 0x6f, 0xff, 0x26, 0x0c, 0x0d, 0x0e, 0x0f}},
    {0xC9, 4, 0, {0x33, 0x44, 0x44, 0x01}},
    {0xCF, 27, 0, {0x34, 0x1E, 0x88, 0x58, 0x13, 0x18, 0x56, 0x18, 0x1E,
                   0x68, 0xF7, 0x00, 0x65, 0x0C, 0x22, 0xC4, 0x0C, 0x77,
                   0x22, 0x44, 0xAA, 0x55, 0x04, 0x04, 0x12, 0xA0, 0x08}},
    {0xD5, 30, 0, {0x3E, 0x3E, 0x88, 0x00, 0x44, 0x04, 0x78, 0x33, 0x20, 0x78,
                   0x33, 0x20, 0x04, 0x28, 0xD3, 0x47, 0x03, 0x03, 0x03, 0x03,
                   0x86, 0x00, 0x00, 0x00, 0x30, 0x52, 0x3f, 0x40, 0x40, 0x96}},
    {0xD6, 30, 0, {0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE, 0x95, 0x00,
                   0x01, 0x83, 0x75, 0x36, 0x20, 0x75, 0x36, 0x20, 0x3F, 0x03,
                   0x03, 0x03, 0x10, 0x10, 0x00, 0x04, 0x51, 0x20, 0x01, 0x00}},
    {0xD7,
     19,
     0,
     {0x0a, 0x08, 0x0e, 0x0c, 0x1E, 0x18, 0x19, 0x1F, 0x00, 0x1F, 0x1A, 0x1F, 0x3E, 0x3E, 0x04,
      0x00, 0x1F, 0x1F, 0x1F}},
    {0xD8, 12, 0, {0x0B, 0x09, 0x0F, 0x0D, 0x1E, 0x18, 0x19, 0x1F, 0x01, 0x1F, 0x1A, 0x1F}},
    {0xD9, 13, 0, {0x00, 0x0D, 0x0F, 0x09, 0x0B, 0x1F, 0x18, 0x19, 0x1F, 0x01, 0x1E, 0x1A, 0x1F}},
    {0xDD, 12, 0, {0x0C, 0x0E, 0x08, 0x0A, 0x1F, 0x18, 0x19, 0x1F, 0x00, 0x1E, 0x1A, 0x1F}},
    {0xDF, 8, 0, {0x44, 0x73, 0x4B, 0x69, 0x00, 0x0A, 0x02, 0x90}},
    {0xE0,
     17,
     0,
     {0x19, 0x20, 0x0A, 0x13, 0x0E, 0x09, 0x12, 0x28, 0xD4, 0x24, 0x0C, 0x35, 0x13, 0x31, 0x36,
      0x2f, 0x03}},
    {0xE1,
     17,
     0,
     {0x38, 0x20, 0x09, 0x12, 0x0E, 0x08, 0x12, 0x28, 0xC5, 0x24, 0x0C, 0x34, 0x12, 0x31, 0x36,
      0x2f, 0x27}},
    {0xE2,
     17,
     0,
     {0x19, 0x20, 0x0A, 0x11, 0x09, 0x06, 0x11, 0x25, 0xD4, 0x22, 0x0B, 0x33, 0x12, 0x2D, 0x32,
      0x2f, 0x03}},
    {0xE3,
     17,
     0,
     {0x38, 0x20, 0x0A, 0x11, 0x09, 0x06, 0x11, 0x25, 0xC4, 0x21, 0x0A, 0x32, 0x11, 0x2C, 0x32,
      0x2f, 0x27}},
    {0xE4,
     17,
     0,
     {0x19, 0x20, 0x0D, 0x14, 0x0D, 0x08, 0x12, 0x2A, 0xD4, 0x26, 0x0E, 0x35, 0x13, 0x34, 0x39,
      0x2f, 0x03}},
    {0xE5,
     17,
     0,
     {0x38, 0x20, 0x0D, 0x13, 0x0D, 0x07, 0x12, 0x29, 0xC4, 0x25, 0x0D, 0x35, 0x12, 0x33, 0x39,
      0x2f, 0x27}},
    {0xBB, 8, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
    {0x13, 0, 0, {}},
    {0x11, 0, 200, {}},
    {0x29, 0, 200, {}},
    {0x2C, 4, 0, {0x00, 0x00, 0x00, 0x00}},
    {0x22, 0, 200, {}},
};

struct lcd_axs15231b_context
{
    struct stm_lcd_panel base;     /**< Embedded generic interface. */
    lcd_axs15231b_config_t config; /**< Controller-specific configuration. */
    uint8_t initialized;           /**< Initialization succeeded. */
};

static stm_err_t panel_reset(stm_lcd_panel_handle_t base)
{
    struct lcd_axs15231b_context *panel = (void *)base;
    if (!panel)
        return STM_ERR_INVALID_ARG;
    panel->initialized = 0;
    if (!panel->config.reset)
        return STM_ERR_NOT_SUPPORTED;
    stm_err_t err = panel->config.reset(panel->config.control_context, 1);
    if (err != STM_OK)
        return err;
    panel->config.delay_ms(panel->config.control_context, 10);
    err = panel->config.reset(panel->config.control_context, 0);
    if (err != STM_OK)
        return err;
    panel->config.delay_ms(panel->config.control_context, 10);
    err = panel->config.reset(panel->config.control_context, 1);
    if (err != STM_OK)
        return err;
    panel->config.delay_ms(panel->config.control_context, 120);
    return STM_OK;
}

static stm_err_t panel_init(stm_lcd_panel_handle_t base)
{
    struct lcd_axs15231b_context *panel = (void *)base;
    if (!panel)
        return STM_ERR_INVALID_ARG;
    // 初始化失败时禁止继续绘制，避免将半完成配置当作可用面板。
    panel->initialized = 0;
    stm_err_t err = stm_lcd_io_tx_param(panel->config.io, 0x11, NULL, 0);
    if (err != STM_OK)
        return err;
    panel->config.delay_ms(panel->config.control_context, 100);
    uint8_t madctl = panel->config.bgr ? 0x08u : 0x00u;
    err = stm_lcd_io_tx_param(panel->config.io, 0x36, &madctl, 1);
    if (err != STM_OK)
        return err;
    const uint8_t colmod = 0x55u;
    err = stm_lcd_io_tx_param(panel->config.io, 0x3A, &colmod, 1);
    if (err != STM_OK)
        return err;
    for (size_t i = 0; i < sizeof init_commands / sizeof init_commands[0]; ++i)
    {
        const init_command_t *op = &init_commands[i];
        err = stm_lcd_io_tx_param(panel->config.io, op->cmd, op->size ? op->data : NULL, op->size);
        if (err != STM_OK)
            return err;
        if (op->delay_ms)
            panel->config.delay_ms(panel->config.control_context, op->delay_ms);
    }
    panel->initialized = 1;
    return STM_OK;
}

static stm_err_t panel_display_on_off(stm_lcd_panel_handle_t base, int on)
{
    struct lcd_axs15231b_context *panel = (void *)base;
    if (!panel || (on != 0 && on != 1))
        return STM_ERR_INVALID_ARG;
    if (!panel->initialized)
        return STM_ERR_INVALID_STATE;
    return stm_lcd_io_tx_param(panel->config.io, on ? 0x29 : 0x28, NULL, 0);
}

static stm_err_t
panel_set_window(stm_lcd_panel_handle_t base, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    struct lcd_axs15231b_context *panel = (void *)base;
    if (!panel || x1 >= x2 || y1 >= y2)
        return STM_ERR_INVALID_ARG;
    if (x2 > panel->config.width || y2 > panel->config.height)
        return STM_ERR_OUT_OF_RANGE;
    if (!panel->initialized)
        return STM_ERR_INVALID_STATE;
    uint32_t xa = (uint32_t)x1 + panel->config.x_gap, xb = (uint32_t)x2 - 1u + panel->config.x_gap;
    uint32_t ya = (uint32_t)y1 + panel->config.y_gap, yb = (uint32_t)y2 - 1u + panel->config.y_gap;
    uint8_t x[4] = {(uint8_t)(xa >> 8), (uint8_t)xa, (uint8_t)(xb >> 8), (uint8_t)xb};
    uint8_t y[4] = {(uint8_t)(ya >> 8), (uint8_t)ya, (uint8_t)(yb >> 8), (uint8_t)yb};
    stm_err_t err = stm_lcd_io_tx_param(panel->config.io, 0x2A, x, sizeof x);
    if (err != STM_OK)
        return err;
    err = stm_lcd_io_tx_param(panel->config.io, 0x2B, y, sizeof y);
    if (err != STM_OK)
        return err;
    return STM_OK;
}

static stm_err_t panel_draw_bitmap(stm_lcd_panel_handle_t base,
                                   uint16_t x1,
                                   uint16_t y1,
                                   uint16_t x2,
                                   uint16_t y2,
                                   const void *pixels)
{
    struct lcd_axs15231b_context *panel = (void *)base;
    if (!panel || !pixels || x1 >= x2 || y1 >= y2)
        return STM_ERR_INVALID_ARG;
    if (x2 > panel->config.width || y2 > panel->config.height)
        return STM_ERR_OUT_OF_RANGE;
    size_t width = x2 - x1, height = y2 - y1;
    if (width > SIZE_MAX / height / 2u)
        return STM_ERR_OUT_OF_RANGE;
    stm_err_t err = panel_set_window(base, x1, y1, x2, y2);
    if (err != STM_OK)
        return err;
    // 每次调用都是独立窗口：RAMWR 从刚设置的 CASET/RASET 起点写入。
    // RAMWRC 会延续上一次写入游标，不能凭 y1 != 0 用于 LVGL 的不连续脏矩形。
    return stm_lcd_io_tx_color(panel->config.io, 0x2C, pixels, width * height * 2u);
}

static stm_err_t panel_draw_async(stm_lcd_panel_handle_t base,
                                  uint16_t x1,
                                  uint16_t y1,
                                  uint16_t x2,
                                  uint16_t y2,
                                  const void *pixels)
{
    struct lcd_axs15231b_context *panel = (void *)base;
    stm_err_t err = panel_set_window(base, x1, y1, x2, y2);
    if (err != STM_OK)
        return err;
    return stm_lcd_io_tx_color_async(panel->config.io, 0x2C, pixels,
                                     (size_t)(x2 - x1) * (y2 - y1) * 2u);
}

static void panel_destroy(stm_lcd_panel_handle_t panel)
{
    free(panel);
}

static const stm_lcd_panel_ops_t ops = {
    .reset = panel_reset,
    .init = panel_init,
    .display_on_off = panel_display_on_off,
    .set_window = panel_set_window,
    .draw = panel_draw_bitmap,
    .draw_async = panel_draw_async,
    .destroy = panel_destroy,
};

stm_err_t lcd_axs15231b_create(const lcd_axs15231b_config_t *config, stm_lcd_panel_handle_t *out)
{
    if (!config || !out)
        return STM_ERR_INVALID_ARG;
    if (*out)
        return STM_ERR_INVALID_STATE;
    if (!config->io || !config->io->ops || !config->io->ops->tx_param ||
        !config->io->ops->tx_color || !config->delay_ms || !config->width || !config->height ||
        config->bgr > 1 || (uint32_t)config->width + config->x_gap > 65536u ||
        (uint32_t)config->height + config->y_gap > 65536u)
        return STM_ERR_INVALID_CONFIG;
    struct lcd_axs15231b_context *panel = calloc(1, sizeof(*panel));
    if (!panel)
        return STM_ERR_NO_MEM;
    panel->config = *config;
    stm_err_t err =
        stm_lcd_panel_base_init(&panel->base, &ops, config->io, config->width, config->height);
    if (err != STM_OK)
    {
        free(panel);
        return err;
    }
    *out = &panel->base;
    return STM_OK;
}
