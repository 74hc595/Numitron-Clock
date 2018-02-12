#include "display.h"
#include "event.h"
#include "rtc.h"
#include "beeper.h"
#include "app.h"
#include "date.h"
#include "version.h"
#include "LPC8xx.h"

#define MIN(a,b) ({ auto _a = (a); auto _b = (b); _a < _b ? _a : _b; })

#define SLEEP_DELAY_TICKS 600

/**
 * Generate getters/setters for settings bitfields, because I can't seem
 * to get lambdas to work right
 */
#define WRAP_FLAG(f) \
  static void SET_##f(bool value) { rtc.settings.f = value; } \
  static bool GET_##f() { return rtc.settings.f; }

#define WRAP_VALUE(v) \
  static void SET_##v(unsigned value) { rtc.settings.v = value; } \
  static unsigned GET_##v() { return rtc.settings.v; }

WRAP_FLAG(sleep)
WRAP_FLAG(date_cycling)
WRAP_VALUE(display_style)

static bool GET_auto_dst() { return rtc.settings.auto_dst; }
static void SET_auto_dst(bool value) {
  if (rtc.settings.auto_dst != value) {
    rtc.settings.auto_dst = value;
    rtc_update_dst_flag(&rtc);
  }
}


/* Messages */
const uint16_t set_sleep_message[6]    = { 0, 0, 0, c_z, c_z, c_z };
const uint16_t set_date_message[6]     = { c_arrow, 0, c_E, c_t, c_A, c_D };
const uint16_t set_adst_message[6]     = { 0, 0, c_t, c_S, c_D, c_A|sP };
const uint16_t set_style_message[6]    = { c_arrow, c_E, c_L, c_Y, c_t, c_S };
const uint16_t set_cycle_message[6]    = { 0, 0, c_L, c_C, c_Y, c_C };
const uint16_t done_message[6]         = { c_arrow, 0, c_E, c_N, c_O, c_D };
const uint16_t battery_low_message[6]  = { c_O, c_L, 0, c_t, c_A, c_B };
const uint16_t battery_ok_message[6]   = { c_K, c_O, 0, c_t, c_A, c_B };
const uint16_t firmware_version_message[6] = {
  DIGIT_TO_PATTERN(FIRMWARE_VERSION_MINOR), DIGIT_TO_PATTERN(FIRMWARE_VERSION_MAJOR)|sP, 0, c_r|sP, c_e, c_V };

/* Temporary values shared across apps */
static bcd_t tmp1, tmp2, tmp3;
static bcd_t days_in_selected_month;

/* Apps */
const StartupApp App_Startup(&App_Clock);
const ClockApp App_Clock(&App_Date);
const DateApp App_Date(&App_Clock);
const AlarmGoingOffApp App_AlarmGoingOff(&App_Clock);

const SetTimeComponentsInitialApp App_BeginSetTime(&App_SetHour);
const SetTimeComponentApp App_SetHour(0x00, 0x23, 0b10011000, &tmp1, &App_SetMinute);
const SetTimeComponentApp App_SetMinute(0x00, 0x59, 0b000110, &tmp2, &App_FinishSetTime);
const SetTimeComponentsFinishApp App_FinishSetTime(&App_Clock);

const SetDateComponentsInitialApp App_BeginSetDate(&App_SetMonth);
const SetDateComponentApp App_SetMonth(0x01, 0x12, 0b10110000, &tmp1, &App_SetDay);
const SetDaysInMonthApp App_SetDay(0x01, &days_in_selected_month, 0b10001100, &tmp2, &App_SetYear);
const SetDateComponentApp App_SetYear(0x00, 0x99, 0b000011, &tmp3, &App_FinishSetDate);
const SetDateComponentsFinishApp App_FinishSetDate(&App_SetDatePrompt);

const SetAlarmApp App_SetAlarm(&App_SetAlarmHour);
const SetAlarmComponentApp App_SetAlarmHour(0x00, 0x23, 0b10001100, &tmp1, &App_SetAlarmMinute);
const SetAlarmComponentApp App_SetAlarmMinute(0x00, 0x59, 0b10000011, &tmp2, &App_FinishSetAlarm);
const SetAlarmComponentsFinishApp App_FinishSetAlarm(&App_Clock);

const BatteryLowCheckApp App_DateCycleStart(&App_Date);
const BatteryLowAlertApp App_BatteryLowAlert(&App_Date);

const FlagMenuApp App_SetSleepEnabled(&App_SetDatePrompt, &App_MenuDone, set_sleep_message, GET_sleep, SET_sleep);
const MenuApp App_SetDatePrompt(&App_SetAutoDST, &App_SetSleepEnabled, &App_BeginSetDate, set_date_message);
const SetAutoDSTApp App_SetAutoDST(&App_SetDisplayStylePrompt, &App_SetDatePrompt, set_adst_message, GET_auto_dst, SET_auto_dst);
const MenuApp App_SetDisplayStylePrompt(&App_SetDateCycling, &App_SetAutoDST, &App_SetDisplayStyle, set_style_message);
const FlagMenuApp App_SetDateCycling(&App_ShowBatteryStatus, &App_SetDisplayStylePrompt, set_cycle_message, GET_date_cycling, SET_date_cycling);
const BatteryStatusApp App_ShowBatteryStatus(&App_ShowFirmwareVersion, &App_SetDateCycling);
const FirmwareVersionApp App_ShowFirmwareVersion(&App_MenuDone, &App_ShowBatteryStatus, firmware_version_message);
const MenuApp App_MenuDone(&App_SetSleepEnabled, &App_ShowFirmwareVersion, &App_Clock, done_message);

const SetDisplayStyleApp App_SetDisplayStyle(&App_SetDisplayStylePrompt, GET_display_style, SET_display_style);

const SleepApp App_Sleep;


extern uint32_t flash_count;
extern uint32_t ticks_since_last_second;
static uint32_t sleep_timer = 0;


static void draw_alarm_indicator()
{
  /* Light up last decimal point if alarm is set */
  if (rtc_alarm_is_enabled(&rtc.alarm)) {
    display[0] |= sP;
  } else {
    display[0] &= (~sP);
  }
}


static __attribute__((noreturn)) void enter_bootloader()
{
  display_init();
  NVIC_SystemReset();
  while (1) {} /* should not get here */
}


void App::init() const
{
  display_buffer_clear();
  flash_count = 0;
}


void StartupApp::init() const
{
  /* Ensure display is on at appropriate brightness */
  /* when powering up or waking from sleep. */
  display_set_brightness(rtc.settings.brightness+1);
}


AppPtr MainApp::update(event_set_t events) const
{
  draw();

  if (events & BTN1_TAP) {
    rtc.settings.brightness = display_decrease_brightness_wrap()-1;
    rtc_update_settings(&rtc.settings);
  }


  if (events & BTN1_HOLD) {
#if PROGRAMMER_MODE
    enter_bootloader();
#else
    return &App_Sleep;
#endif
  }

  if (events & BTN2_TAP) {
    return &App_SetAlarm;
  }

  if (events & BTN2_HOLD) {
    return &App_BeginSetTime;
  }

  if (events & (BTN3_TAP|EV_TIMEOUT)) {
    return nextapp_;
  }

  if (events & BTN3_HOLD) {
    return &App_SetSleepEnabled;
  }

  return 0;
}


void ClockApp::draw() const
{
  switch (rtc.settings.display_style) {
    case 0:
      display_buffer_write_bcd(5, rtc.time.hours);
      display_buffer_write_bcd(3, rtc.time.minutes);
      display_buffer_write_bcd(1, rtc.time.seconds);
      if (ticks_since_last_second <= 30) {
        display[2] |= sP;
        display[4] |= sP;
      }
      break;

    case 1:
      display[5] = display[0] = 0;
      display_buffer_write_bcd(4, rtc.time.hours);
      display_buffer_write_bcd(2, rtc.time.minutes);
      if (ticks_since_last_second <= 30) {
        display[3] |= sP;
      }
      break;

    case 2:
      display[5] = 0;
      display_buffer_write_bcd(4, rtc.time.hours);
      display_buffer_write_bcd(1, rtc.time.minutes);
      display[2] = (ticks_since_last_second <= 30) ? sH|sI : 0;
      break;

    case 3:
      display_buffer_write_bcd(5, rtc.time.hours);
      display_buffer_write_bcd(3, 0xFF);
      display_buffer_write_bcd(1, rtc.time.minutes);
      if (ticks_since_last_second <= 30) {
        display[3] |= sP;
      }
      break;
  }    
  draw_alarm_indicator();
}


AppPtr ClockApp::update(event_set_t events) const
{
  if (rtc.settings.sleep) {
    if (events & EV_INITIAL) {
      sleep_timer = SLEEP_DELAY_TICKS;
    }
    /* don't decrement sleep timer if there is button activity */
    else if ((events & (BTN1_ANY|BTN2_ANY|BTN3_ANY)) == 0 && sleep_timer > 0) {
      sleep_timer--;
    }
    if (sleep_timer == 0) { return &App_Sleep; }
  }
    
  /* If date cycling is on, show the date at the 26-second mark */
  /* of even minutes. */
  if (rtc.settings.date_cycling && rtc.time.seconds == 0x26 && (rtc.time.minutes & 1) == 0) {
    return &App_DateCycleStart;
  }
  return MainApp::update(events);
}


void AlarmGoingOffApp::init() const
{
  /* Wake up from sleep if necessary */
  App_Startup.init();
  beeper_on();
}


AppPtr AlarmGoingOffApp::update(event_set_t events) const
{
  set_beeper(events & FLASH_FAST);

  if ((events & FLASH_FAST) == 0) {
    draw();
  } else {
    display_buffer_clear();
  }

  if (events & (BTN1_HOLD|BTN2_HOLD|BTN3_HOLD)) {
    beeper_off();
    rtc_clear_alarm();
    return nextapp_;
  }

  return 0;
}


void DateApp::draw() const
{
  display_buffer_write_bcd(5, suppress_leading_zero_left_align(rtc.time.months));
  display_buffer_write_bcd(3, suppress_leading_zero_left_align(rtc.time.days));
  display_buffer_write_bcd(1, rtc.time.years);
  display[2+(rtc.time.days<0x10)] |= sP;
  display[4+(rtc.time.months<0x10)] |= sP;
  draw_alarm_indicator();
}


void BatteryLowAlertApp::draw() const
{
  display_buffer_set_digits(battery_low_message);
}


void SetTimeComponentsInitialApp::init() const
{
  tmp1 = rtc.time.hours;
  tmp2 = rtc.time.minutes;
}


void SetTimeComponentsFinishApp::init() const
{
  /* Always reset seconds to zero */
  rtc.time.hours = tmp1;
  rtc.time.minutes = tmp2;
  rtc.time.seconds = 0;
  rtc_set_time_and_date_with_dst_update(&rtc);
}


AppPtr SetTimeComponentApp::update(event_set_t events) const
{
  bool was_updated = false;
  if (events & (BTN1_PRESS|BTN1_REPEAT)) {
    was_updated = true;
    increment_value();
  }

  if (events & (BTN3_PRESS|BTN3_REPEAT)) {
    was_updated = true;
    decrement_value();
  }

  if (was_updated) {
    flash_count = 0;
    did_update();
  }

  if (events & BTN2_TAP) {
    return nextapp_;
  }

  if (events & (BTN2_HOLD|EV_TIMEOUT)) {
    return cancel_app();
  }

  draw();
  return 0;
}


void SetTimeComponentApp::draw() const
{
  display_buffer_write_bcd(4, tmp1);
  display_buffer_write_bcd(2, tmp2);
  display[3] |= sP;
}


AppPtr SetTimeComponentApp::cancel_app() const
{
  return &App_Clock;
}


void SetDateComponentsInitialApp::init() const
{
  tmp1 = rtc.time.months;
  tmp2 = rtc.time.days;
  tmp3 = rtc.time.years;
  App_SetMonth.did_update(); /* get number of days in current month */
}



void SetDateComponentsFinishApp::init() const
{
  rtc.time.months = tmp1;
  rtc.time.days = tmp2;
  rtc.time.years = tmp3;
  rtc_set_date_with_dst_update(&rtc);
}


void SetDateComponentApp::draw() const
{
  display_buffer_write_bcd(5, suppress_leading_zero_left_align(tmp1));
  display_buffer_write_bcd(3, suppress_leading_zero_left_align(tmp2));
  display_buffer_write_bcd(1, tmp3);
  display[2+(tmp2<0x10)] |= sP;
  display[4+(tmp1<0x10)] |= sP;
}


void SetDateComponentApp::did_update() const
{
  days_in_selected_month = days_in_month_for_year(tmp1, tmp3);
  tmp2 = MIN(tmp2, days_in_selected_month);
}


AppPtr SetDateComponentApp::cancel_app() const
{
  return &App_FinishSetDate;
}


AppPtr SetAlarmComponentApp::cancel_app() const
{
  return &App_FinishSetAlarm;
}


void SetAlarmComponentsFinishApp::init() const
{
  rtc.alarm.seconds = 0;
  rtc.alarm.hours = tmp1;
  rtc.alarm.minutes = tmp2;
  rtc_set_alarm_time(&rtc.alarm);
}



void SetAlarmApp::init() const
{
  App::init();
  tmp1 = rtc.alarm.hours;
  tmp2 = rtc.alarm.minutes;
}


AppPtr SetAlarmApp::update(event_set_t events) const
{
  draw();
  if (events & (BTN1_HOLD|BTN2_HOLD|EV_TIMEOUT)) {
    return cancel_app();
  }
  if (events & BTN2_TAP) {
    rtc_set_alarm_enabled(&rtc.alarm, true);
    return &App_SetAlarmHour;
  }
  if (events & (BTN1_TAP|BTN3_TAP)) {
    rtc_set_alarm_enabled(&rtc.alarm, !rtc_alarm_is_enabled(&rtc.alarm));
  }

  return 0;
}


void SetAlarmComponentApp::draw() const
{
  display[5] = c_A;
  display[4] = c_L|sP;
  display_buffer_write_bcd(3, tmp1);
  display_buffer_write_bcd(1, tmp2);
  display[2] |= sP;
  draw_alarm_indicator();
}


AppPtr BatteryLowCheckApp::update(event_set_t events) const
{
  return (rtc_battery_is_low()) ? &App_BatteryLowAlert : nextapp_;
}


void MenuApp::draw() const
{
  if (message_) {
    display_buffer_set_digits(message_);
  }
}


AppPtr MenuApp::update(event_set_t events) const
{
  draw();
  /* Holding any button exits the menu */
  if (events & (BTN1_HOLD|BTN2_HOLD|BTN3_HOLD|EV_TIMEOUT)) {
    return &App_Clock;
  }
  /* Pressing the center button advances into a menu */
  if (events & BTN2_TAP && enterapp_) {
    return enterapp_;
  }
  /* Pressing bottom button goes to the next menu item */
  if (events & BTN3_TAP) {
    return nextapp_;
  }
  /* Pressing top button goes to the previous menu item */
  if (events & BTN1_TAP) {
    return prevapp_;
  }

  return 0;
}


/* Only need to show the message once. */
void BatteryStatusApp::init() const
{
  display_buffer_set_digits((rtc_battery_is_low()) ? battery_low_message : battery_ok_message);
}

AppPtr FirmwareVersionApp::update(event_set_t events) const
{
  /* Holding top button enters bootloader. */
  if (events & BTN1_HOLD) {
    enter_bootloader();
  } else {
    return MenuApp::update(events);
  }
}


void FlagMenuApp::draw() const
{
  MenuApp::draw();
  display[0] = (get_()) ? c_Y : c_N;
}


AppPtr FlagMenuApp::update(event_set_t events) const
{
  /* center button toggles value */
  if (events & BTN2_PRESS) {
    set_(!get_());
    rtc_update_settings(&rtc.settings);
  }
  return MenuApp::update(events);
}


AppPtr ValueMenuApp::update(event_set_t events) const
{
  /* top button increases value */
  if (events & BTN1_TAP) {
    set_(get_()+1);
    rtc_update_settings(&rtc.settings);
    return 0;
  }
  /* bottom button decreases value */
  if (events & BTN3_TAP) {
    set_(get_()-1);
    rtc_update_settings(&rtc.settings);
    return 0;
  }
  return MenuApp::update(events);
}


void SetAutoDSTApp::draw() const
{
  FlagMenuApp::draw();
  /* Show a decimal point if currently in DST. */
  display[1] = (rtc.settings.in_dst) ? sP : 0;
}


void SetDisplayStyleApp::draw() const
{
  uint32_t prev = ticks_since_last_second;
  ticks_since_last_second = 0; /* don't blink colons */
  App_Clock.draw();
  ticks_since_last_second = prev;
}


/**
 * Trapped in a world under leagues of ocean.
 * The clergy arrives with the magic potion.
 */
AppPtr SleepApp::update(event_set_t events) const
{
#if PROGRAMMER_MODE
  if (events & BTN1_HOLD) {
    enter_bootloader();
  }
#endif

  /* Display is blanked. */
  display_off();
  /* Any button press wakes up. */
  return (events & (BTN1_TAP|BTN2_TAP|BTN3_TAP)) ? &App_Startup : 0;
}
