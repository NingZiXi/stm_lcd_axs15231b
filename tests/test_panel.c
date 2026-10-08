/**
 * @file test_panel.c
 *
 * @brief 组件主机回归测试，不替代实板验收。
 */
#include "stm_lcd_axs15231b.h"
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

int main(void)
{
    lcd_axs15231b_handle_t h = NULL;
    lcd_axs15231b_config_t c = {.tx_param = tx,
                                .tx_color = color,
                                .delay_ms = delay,
                                .reset = reset,
                                .width = 170,
                                .height = 560,
                                .x_gap = 2,
                                .y_gap = 3};
    assert(lcd_axs15231b_create(&c, &h) == STM_OK);
    assert(lcd_axs15231b_create(&c, &h) == STM_ERR_INVALID_STATE);
    assert(lcd_axs15231b_display_on_off(h, 1) == STM_ERR_INVALID_STATE);
    assert(lcd_axs15231b_init(h) == STM_OK && call == 35);
    uint8_t px[8] = {0};
    call = 0;
    assert(lcd_axs15231b_draw_bitmap(h, 0, 0, 2, 2, px) == STM_OK);
    assert(call == 3 && cmds[0] == 0x2A && cmds[1] == 0x2B && cmds[2] == 0x2C && lens[2] == 8);
    call = 0;
    assert(lcd_axs15231b_draw_bitmap(h, 1, 1, 3, 3, px) == STM_OK && cmds[2] == 0x3C);
    assert(lcd_axs15231b_draw_bitmap(h, 0, 0, 171, 1, px) == STM_ERR_OUT_OF_RANGE);
    assert(lcd_axs15231b_draw_bitmap(h, 0, 0, 0, 1, px) == STM_ERR_INVALID_ARG);
    call = 0;
    fail_at = 2;
    assert(lcd_axs15231b_draw_bitmap(h, 0, 0, 1, 1, px) == STM_ERR_IO && call == 2);
    fail_at = 0;
    call = 0;
    assert(lcd_axs15231b_reset(h) == STM_OK && reset_count == 3);
    assert(lcd_axs15231b_draw_bitmap(h, 0, 0, 1, 1, px) == STM_ERR_INVALID_STATE);
    call = 0;
    fail_at = 4;
    assert(lcd_axs15231b_init(h) == STM_ERR_IO);
    assert(lcd_axs15231b_display_on_off(h, 1) == STM_ERR_INVALID_STATE);
    assert(lcd_axs15231b_delete(&h) == STM_OK && h == NULL);
    assert(lcd_axs15231b_delete(&h) == STM_OK);
    return 0;
}
