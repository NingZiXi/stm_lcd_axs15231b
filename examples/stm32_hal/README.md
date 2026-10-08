# STM32F407 HAL 接入参考

这是最小板级接口示例，不是带 `.ioc`、完整时钟和实际接线的固件工程。
`board_init()` 故意返回 `STM_ERR_INVALID_CONFIG`；实现 SPI mode 0 / 8-bit、CS/DC/RST GPIO、供电及句柄赋值后再运行。
调用顺序为 create -> reset -> init -> display_on_off。RGB565 像素在线上高字节在前。

示例不要求模组引出背光控制脚；背光或电源如可控，由实际板级另行管理，不能用 MCU GPIO 给背光供电。
共享 SPI 时对命令及其全部数据保持 CS/事务锁，并防止窗口与像素写入交错；不要从 ISR 调用。
示例本身未进行实板验收，不能由主机编译结果推定模组时序和颜色正确。
