/**
 * @file    test_partial_windows.c
 * @brief   验证组件主机回归，不替代实板验收。
 */
#include "stm_lcd_axs15231b.h"
#include "stm_lcd_impl.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static uint16_t frame[560][170];
static uint16_t x1, y1, x2, y2, cursor_x, cursor_y;
static unsigned color_calls;

static uint16_t word(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static stm_err_t param(void *context, uint8_t cmd, const uint8_t *p, size_t size)
{
    (void)context;
    if ((cmd == 0x2A || cmd == 0x2B) && size == 4)
    {
        if (cmd == 0x2A)
        {
            x1 = word(p);
            x2 = word(p + 2);
        }
        else
        {
            y1 = word(p);
            y2 = word(p + 2);
        }
    }
    return STM_OK;
}

static stm_err_t color(void *context, uint8_t cmd, const void *pixels, size_t size)
{
    (void)context;
    const uint8_t *p = pixels;
    assert(p && size && !(size % 2) && (cmd == 0x2C || cmd == 0x3C));
    if (cmd == 0x2C)
    {
        cursor_x = x1;
        cursor_y = y1;
    }
    ++color_calls;
    for (size_t i = 0; i < size; i += 2)
    {
        assert(cursor_x < 170 && cursor_y < 560);
        frame[cursor_y][cursor_x] = word(p + i);
        if (++cursor_x > x2)
        {
            cursor_x = x1;
            if (++cursor_y > y2)
                cursor_y = y1;
        }
    }
    return STM_OK;
}

static void delay(void *context, uint32_t ms)
{
    (void)context;
    (void)ms;
}
static struct stm_lcd_io io;

static void process(void *context)
{
    (void)context;
}

int main(void)
{
    stm_lcd_panel_handle_t panel = NULL;
    stm_lcd_io_ops_t ops = {.tx_param = param, .tx_color = color, .process = process};
    assert(stm_lcd_io_init(&io, &ops, NULL) == STM_OK);
    lcd_axs15231b_config_t config = {
        .io = &io,
        .delay_ms = delay,
        .width = 170,
        .height = 560,
    };
    assert(lcd_axs15231b_create(&config, &panel) == STM_OK);
    assert(stm_lcd_panel_init(panel) == STM_OK);
    const uint8_t red[] = {0xF8, 0x00, 0xF8, 0x00};
    const uint8_t green[] = {0x07, 0xE0, 0x07, 0xE0};
    const uint8_t blue[] = {0x00, 0x1F, 0x00, 0x1F};
    assert(stm_lcd_panel_draw_bitmap(panel, 0, 0, 2, 1, red) == STM_OK);
    assert(frame[0][0] == 0xF800);
    // 下一脏矩形跳至底部百分比区域。
    assert(stm_lcd_panel_draw_bitmap(panel, 75, 544, 77, 545, green) == STM_OK);
    assert(frame[544][75] == 0x07E0 && frame[544][76] == 0x07E0);
    assert(frame[0][0] == 0xF800);
    // 再向前跳至无关加载动画区域。
    assert(stm_lcd_panel_draw_bitmap(panel, 20, 88, 22, 89, blue) == STM_OK);
    assert(frame[88][20] == 0x001F && frame[88][21] == 0x001F);
    assert(frame[544][75] == 0x07E0 && frame[0][0] == 0xF800);
    assert(color_calls == 3);
    assert(stm_lcd_panel_delete(&panel) == STM_OK);
    puts("Disjoint bottom-label / loader windows and untouched pixels: OK");
    return 0;
}
