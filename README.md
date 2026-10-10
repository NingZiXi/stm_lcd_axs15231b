# stm_lcd_axs15231b

可移植 AXS15231B 面板协议驱动，只依赖 `stm_lcd` / `stm_common`，不依赖 HAL 或 LVGL。构造返回 `stm_lcd_panel_handle_t`，后续操作统一使用公共面板 API。

## 🤖 让 Agent 帮助接入

复制以下 Prompt，将 `<...>` 替换为实际需求：

> 将 [stm32-hal-lib](https://github.com/NingZiXi/stm32-hal-lib) 中的 `stm_lcd_axs15231b` 接入当前工程，实现 `<功能>`。硬件：`<MCU/器件型号、外设与引脚、屏幕尺寸与方向（按需填写）>`。先读组件 AGENTS.md 和 README，核对配置，保留已有代码；缺失信息先询问，不猜接线。完成后说明版本、编译结果和未验证项；未经确认不烧录或擦写。

💡 可使用 [Gitee 镜像](https://gitee.com/nzxhg/stm32-hal-lib)。需要业务入口和日志时，参考 [skills/README.md](https://github.com/NingZiXi/stm32-hal-lib/blob/main/skills/README.md)（仅适用于已有 CubeMX1 CMake 工程）；编译通过不代表实板验证通过。

## 目录与职责

- `include/stm_lcd_axs15231b.h`：芯片配置与构造函数。
- `src/stm_lcd_axs15231b.c`：初始化表、窗口命令、同步/异步像素传输。
- `examples/stm32_hal/`：板级占位示例，需实现 `board_init`，不是可直接烧录的项目。
- `tests/`：面板协议、非连续窗口、异步所有权与 C/C++ 头文件检查。

GPIO、SPI、DMA、CS/DC、IRQ、缓存一致性和背光由板级/IO 适配负责。HAL 适配在 `stm_lcd/adapters/stm32f4`，不要复制回芯片驱动。

## 最小使用过程

下面函数在应用已初始化 IO 后调用，延时回调由应用提供；硬件复位由板级在调用前协调。尺寸是已验证模组的配置示例，不代表其他模组可共用初始化表。

```c
#include "stm_lcd_axs15231b.h"

stm_err_t create_panel(stm_lcd_io_handle_t io,
                       void (*delay_ms)(void *, uint32_t),
                       stm_lcd_panel_handle_t *panel)
{
    lcd_axs15231b_config_t config =
    {
        .io = io,
        .width = 170,
        .height = 560,
        .delay_ms = delay_ms,
    };
    stm_err_t err = lcd_axs15231b_create(&config, panel);
    if (err != STM_OK)
    {
        return err;
    }
    err = stm_lcd_panel_init(*panel);
    if (err == STM_OK)
    {
        err = stm_lcd_panel_display_on_off(*panel, 1);
    }
    if (err != STM_OK)
    {
        (void)stm_lcd_panel_delete(panel);
    }
    return err;
}
```

调用前 `*panel` 必须为 NULL。完整 HAL 示例见 [example.c](examples/stm32_hal/example.c) 和[示例说明](examples/stm32_hal/README.md)。如果配置 `reset` 回调，可在 init 前调用 `stm_lcd_panel_reset()`；reset 低有效，传入 0/1 为电平。不得重复复位已经接入 port 的共享触摸。

## 配置与绘图契约

| 配置 | 说明 |
| --- | --- |
| `io` | 必须支持命令和同步像素；异步能力可选，生命周期覆盖面板 |
| `width` / `height` | 非零逻辑尺寸；不自动旋转，也不替换模组时序表 |
| `x_gap` / `y_gap` | 控制器窗口偏移，必须保证加偏移后的坐标有效 |
| `bgr` | 布尔颜色顺序，固定 RGB565；不是字节交换开关 |
| `delay_ms` | 必需的启动延时，初始化过程可阻塞 |
| `reset` / `control_context` | 可选低有效复位及持续有效的控制上下文 |

- 绘图区域为 `[x1, x2) × [y1, y2)`，缓冲至少包含 `(x2-x1)*(y2-y1)*2` 字节。
- 每次绘图独立发送 CASET、RASET、RAMWR，不根据窗口位置选择 RAMWRC 续写。
- init 未成功不能绘图；同步返回后可复用缓冲，异步安全完成前不可修改或释放。
- 驱动不修改像素字节序；LVGL 场景可用 port 的 `rgb565_swap`，避免重复交换。
- 构造只分配对象、复制配置，不访问总线；失败保持空输出，非空输出拒绝覆盖。
- 借用 IO，删除不删除 IO；port 订阅或传输在途时删除返回 INVALID_STATE。

## 返回值与常见问题

参数/配置错误检查尺寸、IO 能力和延时；未初始化、在途或所有者冲突返回 INVALID_STATE；缺少 reset 或异步能力返回 NOT_SUPPORTED；通信错误由 IO 返回。

若局部残影，先检查独立窗口命令、渲染前脏区域扩展和缓冲归还时机；不要在 flush 传输时扩大区域并发送未渲染像素。颜色异常先区分 BGR 与 RGB565 字节序，不以改初始化表试错。无显示还需确认模组供电、复位及板级背光配置。

## CMake 与依赖


```cmake
add_subdirectory(Lib/stm_lcd_axs15231b)
target_link_libraries(your_firmware PRIVATE stm_lcd_axs15231b)
```

独立依赖回归包含本地/离线/固定拉取及 C11/C++17，默认从本地 `stm_lcd` Git checkout 获取且不访问网络。该 checkout 必须包含上述固定提交；普通源码拷贝可另外用 `--lcd-repository <本地Git镜像>` 提供提交来源。真实网络检查须显式指定远程 `--lcd-repository`：

```sh
python tests/test_dependency.py --lcd-source /absolute/path/to/stm_lcd --common-source /absolute/path/to/stm_common
```

独立接入只需添加本组件并链接其 target；不必手动下载 `stm_lcd`。两个 AXS 组件共用依赖，先添加任意一个均可，不重复获取。测试加 `--peer-source <另一AXS组件目录>` 可覆盖双组件添加顺序，加 `--build-dir <目录>` 可保留日志。

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


## 主机测试

需要 CMake 3.22+、Ninja、支持 C11/C++17 的主机编译器。从**组件根目录**执行：

```sh
cmake -S tests -B build/tests -G Ninja
cmake --build build/tests
ctest --test-dir build/tests --output-on-failure
```

测试时需将 `stm_common` 和 `stm_lcd` 放在本组件同级（`stm_lcd` 自身只需同级 `stm_common`），或按测试 CMake 的要求准备依赖。嵌入式交叉编译器不用于运行主机测试。可显式传入 `-DCMAKE_C_COMPILER=<主机C编译器>` 和 `-DCMAKE_CXX_COMPILER=<主机C++编译器>`。

保留三项协议测试：初始化/面板、非连续脏矩形、异步完成所有权；同时编译 C11/C++17 消费头文件。

## 验证范围与许可

2026-10-10，消费工程在 STM32F407 + AXS15231B、21 MHz SPI、170×560 原生竖屏、RGB565、两个 16 行普通 SRAM 缓冲的配置下完成显示和触摸验收。当前文档整理不改变这一运行配置。

该结果不等于其他模组、其他 MCU 或长时间稳定性验收；既有触摸畸形帧问题不能标记为已修复。统一接口已替换旧 API，接入时以实际检出的公开头文件及提交为准，不将其视为旧版本兼容补丁，也不把依赖固定提交当作正式 Release。

维护者的 MIT 许可证保持原内容，见 [LICENSE](LICENSE)。源码中已有的第三方来源及许可说明保持保留。Agent 工作约束见 [AGENTS.md](AGENTS.md)。
