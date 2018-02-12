#include "rtc.h"
#include "spi.h"
#include "delay.h"
#include "pins.h"
#include "date.h"

/* PCF2129 can do 6.5 MHz. 6 MHz evenly divides the system clock */
#define RTC_SERIAL_BIT_RATE       6000000
#define RTC_SPI_DIV               ((__SYSTEM_CLOCK/RTC_SERIAL_BIT_RATE)-1)

#define RTC_INIT_POLL_INTERVAL_MS 100
#define RTC_INIT_TIMEOUT_MS       3000
#define RTC_OTP_REFRESH_DELAY_MS  100

#define RTC_PIN_INT               0

/* Default values for registers CLKOUT_ctl through Timestp_ctl (0x0F-0x12) */
static const uint8_t default_feature_regs[4] = {
  0b00000111, /* CLKOUT disabled */
  0b00100011, /* Watchdog disabled, second interrupts generate /INT pulses */
  0,          /* Watchdog timer value (unused) */
  0b01000000, /* Timestamp disabled */
};

/* Default values for registers Control_1 through Control_3 */
static const uint8_t default_control_regs[3] = {
  0b00001001, /* Enable second interrupts */
  0b00000000, /* Clear all flags, disable timestamp and alarm interrupts */
  0b00000000, /* Standard switchover mode, battery low detection on, disable battery interrputs */
};

static const rtc_state_t default_state = {
  .time = {
    .seconds  = 0x00,
    .OSF      = 0,
    .minutes  = 0x00,
    .hours    = 0x00,
    .days     = 0x01,
    .weekdays = 0x00,
    .months   = 0x01,
    .years    = 0x00
  },
  .alarm = {
    .seconds = 0x00,
    .nAE_S   = 1,
    .minutes = 0x00,
    .nAE_M   = 1,
    .hours   = 0x00,
    .nAE_H   = 1,
  },
  .settings = {
    .in_dst        = 0,
    .auto_dst      = 1,
    .sleep         = 0,
    .date_cycling  = 1,
    .display_style = 0,
    .must_be_1     = 1,
    .brightness    = 7,
    .must_be_1_2   = 1
  }
};



uint8_t rtc_read_register(uint8_t reg)
{
  LPC_SPI0->DIV = RTC_SPI_DIV;
  return spi_read_reg8(LPC_SPI0, RTC_READ_REG(reg));
}


void rtc_write_register(uint8_t reg, uint8_t value)
{
  LPC_SPI0->DIV = RTC_SPI_DIV;
  return spi_write_reg8(LPC_SPI0, RTC_WRITE_REG(reg), value);
}


void rtc_read_registers(uint8_t reg, uint8_t nbytes, uint8_t *data)
{
  LPC_SPI0->DIV = RTC_SPI_DIV;
  spi_read_bytes(LPC_SPI0, RTC_READ_REG(reg), nbytes, data);
}


void rtc_write_registers(uint8_t reg, uint8_t nbytes, const uint8_t *data)
{
  LPC_SPI0->DIV = RTC_SPI_DIV;
  spi_write_bytes(LPC_SPI0, RTC_WRITE_REG(reg), nbytes, data);
}


void rtc_get_state(rtc_state_t *st)
{
  rtc_read_registers(RTC_FIRST_STATE_REGISTER, sizeof(rtc_state_t), st->b);
}


static void rtc_set_state(const rtc_state_t *st)
{
  rtc_write_registers(RTC_FIRST_STATE_REGISTER, sizeof(rtc_state_t), st->b);
}


void rtc_set_time_and_date(const rtc_time_t *t)
{
  rtc_write_registers(RTC_FIRST_TIME_REGISTER, sizeof(rtc_time_t), t->b);
}


void rtc_set_date_only(const rtc_time_t *t)
{
  rtc_write_registers(RTC_FIRST_DATE_REGISTER, RTC_NUM_DATE_REGISTERS, t->b+RTC_NUM_TIME_REGISTERS);
}


void rtc_set_date_with_dst_update(rtc_state_t *st)
{
  const rtc_time_t *t = &(st->time);
  rtc_set_date_only(t);
  rtc_update_dst_flag(st); 
}


void rtc_set_time_and_date_with_dst_update(rtc_state_t *st)
{
  const rtc_time_t *t = &(st->time);
  rtc_set_time_and_date(t);
  rtc_update_dst_flag(st);
}


void rtc_set_alarm_time(const rtc_alarm_time_t *t)
{
  rtc_write_registers(RTC_FIRST_ALARM_REGISTER, sizeof(rtc_alarm_time_t), t->b);
}


void rtc_update_settings(const rtc_settings_t *t)
{
  rtc_write_registers(RTC_FIRST_SETTINGS_REGISTER, sizeof(rtc_settings_t), t->b);
}


int rtc_init(void)
{
  /* Oscillator startup can take between 0.2 to 2 seconds (section 8.7.1). */
  /* During this period the chip won't respond to commands. */
  /* Periodically poll register zero until we get a value */
  /* (0xFF is never a valid value for Control_1 because bit 6 is reserved and */
  /* should always read as zero.) */
  int elapsed = 0;
  delay_ms(RTC_INIT_POLL_INTERVAL_MS*2);
  while ((rtc_read_register(RTC_CONTROL_1) & 0b01001000) != 0b00001000) {
    delay_ms(RTC_INIT_POLL_INTERVAL_MS);
    elapsed += RTC_INIT_POLL_INTERVAL_MS;
    if (elapsed >= RTC_INIT_TIMEOUT_MS) {
      return -1;
    }
  }

  /* Initialize CLKOUT, watchdog and timestamp control registers */
  rtc_write_registers(RTC_CLKOUT_CTL, sizeof(default_feature_regs), default_feature_regs);
  /* Perform an OTP refresh (section 8.3.2) */
  rtc_write_register(RTC_CLKOUT_CTL, 0b00100111);
  delay_ms(RTC_OTP_REFRESH_DELAY_MS);

  /* If the oscillator has stopped, reset the time */
  if (rtc_read_register(RTC_SECONDS) & 0b10000000) {
    rtc_set_state(&default_state);
  }

  /* Configure interrupts */
	LPC_SYSCON->PINTSEL[RTC_PIN_INT] = PIN_nRTC_INT;
  LPC_PIN_INT->SIENF = (1 << RTC_PIN_INT);

  /* Initialize control registers */
  rtc_write_registers(RTC_CONTROL_1, sizeof(default_control_regs), default_control_regs);

  return 0;
}


rtc_event_set_t rtc_poll()
{
  rtc_event_set_t ret = 0;
  if (LPC_PIN_INT->FALL & (1 << RTC_PIN_INT)) {
    ret |= RTC_EV_SECOND;
    LPC_PIN_INT->FALL = (1 << RTC_PIN_INT);
    uint8_t c2 = rtc_read_register(RTC_CONTROL_2);
    if (c2 & (1<<4)) {
      ret |= RTC_EV_ALARM;
    }
  }
  return ret;
}


bool rtc_alarm_is_enabled(const rtc_alarm_time_t *t)
{
  return ((t->nAE_S == 0) || (t->nAE_M == 0) || (t->nAE_H == 0));
}


void rtc_set_alarm_enabled(rtc_alarm_time_t *t, int enable)
{
  /* alarm enable flags are active low; 1=off, 0=on */
  int new_value = !enable;
  t->nAE_S  = new_value;
  t->nAE_M  = new_value;
  t->nAE_H  = new_value;
  rtc_set_alarm_time(t);
}


void rtc_clear_alarm(void)
{
  /* clear alarm flag */
  uint8_t c2 = rtc_read_register(RTC_CONTROL_2);
  c2 &= 0b11101111;
  rtc_write_register(RTC_CONTROL_2, c2);
}


/* Return value of BF flag */
bool rtc_battery_is_low(void)
{
  return rtc_read_register(RTC_CONTROL_3) & (1<<2);
}


void rtc_update_dst_flag(rtc_state_t *st)
{
  bool new_dst = (st->settings.auto_dst) ? is_in_dst(&st->time, st->settings.in_dst) : 0;
  if (new_dst != st->settings.in_dst) {
    st->settings.in_dst = new_dst;
    rtc_update_settings(&st->settings);
  }
}


void rtc_dst_transition_check(rtc_state_t *st)
{
  if (!st->settings.auto_dst) {
    return;
  }

  rtc_time_t *t = &(st->time);
  bool was_in_dst = st->settings.in_dst;
  rtc_update_dst_flag(st);
  if (was_in_dst && !st->settings.in_dst) {
    /* Fall transition: roll back one hour */
    date_subtract_hour(t);
    rtc_set_time_and_date(t);
  } else if (!was_in_dst && st->settings.in_dst) {
    /* Spring transition: jump ahead one hour */
    date_add_hour(t);
    rtc_set_time_and_date(t);    
  }
}


rtc_state_t rtc = {0};
