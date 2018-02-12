/**
 * PCF2129 SPI routines
 */
#ifndef RTC_H_
#define RTC_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "pcf2129.h"

/**
 * Since the day and weekday alarm are not used, but their values
 * are battery-backed, use them for 9 bits of persistent settings.
 * The msb of each byte must be 1 to prevent spurious alarms.
 */
typedef union {
  struct {
    flag_t in_dst:1;
    flag_t auto_dst:1;
    flag_t sleep:1;
    flag_t date_cycling :1;
    bits_t display_style:2;
    pad_t               :1;
    flag_t must_be_1    :1;
    bits_t brightness   :3;
    pad_t               :4;
    flag_t must_be_1_2  :1;
  } PACKED;
  uint8_t b[2];
} STRUCT_SIZE_CHECKED(rtc_settings_t, RTC_NUM_SETTINGS_REGISTERS);

/**
 * Entire nonvolatile state.
 * (registers 0x03 through 0x0E)
 */
typedef union {
  struct {
    rtc_time_t time;
    rtc_alarm_time_t alarm;
    rtc_settings_t settings;
  } PACKED;
  uint8_t b[12];
} STRUCT_SIZE_CHECKED(rtc_state_t, RTC_NUM_STATE_REGISTERS);


enum rtc_event {
  RTC_EV_SECOND = (1<<0), /* second interrupt */
  RTC_EV_ALARM  = (1<<1)  /* alarm flag */
};
typedef uint8_t rtc_event_set_t;

/**
 * Initializes the real-time clock.
 * Blocks until the oscillator has started.
 * Returns nonzero if the oscillator does not start within an acceptable
 * period of time. (possibly because the RTC is not present on the bus)
 */
int rtc_init(void);

uint8_t rtc_read_register(uint8_t reg);

void rtc_write_register(uint8_t reg, uint8_t value);

rtc_event_set_t rtc_poll();

void rtc_get_state(rtc_state_t *st);
void rtc_set_time_and_date(const rtc_time_t *t);
void rtc_set_time_only(const rtc_time_t *t);
void rtc_set_date_only(const rtc_time_t *t);
void rtc_set_alarm_time(const rtc_alarm_time_t *t);
bool rtc_alarm_is_enabled(const rtc_alarm_time_t *t);
void rtc_set_alarm_enabled(rtc_alarm_time_t *t, int enable);
void rtc_clear_alarm(void);

bool rtc_battery_is_low(void);

void rtc_update_settings(const rtc_settings_t *t);

void rtc_update_dst_flag(rtc_state_t *st);
void rtc_set_date_with_dst_update(rtc_state_t *st);
void rtc_set_time_and_date_with_dst_update(rtc_state_t *st);
void rtc_dst_transition_check(rtc_state_t *st);

/* Global state. */
extern rtc_state_t rtc;

#ifdef __cplusplus
}
#endif
#endif
