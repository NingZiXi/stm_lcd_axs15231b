/* F407 HAL transport example; supply wiring/clock/extern SPI handle in board_init(). */
#include "stm32f4xx_hal.h"
#include "stm_lcd_axs15231b.h"
#include <limits.h>
typedef struct {
    SPI_HandleTypeDef *spi;
    GPIO_TypeDef *cs_port,*dc_port,*rst_port,*bl_port;
    uint16_t cs_pin,dc_pin,rst_pin,bl_pin;
} board_t;
static board_t board;
static stm_err_t board_init(void) {
    /* TODO: initialize SPI (8-bit, mode 0), output pins, reset/backlight and supply
       their handles/ports/pins in board. Do not use the GD25Q128 bus without
       proper CS isolation and transaction-level arbitration. */
    return STM_ERR_INVALID_CONFIG;
}
static stm_err_t send_bytes(const void *data,size_t length) {
    const uint8_t *p=data;
    while (length) {
        uint16_t n=(uint16_t)(length>UINT16_MAX ? UINT16_MAX : length);
        HAL_StatusTypeDef r=HAL_SPI_Transmit(board.spi,(uint8_t *)p,n,1000);
        if (r!=HAL_OK) return r==HAL_TIMEOUT ? STM_ERR_TIMEOUT : STM_ERR_IO;
        p+=n;length-=n;
    }
    return STM_OK;
}
static stm_err_t transfer(void *io,uint8_t cmd,const void *data,size_t length) {
    (void)io;
    /* A shared bus must be locked here across command + data + CS release. */
    HAL_GPIO_WritePin(board.cs_port,board.cs_pin,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(board.dc_port,board.dc_pin,GPIO_PIN_RESET);
    stm_err_t err=send_bytes(&cmd,1);
    if (err==STM_OK && length) {
        HAL_GPIO_WritePin(board.dc_port,board.dc_pin,GPIO_PIN_SET);
        err=send_bytes(data,length);
    }
    HAL_GPIO_WritePin(board.cs_port,board.cs_pin,GPIO_PIN_SET);
    return err;
}
static stm_err_t tx_param(void *io,uint8_t cmd,const uint8_t *data,size_t n) {return transfer(io,cmd,data,n);}
static stm_err_t tx_color(void *io,uint8_t cmd,const void *pixels,size_t n) {return transfer(io,cmd,pixels,n);}
static stm_err_t reset(void *io,int high) {
    (void)io;HAL_GPIO_WritePin(board.rst_port,board.rst_pin,high?GPIO_PIN_SET:GPIO_PIN_RESET);return STM_OK;
}
static void delay_ms(void *io,uint32_t ms) {(void)io; HAL_Delay(ms);}
int main(void) {
    HAL_Init();
    if (board_init()!=STM_OK) for (;;) {} /* wire board_init before executing */
    HAL_GPIO_WritePin(board.bl_port,board.bl_pin,GPIO_PIN_RESET);
    lcd_axs15231b_handle_t lcd=NULL;
    lcd_axs15231b_config_t cfg={.tx_param=tx_param,.tx_color=tx_color,.reset=reset,
        .delay_ms=delay_ms,.width=170,.height=560,.io=&board};
    stm_err_t err=lcd_axs15231b_create(&cfg,&lcd);
    if (err==STM_OK) err=lcd_axs15231b_reset(lcd);
    if (err==STM_OK) err=lcd_axs15231b_init(lcd);
    if (err==STM_OK) err=lcd_axs15231b_display_on_off(lcd,1);
    if (err==STM_OK) HAL_GPIO_WritePin(board.bl_port,board.bl_pin,GPIO_PIN_SET);
    else { lcd_axs15231b_delete(&lcd); for (;;) {} }
    for (;;) { /* lcd_axs15231b_draw_bitmap(lcd,x1,y1,x2,y2,rgb565_bytes); */ }
}
