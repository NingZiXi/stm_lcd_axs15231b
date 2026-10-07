# STM32F407 HAL 接入参考

这是可编译的接口示例，不是带 IOC/时钟/真实接线的工程。完成 `example.c` 中 `board_init()`，配置 SPI mode 0、8bit、独立 DC/CS/RST/背光 GPIO；先关背光、reset/init、开 display、再开背光。需要共享 GD25Q128 所在 SPI1 时，确保两个 CS 互斥并对完整命令+像素事务加锁；也可选另一 SPI。每次 flush 使用紧密排列 RGB565 高字节在前。CubeMX 工程不含该屏实际引脚，先核对接线。示例未实测，不保证时序和颜色。
