/** @file test_async.c @brief Panel owns protocol and pending lifetime, without HAL/LVGL. */
#include "stm_lcd_axs15231b.h"
#include "stm_lcd_impl.h"
#include <assert.h>
#include <string.h>

static stm_lcd_panel_handle_t panel;
static struct stm_lcd_io g_io;
static unsigned calls, submits, fail_at;
static uint8_t commands[64], payload[4];
static size_t lengths[64];
static const void *borrowed;
static stm_err_t submit_result;
static int inline_complete;

static stm_err_t command(void *io, uint8_t cmd, const uint8_t *data, size_t bytes)
{
    assert(io == &calls && calls < 64);
    commands[calls] = cmd;
    lengths[calls++] = bytes;
    if (bytes == 4 && data)
        memcpy(payload, data, 4);
    return calls == fail_at ? STM_ERR_IO : STM_OK;
}

static stm_err_t sync_color(void *io, uint8_t cmd, const void *data, size_t bytes)
{
    assert(!borrowed);
    return command(io, cmd, data, bytes);
}

static stm_err_t async_color(void *io, uint8_t cmd, const void *data, size_t bytes)
{
    assert(io == &calls && cmd == 0x2C && data && bytes == 8 && calls == 2);
    assert(commands[0] == 0x2A && commands[1] == 0x2B);
    assert(payload[0] == 0 && payload[1] == 5 && payload[2] == 0 && payload[3] == 6);
    ++submits;
    if (submit_result != STM_OK)
        return submit_result; /* Failed submission retains no external ownership. */
    borrowed = data;
    assert(stm_lcd_panel_display_on_off(panel, 0) == STM_ERR_INVALID_STATE);
    if (inline_complete)
    {
        borrowed = NULL; /* Physical transport quiescent before completion. */
        assert(stm_lcd_io_complete(&g_io, STM_OK) == STM_OK);
    }
    return STM_OK;
}

static void delay(void *io, uint32_t ms)
{
    (void)io;
    (void)ms;
}

static void process(void *context)
{
    (void)context;
}

int main(void)
{
    uint8_t pixels[8] = {0};
    stm_lcd_io_ops_t ops = {.tx_param = command,
                            .tx_color_async = async_color,
                            .tx_color = sync_color,
                            .process = process};
    assert(stm_lcd_io_init(&g_io, &ops, &calls) == STM_OK);
    lcd_axs15231b_config_t config = {

        .delay_ms = delay,
        .io = &g_io,
        .control_context = &calls,
        .width = 170,
        .height = 560,
        .x_gap = 2,
        .y_gap = 3,

    };
    assert(stm_lcd_io_complete(NULL, STM_OK) == STM_ERR_INVALID_ARG);
    assert(lcd_axs15231b_create(&config, &panel) == STM_OK);
    assert(stm_lcd_panel_draw_bitmap_async(panel, 1, 2, 3, 4, pixels) == STM_ERR_INVALID_STATE);
    assert(stm_lcd_panel_init(panel) == STM_OK);
    calls = 0;
    assert(stm_lcd_panel_draw_bitmap_async(panel, 1, 2, 3, 4, pixels) == STM_OK);
    assert(submits == 1 && borrowed == pixels && calls == 2);
    assert(stm_lcd_panel_draw_bitmap_async(panel, 1, 2, 3, 4, pixels) == STM_ERR_INVALID_STATE);
    assert(stm_lcd_panel_draw_bitmap(panel, 1, 2, 3, 4, pixels) == STM_ERR_INVALID_STATE);
    assert(stm_lcd_panel_set_window(panel, 1, 2, 3, 4) == STM_ERR_INVALID_STATE);
    assert(stm_lcd_panel_reset(panel) == STM_ERR_INVALID_STATE);
    assert(stm_lcd_panel_init(panel) == STM_ERR_INVALID_STATE);
    assert(stm_lcd_panel_delete(&panel) == STM_ERR_INVALID_STATE && panel);
    assert(calls == 2 && borrowed == pixels);
    borrowed = NULL;
    assert(stm_lcd_io_complete(&g_io, STM_OK) == STM_OK);
    assert(stm_lcd_io_complete(&g_io, STM_OK) == STM_ERR_INVALID_STATE);
    calls = 0;
    assert(stm_lcd_panel_draw_bitmap(panel, 1, 2, 3, 4, pixels) == STM_OK);
    assert(calls == 3 && commands[2] == 0x2C && lengths[2] == 8 && submits == 1);

    calls = 0;
    fail_at = 2;
    assert(stm_lcd_panel_draw_bitmap_async(panel, 1, 2, 3, 4, pixels) == STM_ERR_IO);
    assert(calls == 2 && submits == 1 && !borrowed);
    fail_at = 0;
    calls = 0;
    submit_result = STM_ERR_TIMEOUT;
    assert(stm_lcd_panel_draw_bitmap_async(panel, 1, 2, 3, 4, pixels) == STM_ERR_TIMEOUT);
    assert(submits == 2 && !borrowed);
    assert(stm_lcd_io_complete(&g_io, STM_OK) == STM_ERR_INVALID_STATE);
    assert(stm_lcd_panel_display_on_off(panel, 1) == STM_OK);
    calls = 0;
    submit_result = STM_OK;
    inline_complete = 1;
    assert(stm_lcd_panel_draw_bitmap_async(panel, 1, 2, 3, 4, pixels) == STM_OK);
    assert(!borrowed && stm_lcd_io_complete(&g_io, STM_OK) == STM_ERR_INVALID_STATE);
    assert(stm_lcd_panel_draw_bitmap_async(panel, 0, 0, 171, 1, pixels) == STM_ERR_OUT_OF_RANGE);
    assert(stm_lcd_panel_draw_bitmap_async(panel, 0, 0, 0, 1, pixels) == STM_ERR_INVALID_ARG);
    assert(stm_lcd_panel_draw_bitmap_async(panel, 0, 0, 1, 1, NULL) == STM_ERR_INVALID_ARG);
    assert(stm_lcd_panel_delete(&panel) == STM_OK);
    assert(stm_lcd_io_deinit(&g_io) == STM_OK);
    ops.tx_color_async = NULL;
    assert(stm_lcd_io_init(&g_io, &ops, &calls) == STM_OK);
    assert(lcd_axs15231b_create(&config, &panel) == STM_OK);
    calls = 0;
    assert(stm_lcd_panel_init(panel) == STM_OK);
    calls = 0;
    assert(stm_lcd_panel_draw_bitmap_async(panel, 1, 2, 3, 4, pixels) == STM_ERR_NOT_SUPPORTED);
    assert(calls == 0);
    assert(stm_lcd_panel_draw_bitmap(panel, 1, 2, 3, 4, pixels) == STM_OK);
    assert(stm_lcd_panel_delete(&panel) == STM_OK);
    return 0;
}
