# STM32F4 面板接入示例

[example.c](example.c) 演示 HAL 初始化 → board_init → SPI IO 适配 → 构造通用 panel → 可选复位 → 初始化 → 显示开启。它不是可直接烧录的完整板级项目，也不创建 UI 或实际像素内容。

应用必须实现 example.c 声明的 `board_init`，提供时钟、GPIO、SPI、CS/DC、超时、可选 reset/control_context；必要时初始化 DMA/IRQ。配置来源必须是实际接线，不能直接采用占位引脚。

CMake 链接 `stm_lcd_axs15231b` 和 `stm_lcd_spi_f4`，适配要求真实 `stm32_hal` target。调用方静态持有清零适配和 panel 句柄；构造不初始化外设。示例不操作 PD15 或背光。

启用异步时应用显式转发 HAL SPI 完成/错误回调，主循环服务 IO；使用 LVGL port 后改为由 port process 服务，不同时运行两条路径。初始化失败不继续绘图，删除面板后再 deinit IO。所有返回值需要纳入消费工程错误处理。

具体契约与依赖选项见[组件 README](../../README.md)；未经确认不烧录或擦写。
