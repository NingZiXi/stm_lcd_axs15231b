# stm_lcd_axs15231b：Agent 工作指南

## 范围与阅读顺序

本文件约束本组件目录及子目录；与上级指令合并使用，存在更深层 AGENTS.md 时遵循其局部约束。README 面向使用者，本文件用于 Agent 的接入、修改和验证。

先检查消费工程构建方式、已存在的 target、外设初始化及用户未提交改动，再读本组件 README/CMake/公开头；不覆盖已有业务代码。

## 接入与不可破坏的约束

- 先读 `include/stm_lcd_axs15231b.h`、`src/stm_lcd_axs15231b.c`、`examples/stm32_hal/example.c`；再核对框架公开面板接口。
- 应用提供 IO 和控制上下文，构造返回通用 panel；没有旧 API 包装。构造不访问硬件，初始输出必须为空，失败回滚。
- 默认复用 target/本地源码；两个 AXS 共用固定 stm_lcd v1.0.0 提交 `c359e54a657be38aec90c797ea19ee3d492d9284`，不改为 main。选项与解析优先级以本地 CMake 和 README 为准。
- 不修改模组初始化表来尝试旋转/提速；170×560 为当前原生竖屏模组，不代表所有 AXS 模组相同。
- 每次独立 CASET/RASET/RAMWR，不能因 y1 非零改用 RAMWRC；绘图右下边界不包含，固定 RGB565。
- 芯片驱动不依赖 HAL/LVGL，不能塞入 CS/DC/DMA、引脚、共享复位协调或 port 调度。复用 stm_lcd 的可选适配。
- init 成功后才能绘图；缓冲字节序由应用/port 准备。异步完成前不复用缓冲，被订阅或忙时不能删除。
- 主机测试需同级 stm_common/stm_lcd，保留面板、非连续窗口、异步三个用例及 C/C++ 消费编译。

## 编码与注释

- 中文交流；C11；Allman 括号、4 空格缩进，按本目录 `.clang-format` 格式化，只处理自有代码。
- `.c` / `.h` 文件头仅有 `@file` 和一句 `@brief`，不写 author/date/version/copyright。
- 头文件结构体外的公开函数声明使用 Doxygen：brief、真实参数语义、关键返回值；void 不虚构返回值。
- struct/enum 可在上方写一句说明；成员和枚举项仅在同行右侧用简短 `//`，不使用 `/**< */`，不把字段注释写在上方。
- C 实现不写函数 Doxygen；函数前至多一句 `//`。复杂约束最多 1–3 行，不写教学块、历史或章节分隔线。
- 不改维护者 MIT LICENSE；保留现有第三方来源/许可说明，不把第三方内容直接改署名。

## 验证与交付

从本组件根目录执行，需要主机 C11/C++17 编译器、CMake 3.22+、Ninja：

```sh
cmake -S tests -B build/tests -G Ninja
cmake --build build/tests
ctest --test-dir build/tests --output-on-failure
```

没有同级依赖时先准备源码，不能删测试或伪造空 target 来通过。主机测试不用 MCU 交叉编译器。测试包含 C/C++ 头文件消费；改生命周期/协议时增加对应失败路径测试。

接入消费工程还须使用其工具链编译固件，并运行可用的真实 LVGL / HAL 回归。仅文档注释整理时校验非注释 C token 不变。报告实际依赖版本/提交、修改文件、执行命令、测试/编译结果及未验证项；编译成功不等于实板通过。

未经用户明确授权不烧录、擦写存储、推送、发布版本、修改远程仓库或自动升级依赖。缺少接线、共享复位、目标器件或构建配置时先询问，不猜测。不要修改消费者的 CubeMX 生成文件、HAL 或供应商 LVGL；必要配置列出操作步骤。

## 依赖回归

在本组件根目录执行以下命令，需要 Python 3 和 Git。测试还包含固定提交拉取和双组件解析，默认本地 Git 仓库必须包含 README 标注的固定 stm_lcd v1.0.0 提交，普通无 .git 的拷贝不能替代：

```sh
python tests/test_dependency.py --lcd-source /absolute/path/to/stm_lcd --common-source /absolute/path/to/stm_common
```

可用 `--build-dir <新目录>` 保留日志，`--lcd-repository <可信仓库>` 显式选择拉取来源；不擅自改固定提交来让测试通过。
