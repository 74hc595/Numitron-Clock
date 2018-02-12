#include "LPC8xx.h"
#include "pins.h"

void pins_init()
{
  /* Configure switch matrix */
  LPC_SWM->PINASSIGN3 = 0x00ffffffUL | (PIN_SCK<<24);
  LPC_SWM->PINASSIGN4 = 0xff000000UL | (PIN_nRTC_SS<<16) | (PIN_MISO<<8) | PIN_MOSI;
  LPC_SWM->PINENABLE0 = 0xffffffffUL;
  LPC_SWM->PINASSIGN6 = 0x00ffffffUL | (PIN_SPKR1<<24);
  LPC_SWM->PINASSIGN7 = 0xffffff00UL | PIN_SPKR2;
  pin_output(PIN_LATCH);
  pin_low(PIN_LATCH);
}
