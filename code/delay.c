#include "delay.h"

void stall_ticks(uint32_t ticks)
{
  LPC_MRT->Channel[3].CTRL = 2; /* One-shot bus stall */
  LPC_MRT->Channel[3].INTVAL = ticks | (1UL<<31);
}

void delay_ticks(uint32_t ticks)
{
  LPC_MRT->Channel[3].CTRL = 1; /* One-shot interrupt */
  LPC_MRT->Channel[3].INTVAL = ticks | (1UL<<31);
  while ((LPC_MRT->Channel[3].STAT & 1) == 0) {}
  LPC_MRT->Channel[3].STAT = 1;
}
