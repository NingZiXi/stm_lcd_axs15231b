/** @file example.c @brief Configure the board, then create a generic panel. */
#include "stm_lcd_axs15231b.h"
#include "stm_lcd_spi_f4.h"
static stm_lcd_spi_f4_t spi;
static stm_lcd_panel_handle_t panel;

static void delay_ms(void *context, uint32_t ms)
{
    (void)context;
    HAL_Delay(ms);
}

/* Application supplies clocks/GPIO/SPI and reset configuration. No PD15/backlight writes. */
extern stm_err_t board_init(stm_lcd_spi_f4_config_t *spi_config,
                            lcd_axs15231b_config_t *panel_config);

int main(void)
{
    stm_lcd_spi_f4_config_t io_config = {0};
    lcd_axs15231b_config_t config = {.width = 170, .height = 560, .delay_ms = delay_ms};
    HAL_Init();
    stm_err_t err = board_init(&io_config, &config);
    if (err == STM_OK)
        err = stm_lcd_spi_f4_init(&spi, &io_config);
    config.io = &spi.io;
    if (err == STM_OK)
        err = lcd_axs15231b_create(&config, &panel);
    if (err == STM_OK && config.reset)
        err = stm_lcd_panel_reset(panel);
    if (err == STM_OK)
        err = stm_lcd_panel_init(panel);
    if (err == STM_OK)
        err = stm_lcd_panel_display_on_off(panel, 1);
    if (err != STM_OK)
    {
        (void)stm_lcd_panel_delete(&panel);
        if (spi.io.ops)
            (void)stm_lcd_spi_f4_deinit(&spi);
    }
    for (;;)
    {
        stm_lcd_io_process(&spi.io);
        HAL_Delay(1);
    }
}
