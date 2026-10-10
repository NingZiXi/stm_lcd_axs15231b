# stm_lcd_axs15231b

可移植 AXS15231B 面板驱动，只依赖 `stm_lcd` / `stm_common`，不依赖 HAL/LVGL。

`lcd_axs15231b_create(&config, &panel)` 返回 `stm_lcd_panel_handle_t`；配置包含通用 IO、尺寸/gap、BGR、可选 reset、必要 delay 和控制上下文。后续调用 `stm_lcd_panel_reset/init/display_on_off/draw_bitmap/draw_bitmap_async/delete`。没有旧操作 API 兼容包装。

- IO 必須支持命令和同步像素；异步可选，不支持返回 NOT_SUPPORTED。
- 保留原始初始化表、RGB565 和每次独立 CASET/RASET/RAMWR；右下边界不包含，不依赖连续窗口续写。
- init 失败禁止继续绘图；reset 不存在返回 NOT_SUPPORTED。缓冲字节序由应用/port 配置，驱动不修改像素。
- 构造失败保持空输出；非空输出拒绝覆盖。借用 IO，删除不销毁共享硬件；pending 或被 port 订阅时删除拒绝。
- HAL 适配迁移到 `stm_lcd/adapters/stm32f4`；此组件仅提供芯片协议。

CMake 优先复用 `stm_lcd` target/本地源码，缺失时自动获取固定提交，详见下方依赖说明。测试目录保留面板、非连续窗口和异步协议所有权三项及 C/C++ 头编译。

示例需实现 board_init 后使用，不能直接烧录占位配置。

## CMake 与依赖

```cmake
add_subdirectory(Lib/stm_lcd_axs15231b)
target_link_libraries(your_firmware PRIVATE stm_lcd_axs15231b)
```

独立依赖回归（含本地/离线/固定拉取及 C11/C++17，默认从本地 `stm_lcd` Git checkout 获取，不访问网络；真实网络检查可显式指定 `--lcd-repository`）：

```sh
python tests/test_dependency.py --lcd-source /absolute/path/to/stm_lcd --common-source /absolute/path/to/stm_common
```

独立接入只需添加本组件并链接其 target；不必手动下载 `stm_lcd`。两个 AXS 组件共用依赖，先添加任意一个均可，不重复获取。

`stm_lcd` 解析顺序：已有 target（含 alias） → `STM_LCD_SOURCE_DIR` → `FETCHCONTENT_SOURCE_DIR_STM_LCD` → 同级 `../stm_lcd` 源码 → 自动下载。默认固定已验证开发提交 `da713eecea80c5abb59973a3b003015800011b4a`，不跟随 `main`，不代表已创建正式版本。下载源码位于构建目录的 `_deps/stm_lcd-src`。

| 选项 | 默认值 | 用途 |
| --- | --- | --- |
| `STM_LCD_FETCH` | `ON` | 缺失时允许获取；`OFF` 仍可复用已有 target 或本地源码 |
| `STM_LCD_SOURCE_DIR` | 空 | 显式指定本地框架目录；无效时明确报错，不回退网络 |
| `FETCHCONTENT_SOURCE_DIR_STM_LCD` | 空 | 框架离线源码覆盖，同样可在关闭下载时使用 |
| `STM_LCD_GIT_REPOSITORY` | GitHub `NingZiXi/stm_lcd.git` | 可信镜像，例如 Gitee `https://gitee.com/nzxhg/stm_lcd.git` |

`stm_common` 保持优先复用 target/同级源码，缺失时固定获取 v1.0.0 提交；可使用 `STM_COMMON_FETCH=OFF`、`STM_COMMON_GIT_REPOSITORY` 和 `FETCHCONTENT_SOURCE_DIR_STM_COMMON`。完全离线请同时提供两个依赖的 target/本地源码并关闭下载。`FETCHCONTENT_FULLY_DISCONNECTED=ON` 且框架首次缺失时会明确报错。

```cmake
# 可选：使用可信镜像；两个 AXS 组件共享这些选项。
set(STM_LCD_GIT_REPOSITORY "https://gitee.com/nzxhg/stm_lcd.git" CACHE STRING "")
# 或离线使用本地框架，并禁止下载：
# set(STM_LCD_SOURCE_DIR "D:/components/stm_lcd" CACHE PATH "")
# set(STM_LCD_FETCH OFF CACHE BOOL "")
```

## 验证与许可

2026-10-10，在 STM32F407 + AXS15231B 消费工程以 21 MHz SPI、170×560 原生竖屏、RGB565、两个 16 行普通 SRAM 缓冲运行，用户确认显示与触摸正常。该结论仅覆盖上述配置，未作为独立长时间 soak 验收，也不宣称既有触摸畸形帧问题已修复。当前为未发布开发提交，未创建新 tag 或 Release。

自有源码沿用维护者原 MIT，见 [LICENSE](LICENSE)。
