#ifndef EVENT_H_
#define EVENT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "button.h"
#include "rtc.h"

#define BTN1_EV_SHIFT 0
#define BTN2_EV_SHIFT 8
#define BTN3_EV_SHIFT 16
#define RTC_EV_SHIFT  24

enum event {
  /* Button 1 events */
  BTN1_PRESS   = (BTN_EV_PRESS   << BTN1_EV_SHIFT),
  BTN1_RELEASE = (BTN_EV_RELEASE << BTN1_EV_SHIFT),
  BTN1_TAP     = (BTN_EV_TAP     << BTN1_EV_SHIFT),
  BTN1_HOLD    = (BTN_EV_HOLD    << BTN1_EV_SHIFT),
  BTN1_REPEAT  = (BTN_EV_REPEAT  << BTN1_EV_SHIFT),
  BTN1_ANY     = (0xFF           << BTN1_EV_SHIFT),
  /* Button 2 events */
  BTN2_PRESS   = (BTN_EV_PRESS   << BTN2_EV_SHIFT),
  BTN2_RELEASE = (BTN_EV_RELEASE << BTN2_EV_SHIFT),
  BTN2_TAP     = (BTN_EV_TAP     << BTN2_EV_SHIFT),
  BTN2_HOLD    = (BTN_EV_HOLD    << BTN2_EV_SHIFT),
  BTN2_REPEAT  = (BTN_EV_REPEAT  << BTN2_EV_SHIFT),
  BTN2_ANY     = (0xFF           << BTN2_EV_SHIFT),
  /* Button 3 events */
  BTN3_PRESS   = (BTN_EV_PRESS   << BTN3_EV_SHIFT),
  BTN3_RELEASE = (BTN_EV_RELEASE << BTN3_EV_SHIFT),
  BTN3_TAP     = (BTN_EV_TAP     << BTN3_EV_SHIFT),
  BTN3_HOLD    = (BTN_EV_HOLD    << BTN3_EV_SHIFT),
  BTN3_REPEAT  = (BTN_EV_REPEAT  << BTN3_EV_SHIFT),
  BTN3_ANY     = (0xFF           << BTN3_EV_SHIFT),
  /* Real-time clock events */
  RTC_SECOND   = (RTC_EV_SECOND  << RTC_EV_SHIFT),
  RTC_ALARM    = (RTC_EV_ALARM   << RTC_EV_SHIFT),
  /* Other events */
  FLASH_FAST   = (1 << 28),
  FLASH        = (1 << 29),
  EV_TIMEOUT   = (1 << 30),
  EV_INITIAL   = (1 << 31)
};
typedef uint32_t event_set_t;

#ifdef __cplusplus
}
#endif
#endif
