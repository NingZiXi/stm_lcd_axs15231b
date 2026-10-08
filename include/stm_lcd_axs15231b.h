/**
 * @file stm_lcd_axs15231b.h
 *
 * @brief AXS15231B 面板接口，统一错误码与不透明句柄。
 */
#ifndef STM_LCD_AXS15231B_H
#define STM_LCD_AXS15231B_H
#include <stddef.h>
#include <stdint.h>
#include "stm_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct lcd_axs15231b_context *lcd_axs15231b_handle_t;

typedef struct
{
    stm_err_t (*tx_param)(void *io,
                          uint8_t cmd,
                          const uint8_t *data,
                          size_t bytes); /**< 同步命令传输； bytes 是字节数，0 时 data 可空。 */
    stm_err_t (*tx_color)(
        void *io,
        uint8_t cmd,
        const void *pixels,
        size_t bytes); /**< 命令与全部像素在一笔独占事务内同步完成，返回前消费完缓冲。 */
    void (*delay_ms)(void *io, uint32_t ms); /**< 必需，阻塞延时，单位毫秒。 */
    stm_err_t (*reset)(void *io,
                       int high); /**< 可选，设置低有效 RST 电平，high 为 0/1； 不管理 GPIO。 */
    void *io;                     /**< 借用的回调上下文，允许 NULL，在实例存续期保持有效。 */
    uint16_t width, height, x_gap, y_gap; /**< 逻辑宽高必须非零； 尺寸加偏移不能超过 65536。 */
    uint8_t bgr; /**< 仅允许 0/1：0 为 RGB，1 为 BGR； 像素格式固定 RGB565。 */
} lcd_axs15231b_config_t;

/**
 * @brief 创建控制对象，不初始化总线，不访问硬件。
 *
 * @param config 配置被复制；
 *     回调及 io 指向的资源由调用者持有。
 * @param out 输出句柄地址，调用前 *out 必须为 NULL；
 *     失败保持原值。
 *
 * @return STM_OK、STM_ERR_INVALID_ARG、STM_ERR_INVALID_STATE、
 *     STM_ERR_INVALID_CONFIG 或 STM_ERR_NO_MEM。
 *
 * @note 使用 calloc/free；
 *     无 HAL、日志或 RTOS 依赖。
 *     实例访问由应用串行化，不从 ISR 调用。
 */
stm_err_t lcd_axs15231b_create(const lcd_axs15231b_config_t *config, lcd_axs15231b_handle_t *out);

/**
 * @brief 释放控制对象并清空句柄，不关闭总线或释放板级资源。
 *
 * @param handle 句柄地址；
 *     *handle 为 NULL 时也成功，handle 本身不能为 NULL。
 *
 * @return STM_OK 或 STM_ERR_INVALID_ARG。
 *
 * @note 删除前停止并发访问；
 *     其他句柄别名不会清空，删除后禁止使用。
 */
stm_err_t lcd_axs15231b_delete(lcd_axs15231b_handle_t *handle);

/**
 * @brief 清除初始化状态，并执行可选硬件复位。
 *
 * @param panel 有效面板句柄。
 *
 * @return STM_OK、STM_ERR_INVALID_ARG 或 reset 回调原样返回的错误。
 *
 * @note 有 reset 时执行高 10ms、低 10ms、高 120ms；
 *     无 reset 时不访问硬件。
 *     成功或失败后均须重新 init；
 *     失败可重试，不自动重试。
 */
stm_err_t lcd_axs15231b_reset(lcd_axs15231b_handle_t panel);

/**
 * @brief 设置 RGB565/色序并执行当前 170x560 模组初始化表，不隐式复位。
 *
 * @param panel 有效面板句柄；
 *     通常按 reset -> init 调用。
 *
 * @return STM_OK、STM_ERR_INVALID_ARG 或传输回调原样返回的错误。
 *
 * @note 失败保持未初始化，可重新 init；
 *     不回滚已发送的命令。
 *     width/height 是逻辑窗口边界，不会自动适配其他模组的初始化参数。
 */
stm_err_t lcd_axs15231b_init(lcd_axs15231b_handle_t panel);

/**
 * @brief 发送显示开关命令，不控制模组电源或背光。
 *
 * @param panel 已初始化的面板句柄。
 * @param on 仅允许 0/1：0 关闭显示，1 开启显示。
 *
 * @return STM_OK、STM_ERR_INVALID_ARG、STM_ERR_INVALID_STATE 或传输回调错误。
 */
stm_err_t lcd_axs15231b_display_on_off(lcd_axs15231b_handle_t panel, int on);

/**
 * @brief 同步绘制左闭右开矩形 [x1,x2) x [y1,y2)，每行紧密排列。
 *
 * @param panel 已初始化的面板句柄。
 * @param x1 矩形左边界，包含；
 *     单位像素。
 * @param y1 矩形上边界，包含；
 *     单位像素。
 * @param x2 矩形右边界，不包含，不大于 width。
 * @param y2 矩形下边界，不包含，不大于 height。
 * @param pixels 至少 (x2-x1)*(y2-y1)*2 字节；
 *     板级保证线上 RGB565 高字节在前。
 *
 * @return STM_OK；
 *     空指针或空/倒置矩形为 STM_ERR_INVALID_ARG；
 *     越界/长度溢出为 STM_ERR_OUT_OF_RANGE；
 *     未初始化为 STM_ERR_INVALID_STATE；
 *     回调错误原样返回。
 *
 * @note y1 为 0 时使用 RAMWR(0x2C)，否则使用 RAMWRC(0x3C)，沿用当前模组协议。
 *     返回前缓冲保持有效，不能将未完成的异步 DMA 冒充同步成功。
 *     错误可能已写入部分像素，不回滚，不自动重试，实例状态保留。
 *     板级负责总线仲裁及 DMA/cache；
 *     多笔窗口/像素操作须防止并发交错。
 */
stm_err_t lcd_axs15231b_draw_bitmap(lcd_axs15231b_handle_t panel,
                                    uint16_t x1,
                                    uint16_t y1,
                                    uint16_t x2,
                                    uint16_t y2,
                                    const void *pixels);

#ifdef __cplusplus
}
#endif
#endif
