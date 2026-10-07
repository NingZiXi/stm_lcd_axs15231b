# stm_lcd_axs15231b：AXS15231B 170×560 SPI 屏幕

将 AgentDock 的 170×560 RGB565 初始化序列和 Espressif 的屏幕命令协议移植为独立 `stm_common` 组件。适用于 **4 线、8-bit 命令/数据 SPI（SPI mode 0）**；不提供 QSPI/I80/DSI。初始 MADCTL=RGB（可设 `bgr=1`），COLMOD=0x55。32 条模组初始化项来自 Espressif Apache-2.0 实现，应保留授权，见 `NOTICE` / `LICENSE-ESPRESSIF`。

## 调用

```c
lcd_axs15231b_handle_t panel = NULL;
lcd_axs15231b_config_t cfg = {
    .tx_param = board_tx_param, .tx_color = board_tx_color,
    .delay_ms = board_delay_ms, .reset = board_reset,
    .io = &board, .width = 170, .height = 560,
};
stm_err_t err = lcd_axs15231b_create(&cfg, &panel);
if (err == STM_OK) err = lcd_axs15231b_reset(panel);
if (err == STM_OK) err = lcd_axs15231b_init(panel);
if (err == STM_OK) err = lcd_axs15231b_display_on_off(panel, 1);
/* [x1,x2)×[y1,y2)，RGB565 紧密排列，每像素 2 字节。 */
if (err == STM_OK) err = lcd_axs15231b_draw_bitmap(panel, 0, 0, 2, 2, pixels);
lcd_axs15231b_delete(&panel);
```

`tx_param(io, cmd, bytes, byte_length)` 和 `tx_color(io, cmd, pixels, byte_length)` 都同步完成，零长度允许空数据；`tx_color` 必须在一笔占用 CS 与总线锁的事务内写命令和全部像素，**返回前完全消费**。非首行刷新使用 RAMWRC(0x3C)，首行使用 RAMWR(0x2C)。板级负责 RGB565 线上字节序（MSB first）、SPI/DC/CS/复位、背光、供电、总线仲裁和 DMA 缓存维护。回调上下文/像素缓冲在使用期间保持有效。不要从 ISR 调用阻塞传输；init/display/draw 非线程安全。绘图错误可能已经部分发送像素，不回滚，不自动重试。LVGL PARTIAL 同步 flush 只在 draw 返回后通知 flush_ready；错误路径也须完成缓冲释放并单独记录错误。未实现异步 DMA 完成通知。

`create` 不访问硬件，输出句柄须初始为空，失败不更改。失败回调原样传递；`STM_ERR_INVALID_ARG` 为空指针/非法矩形，`INVALID_CONFIG` 为缺回调/尺寸/色序配置，`INVALID_STATE` 为未初始化/重复创建，`OUT_OF_RANGE` 为越界，`NO_MEM` 为分配失败。reset/init 失败后需重新初始化。没有 RTOS/RTT 依赖。

## 构建与验证

```cmake
add_subdirectory(Lib/stm_lcd_axs15231b)
target_link_libraries(your_firmware PRIVATE stm_lcd_axs15231b)
```

优先复用同级 `stm_common` 或已有 target；独立构建可从固定提交下载（禁网 `-DSTM_COMMON_FETCH=OFF`）。运行主机测试：`cmake -S tests -B build && cmake --build build && ctest --test-dir build --output-on-failure`。已有 C11/C++17 头文件消费及命令/生命周期/错误测试；**没有 F407 实板时序、颜色和性能验收**。

[`examples/stm32_hal/example.c`](examples/stm32_hal/example.c) 为 F407 板级占位示例，`board_init()` 故意返回配置错误，必须根据真实接线实现后再运行。现有 F407 `.ioc` 的 SPI1 接 GD25Q128，不可复用其 CS 或假设已经接上此屏。

