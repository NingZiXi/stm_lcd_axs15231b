/**
 * @file    test_panel.c
 * @brief   验证组件主机回归，不替代实板验收。
 */
#include "stm_lcd_axs15231b.h"
#include "stm_lcd_impl.h"
#include <assert.h>
#include <string.h>
static int call, fail_at, reset_count;
static uint8_t cmds[64];
static size_t lens[64];
static uint8_t payload[4];

static stm_err_t tx(void *io, uint8_t cmd, const uint8_t *data, size_t len)
{
    (void)io;
    assert(call < 64);
    cmds[call] = cmd;
    lens[call] = len;
    if (len == 4 && data)
        memcpy(payload, data, 4);
    ++call;
    return call == fail_at ? STM_ERR_IO : STM_OK;
}

static stm_err_t color(void *io, uint8_t cmd, const void *p, size_t n)
{
    assert(p && n);
    return tx(io, cmd, p, n);
}

static void delay(void *io, uint32_t ms)
{
    (void)io;
    (void)ms;
}

static stm_err_t reset(void *io, int high)
{
    (void)io;
    (void)high;
    ++reset_count;
    return STM_OK;
}

static struct stm_lcd_io io;

static void process(void *context)
{
    (void)context;
}

int main(void)
{
    stm_lcd_panel_handle_t h = NULL;
    stm_lcd_io_ops_t ops = {.tx_param = tx, .tx_color = color, .process = process};
    assert(stm_lcd_io_init(&io, &ops, NULL) == STM_OK);
    lcd_axs15231b_config_t c = {.io = &io,

                                .delay_ms = delay,
                                .reset = reset,
                                .width = 170,
                                .height = 560,
                                .x_gap = 2,
                                .y_gap = 3};
    assert(lcd_axs15231b_create(&c, &h) == STM_OK);
    assert(lcd_axs15231b_create(&c, &h) == STM_ERR_INVALID_STATE);
    assert(stm_lcd_panel_display_on_off(h, 1) == STM_ERR_INVALID_STATE);
    assert(stm_lcd_panel_init(h) == STM_OK && call == 35);
    call = 0;
    assert(stm_lcd_panel_set_window(h, 1, 2, 3, 4) == STM_OK);
    assert(call == 2 && cmds[0] == 0x2A && cmds[1] == 0x2B);
    assert(payload[0] == 0 && payload[1] == 5 && payload[2] == 0 && payload[3] == 6);
    call = 0;
    fail_at = 1;
    assert(stm_lcd_panel_set_window(h, 1, 2, 3, 4) == STM_ERR_IO && call == 1);
    fail_at = 0;
    assert(stm_lcd_panel_set_window(NULL, 0, 0, 1, 1) == STM_ERR_INVALID_ARG);
    assert(stm_lcd_panel_set_window(h, 0, 0, 171, 1) == STM_ERR_OUT_OF_RANGE);
    uint8_t px[8] = {0};
    call = 0;
    assert(stm_lcd_panel_draw_bitmap(h, 0, 0, 2, 2, px) == STM_OK);
    assert(call == 3 && cmds[0] == 0x2A && cmds[1] == 0x2B && cmds[2] == 0x2C && lens[2] == 8);
    call = 0;
    assert(stm_lcd_panel_draw_bitmap(h, 1, 1, 3, 3, px) == STM_OK && cmds[2] == 0x2C);
    assert(stm_lcd_panel_draw_bitmap(h, 0, 0, 171, 1, px) == STM_ERR_OUT_OF_RANGE);
    assert(stm_lcd_panel_draw_bitmap(h, 0, 0, 0, 1, px) == STM_ERR_INVALID_ARG);
    call = 0;
    fail_at = 2;
    assert(stm_lcd_panel_draw_bitmap(h, 0, 0, 1, 1, px) == STM_ERR_IO && call == 2);
    fail_at = 0;
    call = 0;
    assert(stm_lcd_panel_reset(h) == STM_OK && reset_count == 3);
    assert(stm_lcd_panel_draw_bitmap(h, 0, 0, 1, 1, px) == STM_ERR_INVALID_STATE);
    assert(stm_lcd_panel_set_window(h, 0, 0, 1, 1) == STM_ERR_INVALID_STATE);
    call = 0;
    fail_at = 4;
    assert(stm_lcd_panel_init(h) == STM_ERR_IO);
    assert(stm_lcd_panel_display_on_off(h, 1) == STM_ERR_INVALID_STATE);
    assert(stm_lcd_panel_delete(&h) == STM_OK && h == NULL);
    assert(stm_lcd_panel_delete(&h) == STM_OK);
    return 0;
}
