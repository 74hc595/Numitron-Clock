#ifndef APP_H_
#define APP_H_

#include "event.h"
#include "rtc.h"

#define APP_IDLE_TIMEOUT  (60*5)

using AppPtr = const struct App *;

struct App {
  virtual void init() const;
  virtual AppPtr update(event_set_t events) const { return 0; }
  virtual uint32_t timeout() const { return 0; }
  virtual uint8_t flash_mask() const { return 0; }
};

struct ForwardingApp : public App {
  constexpr ForwardingApp(AppPtr nextapp) : nextapp_(nextapp) {}
  virtual void init() const override {}
  virtual AppPtr update(event_set_t events) const override { return nextapp_; }
  const AppPtr nextapp_;
};

struct StartupApp : public ForwardingApp {
  constexpr StartupApp(AppPtr nextapp) : ForwardingApp(nextapp) {}
  virtual void init() const override;
};

struct MainApp : public App {
  constexpr MainApp(AppPtr nextapp) : nextapp_(nextapp) {}
  virtual AppPtr update(event_set_t events) const override;
  virtual void draw() const = 0;
  const AppPtr nextapp_;
};

struct ClockApp : public MainApp {
  constexpr ClockApp(AppPtr nextapp) : MainApp(nextapp) {}
  virtual void draw() const override;
  virtual AppPtr update(event_set_t events) const override;
};

struct AlarmGoingOffApp : public ClockApp {
  constexpr AlarmGoingOffApp(AppPtr nextapp) : ClockApp(nextapp) {}
  virtual void init() const override;
  virtual AppPtr update(event_set_t events) const override;
};

struct DateApp : public MainApp {
  constexpr DateApp(AppPtr nextapp) : MainApp(nextapp) {}
  virtual void draw() const override;
  virtual uint32_t timeout() const { return APP_IDLE_TIMEOUT; }
};

struct BatteryLowAlertApp : public DateApp {
  constexpr BatteryLowAlertApp(AppPtr nextapp) : DateApp(nextapp) {}
  virtual void draw() const override;
};

struct SetTimeComponentsInitialApp : public ForwardingApp {
  constexpr SetTimeComponentsInitialApp(AppPtr nextapp) : ForwardingApp(nextapp) {}
  virtual void init() const override;
};

struct SetTimeComponentApp : public App {
  constexpr SetTimeComponentApp(bcd_t minval, bcd_t maxval, uint8_t flashmask, bcd_t *valptr, AppPtr nextapp)
    : minval_(minval), maxval_(maxval), flashmask_(flashmask), valptr_(valptr), nextapp_(nextapp) {}
  virtual AppPtr update(event_set_t events) const override;
  virtual void increment_value() const { *valptr_ = bcd_increment_wrap(*valptr_, minval_, max_value()); }
  virtual void decrement_value() const { *valptr_ = bcd_decrement_wrap(*valptr_, minval_, max_value()); }
  virtual const bcd_t max_value() const { return maxval_; }
  virtual void did_update() const {};
  virtual void draw() const;
  virtual uint32_t timeout() const { return APP_IDLE_TIMEOUT; }
  virtual uint8_t flash_mask() const override { return flashmask_; }
  virtual AppPtr cancel_app() const;
  const bcd_t minval_;
  const bcd_t maxval_;
  const uint8_t flashmask_;
  bcd_t *valptr_;
  const AppPtr nextapp_;
};

struct SetTimeComponentsFinishApp : public ForwardingApp {
  constexpr SetTimeComponentsFinishApp(AppPtr nextapp) : ForwardingApp(nextapp) {}
  virtual void init() const override;
};

struct SetDateComponentsInitialApp : public ForwardingApp {
  constexpr SetDateComponentsInitialApp(AppPtr nextapp) : ForwardingApp(nextapp) {}
  virtual void init() const override;
  virtual uint8_t flash_mask() const override { return 0b10110000; }
};

struct SetDateComponentApp : public SetTimeComponentApp {
  constexpr SetDateComponentApp(bcd_t minval, bcd_t maxval, uint8_t flashmask, bcd_t *valptr, AppPtr nextapp)
    : SetTimeComponentApp(minval, maxval, flashmask, valptr, nextapp) {}
  virtual void draw() const override;
  virtual void did_update() const override;
  virtual AppPtr cancel_app() const override;
};

struct SetDaysInMonthApp : public SetDateComponentApp {
  constexpr SetDaysInMonthApp(bcd_t minval, bcd_t *maxvalptr, uint8_t flashmask, bcd_t *valptr, AppPtr nextapp)
    : SetDateComponentApp(minval, 0, flashmask, valptr, nextapp), maxvalptr_(maxvalptr) {}
  virtual const bcd_t max_value() const { return *maxvalptr_; }
  bcd_t *maxvalptr_;
};

struct SetDateComponentsFinishApp : public ForwardingApp {
  constexpr SetDateComponentsFinishApp(AppPtr nextapp) : ForwardingApp(nextapp) {}
  virtual void init() const override;
};

struct SetAlarmComponentApp : public SetTimeComponentApp {
  constexpr SetAlarmComponentApp(bcd_t minval, bcd_t maxval, uint8_t flashmask, bcd_t *valptr, AppPtr nextapp)
    : SetTimeComponentApp(minval, maxval, flashmask, valptr, nextapp) {}
  virtual void draw() const override;
  virtual AppPtr cancel_app() const override;
};

struct SetAlarmApp : public SetAlarmComponentApp {
  constexpr SetAlarmApp(AppPtr nextapp)
    : SetAlarmComponentApp(0, 0, 0b110000, nullptr, nextapp) {}
  virtual void init() const override;
  virtual AppPtr update(event_set_t events) const override;
  virtual uint8_t flash_mask() const override { return 0b110000; }
};

struct SetAlarmComponentsFinishApp : public ForwardingApp {
  constexpr SetAlarmComponentsFinishApp(AppPtr nextapp) : ForwardingApp(nextapp) {}
  virtual void init() const override;
};

struct BatteryLowCheckApp : public ForwardingApp {
  constexpr BatteryLowCheckApp(AppPtr nextapp) : ForwardingApp(nextapp) {}
  virtual void init() const override {};
  virtual AppPtr update(event_set_t events) const override;
};

struct MenuApp : public ForwardingApp {
  constexpr MenuApp(AppPtr nextapp, AppPtr prevapp, AppPtr enterapp, const uint16_t *message) : ForwardingApp(nextapp),
    prevapp_(prevapp), enterapp_(enterapp), message_(message) {}
  virtual void init() const override { App::init(); }
  virtual void draw() const;
  virtual AppPtr update(event_set_t events) const override;
  virtual uint8_t flash_mask() const override { return 0b000001; }
  virtual uint32_t timeout() const override { return APP_IDLE_TIMEOUT; }
  const AppPtr prevapp_;
  const AppPtr enterapp_;
  const uint16_t *message_;
};

struct BatteryStatusApp : public MenuApp {
  constexpr BatteryStatusApp(AppPtr nextapp, AppPtr prevapp) :
    MenuApp(nextapp, prevapp, nullptr, nullptr) {}
  virtual void draw() const override {};
  virtual void init() const override;
  virtual uint8_t flash_mask() const override { return 0; }
};

struct FirmwareVersionApp : public MenuApp {
  constexpr FirmwareVersionApp(AppPtr nextapp, AppPtr prevapp, const uint16_t *message) :
    MenuApp(nextapp, prevapp, nullptr, message) {}
  virtual uint8_t flash_mask() const override { return 0; }
  virtual AppPtr update(event_set_t events) const override;
};

struct FlagMenuApp : public MenuApp {
  using FlagGetter = bool (*)();
  using FlagSetter = void (*)(bool);
  constexpr FlagMenuApp(AppPtr nextapp, AppPtr prevapp, const uint16_t *message,
    const FlagGetter get, const FlagSetter set) :
    MenuApp(nextapp, prevapp, nullptr, message), get_(get), set_(set) {}
  virtual void draw() const;
  virtual AppPtr update(event_set_t events) const override;
  const FlagGetter get_;
  const FlagSetter set_;
};

struct ValueMenuApp : public MenuApp {
  using ValueGetter = unsigned (*)();
  using ValueSetter = void (*)(unsigned);
  constexpr ValueMenuApp(AppPtr enterapp, const uint16_t *message,
    const ValueGetter get, const ValueSetter set) :
    MenuApp(nullptr, nullptr, enterapp, message), get_(get), set_(set) {}
  virtual AppPtr update(event_set_t events) const override;
  const ValueGetter get_;
  const ValueSetter set_;
};

struct SetAutoDSTApp : public FlagMenuApp {
  constexpr SetAutoDSTApp(AppPtr nextapp, AppPtr prevapp, const uint16_t *message,
    const FlagGetter get, const FlagSetter set) :
      FlagMenuApp(nextapp, prevapp, message, get, set) {}
  virtual void draw() const;
};

struct SetDisplayStyleApp : public ValueMenuApp {
  constexpr SetDisplayStyleApp(AppPtr enterapp, const ValueGetter get, const ValueSetter set) :
    ValueMenuApp(enterapp, nullptr, get, set) {}
  virtual void draw() const override;
  virtual uint8_t flash_mask() const override { return 0; }
};

struct SleepApp : public App {
  constexpr SleepApp() : App() {}
  virtual AppPtr update(event_set_t events) const override;
};

extern const StartupApp App_Startup;
extern const ClockApp App_Clock;
extern const DateApp App_Date;
extern const AlarmGoingOffApp App_AlarmGoingOff;

extern const SetTimeComponentsInitialApp App_BeginSetTime;
extern const SetTimeComponentApp App_SetHour;
extern const SetTimeComponentApp App_SetMinute;
extern const SetTimeComponentsFinishApp App_FinishSetTime;

extern const SetDateComponentsInitialApp App_BeginSetDate;
extern const SetDateComponentApp App_SetMonth;
extern const SetDaysInMonthApp App_SetDay;
extern const SetDateComponentApp App_SetYear;
extern const SetDateComponentsFinishApp App_FinishSetDate;

extern const SetAlarmApp App_SetAlarm;
extern const SetAlarmComponentApp App_SetAlarmHour;
extern const SetAlarmComponentApp App_SetAlarmMinute;
extern const SetAlarmComponentsFinishApp App_FinishSetAlarm;

extern const BatteryLowCheckApp App_DateCycleStart;
extern const BatteryLowAlertApp App_BatteryLowAlert;

extern const FlagMenuApp App_SetSleepEnabled;
extern const MenuApp App_SetDatePrompt;
extern const SetAutoDSTApp App_SetAutoDST;
extern const MenuApp App_SetDisplayStylePrompt;
extern const FlagMenuApp App_SetDateCycling;
extern const BatteryStatusApp App_ShowBatteryStatus;
extern const FirmwareVersionApp App_ShowFirmwareVersion;
extern const MenuApp App_MenuDone;

extern const SetDisplayStyleApp App_SetDisplayStyle;

extern const SleepApp App_Sleep;

#endif
