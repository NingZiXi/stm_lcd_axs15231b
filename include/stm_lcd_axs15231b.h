/** @file stm_lcd_axs15231b.h @brief AXS15231B constructor; operations use stm_lcd_panel_*. */
#ifndef STM_LCD_AXS15231B_H
#define STM_LCD_AXS15231B_H
#include "stm_lcd.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct
{
    stm_lcd_io_handle_t io;               /**< Borrowed command/pixel IO, must outlive panel. */
    uint16_t width, height, x_gap, y_gap; /**< Logical bounds and controller offsets. */
    uint8_t bgr;                          /**< Boolean, fixed RGB565 format. */
    void (*delay_ms)(void *, uint32_t);   /**< Required blocking startup delay. */
    stm_err_t (*reset)(void *, int);      /**< Optional active-low reset level. */
    void *control_context;                /**< Borrowed reset/delay context, independent of IO. */
} lcd_axs15231b_config_t;

/* No hardware access; *out must be NULL; failure leaves it NULL.
 * Configuration copied; single main-loop caller. Init table is module-specific. */
stm_err_t lcd_axs15231b_create(const lcd_axs15231b_config_t *config, stm_lcd_panel_handle_t *out);
#ifdef __cplusplus
}
#endif
#endif
