/**
 * @file    stm_lcd_axs15231b.h
 * @brief   提供 AXS15231B 通用面板构造接口。
 */
#ifndef STM_LCD_AXS15231B_H
#define STM_LCD_AXS15231B_H
#include "stm_lcd.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct
{
    stm_lcd_io_handle_t io;               // 借用的命令及像素 IO，须比面板存活更久。
    uint16_t width, height, x_gap, y_gap; // 逻辑尺寸与控制器偏移。
    uint8_t bgr;                          // 布尔 BGR 标志，固定 RGB565 格式。
    void (*delay_ms)(void *, uint32_t);   // 必需的启动阻塞延时。
    stm_err_t (*reset)(void *, int);      // 可选复位电平设置，低有效。
    void *control_context;                // 借用的复位与延时上下文。
} lcd_axs15231b_config_t;

/**
 * @brief 创建 AXS15231B 面板，只复制配置、借用 IO，不执行硬件初始化。
 *
 * @param config 有效配置及持续有效的 IO、控制上下文
 * @param out 输出通用面板句柄地址；初始必须为 NULL
 * @return STM_OK；配置、内存或所有权错误时失败；空输出保持 NULL，非空输出不覆盖。
 */
stm_err_t lcd_axs15231b_create(const lcd_axs15231b_config_t *config, stm_lcd_panel_handle_t *out);
#ifdef __cplusplus
}
#endif
#endif
