#ifndef STM_LCD_AXS15231B_H
#define STM_LCD_AXS15231B_H
#include <stddef.h>
#include <stdint.h>
#include "stm_err.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct lcd_axs15231b_context *lcd_axs15231b_handle_t;
typedef struct {
    stm_err_t (*tx_param)(void *io, uint8_t cmd, const uint8_t *data, size_t bytes);
    stm_err_t (*tx_color)(void *io, uint8_t cmd, const void *pixels, size_t bytes);
    void (*delay_ms)(void *io, uint32_t ms);
    stm_err_t (*reset)(void *io, int high); /* optional */
    void *io;
    uint16_t width, height, x_gap, y_gap;
    uint8_t bgr; /* 0 RGB, 1 BGR; RGB565 only */
} lcd_axs15231b_config_t;
stm_err_t lcd_axs15231b_create(const lcd_axs15231b_config_t *config, lcd_axs15231b_handle_t *out);
stm_err_t lcd_axs15231b_delete(lcd_axs15231b_handle_t *handle);
stm_err_t lcd_axs15231b_reset(lcd_axs15231b_handle_t panel);
stm_err_t lcd_axs15231b_init(lcd_axs15231b_handle_t panel);
stm_err_t lcd_axs15231b_display_on_off(lcd_axs15231b_handle_t panel, int on);
/* [x1,x2) x [y1,y2); tightly packed RGB565; synchronous tx_color. */
stm_err_t lcd_axs15231b_draw_bitmap(lcd_axs15231b_handle_t panel, uint16_t x1, uint16_t y1,
                                     uint16_t x2, uint16_t y2, const void *pixels);
#ifdef __cplusplus
}
#endif
#endif
