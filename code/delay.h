/**
 * delay.h
 * 
 * Implements stalls/delays using MRT channel 3.
 */
#ifndef DELAY_H_
#define DELAY_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "LPC8xx.h"

#define us_to_ticks(us) (((unsigned long long)__SYSTEM_CLOCK*(us))/1000000)
#define ms_to_ticks(ms) us_to_ticks((ms)*1000)

void stall_ticks(uint32_t ticks);
#define stall_us(us) stall_ticks(us_to_ticks(us))

void delay_ticks(uint32_t ticks);
#define delay_us(us) delay_ticks(us_to_ticks(us))
#define delay_ms(ms) delay_ticks(ms_to_ticks(ms))

#ifdef __cplusplus
}
#endif
#endif