#ifndef PINS_H_
#define PINS_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "LPC8xx.h"

/* Pin configuration:
  1	   SPI0_SSEL  (/RTC_SS) Slave Select for SPI0.
  2    PIO0_12    (SW1) General purpose digital input/output pin.
  3    PIO0_5     (SW2) General purpose digital input/output pin.
  4    U0_TXD     Transmitter output for USART0.
  5    SPI0_SCK   Serial clock for SPI0.
  6    SPI0_MISO  Master In Slave Out for SPI0.
  7    PIO0_11    (unused) General purpose digital input/output pin.
  8    PIO0_10    (LATCH) General purpose digital input/output pin.
  9    CTOUT_1    (SPKR2) SCT output 1.
  10   CTOUT_0    (SPKR1) SCT output 0.
  11   PIO0_8     (SW3) General purpose digital input/output pin.
  12   VDD        VDD
  13   VSS        VSS
  14   SPI0_MOSI  Master Out Slave In for SPI0.
  15   PIO0_6     (/RTC_INT) General purpose digital input/output pin.
  16   U0_RXD     Receiver input for USART0.
*/
#define PIN_nRTC_SS   13
#define PIN_SW1       12
#define PIN_SW2       5
#define PIN_SCK       3
#define PIN_MISO      2
#define PIN_LATCH     10
#define PIN_SPKR2     1
#define PIN_SPKR1     9
#define PIN_MOSI      7
#define PIN_SW3       8
#define PIN_nRTC_INT  6

/* this is an AVR-GCC idiom */
#define _BV(n)            (1UL<<(n))

#define pins_input(bits)  LPC_GPIO_PORT->DIR0 &= ~(bits)
#define pins_output(bits) LPC_GPIO_PORT->DIR0 |=  (bits)
#define pins_high(bits)   LPC_GPIO_PORT->SET0 =   (bits)
#define pins_low(bits)    LPC_GPIO_PORT->CLR0 =   (bits)

#define pin_input(p)      pins_input(_BV(p))
#define pin_output(p)     pins_output(_BV(p))
#define pin_high(p)       pins_high(_BV(p))
#define pin_low(p)        pins_low(_BV(p))

void pins_init(void);

#ifdef __cplusplus
}
#endif
#endif
