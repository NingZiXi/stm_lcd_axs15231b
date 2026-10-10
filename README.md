# stm_lcd_axs15231b

可移植 AXS15231B 面板驱动，只依赖 `stm_lcd` / `stm_common`，不依赖 HAL/LVGL。

`lcd_axs15231b_create(&config, &panel)` 返回 `stm_lcd_panel_handle_t`；配置包含通用 IO、尺寸/gap、BGR、可选 reset、必要 delay 和控制上下文。后续调用 `stm_lcd_panel_reset/init/display_on_off/draw_bitmap/draw_bitmap_async/delete`。没有旧操作 API 兼容包装。

- IO 必須支持命令和同步像素；异步可选，不支持返回 NOT_SUPPORTED。
- 保留原始初始化表、RGB565 和每次独立 CASET/RASET/RAMWR；右下边界不包含，不依赖连续窗口续写。
- init 失败禁止继续绘图；reset 不存在返回 NOT_SUPPORTED。缓冲字节序由应用/port 配置，驱动不修改像素。
- 构造失败保持空输出；非空输出拒绝覆盖。借用 IO，删除不销毁共享硬件；pending 或被 port 订阅时删除拒绝。
- HAL 适配迁移到 `stm_lcd/adapters/stm32f4`；此组件仅提供芯片协议。

CMake 优先复用 `stm_lcd` target 或同级源码，缺失明确报错，不拉取未经验证的框架。测试目录保留面板、非连续窗口和异步协议所有权三项及 C/C++ 头编译。

示例需实现 board_init 后使用，不能直接烧录占位配置。

## 验证与许可

2026-10-10，在 STM32F407 + AXS15231B 消费工程以 21 MHz SPI、170×560 原生竖屏、RGB565、两个 16 行普通 SRAM 缓冲运行，用户确认显示与触摸正常。该结论仅覆盖上述配置，未作为独立长时间 soak 验收，也不宣称既有触摸畸形帧问题已修复。当前为未发布开发提交，未创建新 tag 或 Release。

自有源码沿用维护者原 MIT，见 [LICENSE](LICENSE)。 必要第三方许可正文保留于 [LICENSE-ESPRESSIF](LICENSE-ESPRESSIF)。
