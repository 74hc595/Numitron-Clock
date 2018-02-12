/**
 * Register definitions for NXP PCF2129 real-time clock
 */
#ifndef PCF2129_H_
#define PCF2129_H_

#include <stdint.h>
#include "bcd.h"

typedef uint8_t flag_t;
typedef uint8_t bits_t;
typedef uint8_t pad_t;

#define RTC_READ_ADDR    0b10100000
#define RTC_WRITE_ADDR   0b00100000
#define RTC_READ_REG(r)  (RTC_READ_ADDR|((r)&0x1F))
#define RTC_WRITE_REG(r) (RTC_WRITE_ADDR|((r)&0x1F))

#define PACKED __attribute__((aligned(1),packed))

#define STRUCT_SIZE_CHECKED(n, sz) n; static_assert(sizeof(n)==sz, "sizeof(" #n ") != " #sz);

enum pcf2129_register {
  RTC_CONTROL_1,
  RTC_CONTROL_2,
  RTC_CONTROL_3,
  RTC_SECONDS,
  RTC_MINUTES,
  RTC_HOURS,
  RTC_DAYS,
  RTC_WEEKDAYS,
  RTC_MONTHS,
  RTC_YEARS,
  RTC_SECOND_ALARM,
  RTC_MINUTE_ALARM,
  RTC_HOUR_ALARM,
  RTC_DAY_ALARM,
  RTC_WEEKDAY_ALARM,
  RTC_CLKOUT_CTL,
  RTC_WATCHDOG_TIM_CTL,
  RTC_WATCHDOG_TIM_VAL,
  RTC_TIMESTP_CTL,
  RTC_SEC_TIMESTP,
  RTC_MIN_TIMESTP,
  RTC_HOUR_TIMESTP,
  RTC_DAY_TIMESTP,
  RTC_MON_TIMESTP,
  RTC_YEAR_TIMESTP,
  RTC_AGING_OFFSET,
  RTC_INTERNAL_REG_1A,
  RTC_INTERNAL_REG_1B,
  NUM_RTC_REGISTERS,
  LAST_RTC_REG = NUM_RTC_REGISTERS-1,
  RTC_FIRST_STATE_REGISTER    = RTC_SECONDS,
  RTC_FIRST_TIME_REGISTER     = RTC_SECONDS,
  RTC_FIRST_DATE_REGISTER     = RTC_DAYS,
  RTC_FIRST_ALARM_REGISTER    = RTC_SECOND_ALARM,
  RTC_FIRST_SETTINGS_REGISTER = RTC_DAY_ALARM,
  RTC_NUM_TIME_REGISTERS      = RTC_FIRST_DATE_REGISTER-RTC_FIRST_TIME_REGISTER,
  RTC_NUM_DATE_REGISTERS      = RTC_FIRST_ALARM_REGISTER-RTC_FIRST_DATE_REGISTER,
  RTC_NUM_ALARM_REGISTERS     = RTC_FIRST_SETTINGS_REGISTER-RTC_FIRST_ALARM_REGISTER,
  RTC_NUM_SETTINGS_REGISTERS  = 2,
  RTC_NUM_STATE_REGISTERS     = RTC_NUM_TIME_REGISTERS+RTC_NUM_DATE_REGISTERS+RTC_NUM_ALARM_REGISTERS+RTC_NUM_SETTINGS_REGISTERS
};

/* Time and date registers */
typedef union {
  struct {
    bcd_t  seconds:7;
    bcd_t  OSF:1;
    bcd_t  minutes:8;
    bcd_t  hours:8; /* always in 24-hour mode */
    bcd_t  days:8;
    bits_t weekdays:8;
    bcd_t  months:8;
    bcd_t  years:8;
  } PACKED;
  uint8_t b[7];
} STRUCT_SIZE_CHECKED(rtc_time_t, RTC_NUM_TIME_REGISTERS+RTC_NUM_DATE_REGISTERS);

/* Second/minute/hour alarm registers */
/* (day and weekday alarms are not used) */
typedef union {
  struct {
    bcd_t  seconds:7;
    flag_t nAE_S:1;
    bcd_t  minutes:7;
    flag_t nAE_M:1;
    bcd_t  hours:7;
    flag_t nAE_H:1;
  } PACKED;
  uint8_t b[3];
} STRUCT_SIZE_CHECKED(rtc_alarm_time_t, RTC_NUM_ALARM_REGISTERS);

#endif
