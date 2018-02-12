#ifndef SYSCON_H_
#define SYSCON_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "LPC8xx.h"

#define CLK_SYS       (1<<0)
#define CLK_ROM       (1<<1)
#define CLK_RAM       (1<<2)
#define CLK_FLASHREG  (1<<3)
#define CLK_FLASH     (1<<4)
#define CLK_I2C       (1<<5)
#define CLK_GPIO      (1<<6)
#define CLK_SWM       (1<<7)
#define CLK_SCT       (1<<8)
#define CLK_WKT       (1<<9)
#define CLK_MRT       (1<<10)
#define CLK_SPI0      (1<<11)
#define CLK_SPI1      (1<<12)
#define CLK_CRC       (1<<13)
#define CLK_UART0     (1<<14)
#define CLK_UART1     (1<<15)
#define CLK_UART2     (1<<16)
#define CLK_WWDT      (1<<17)
#define CLK_IOCON     (1<<18)
#define CLK_ACMP      (1<<19)
#define syscon_enable_clocks(clks)    LPC_SYSCON->SYSAHBCLKCTRL |= (clks)

#define RST_SPI0      (1<<0)
#define RST_SPI1      (1<<1)
#define RST_UARTFRG   (1<<2)
#define RST_UART0     (1<<3)
#define RST_UART1     (1<<4)
#define RST_UART2     (1<<5)
#define RST_I2C       (1<<6)
#define RST_MRT       (1<<7)
#define RST_SCT       (1<<8)
#define RST_WKT       (1<<9)
#define RST_GPIO      (1<<10)
#define RST_FLASH     (1<<11)
#define RST_ACMP      (1<<12)
#define syscon_reset_peripherals(ps)  do{ LPC_SYSCON->PRESETCTRL &= ~(ps); LPC_SYSCON->PRESETCTRL |= (ps); }while(0)

#ifdef __cplusplus
}
#endif
#endif