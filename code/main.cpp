#include "syscon.h"
#include "pins.h"
#include "delay.h"
#include "spi.h"
#include "beeper.h"
#include "display.h"
#include "button.h"
#include "event.h"
#include "rtc.h"
#include "app.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAIN_TICK_HZ    480
#define INPUT_POLL_HZ   60
#define INPUT_TICK_MASK ((MAIN_TICK_HZ/INPUT_POLL_HZ)-1)

static struct button b1;
static struct button b2;
static struct button b3;
static uint32_t main_loop_tick = 0;
static event_set_t events = 0;
static const App *current_app = &App_Startup; /* first app */
static const App *next_app = 0;


uint32_t flash_count = 0;
uint32_t timeout = 0;
uint32_t ticks_since_last_second = 0;

void SysTick_Handler(void)
{
  display_update();
  display_set_digit_font(rtc.settings.alt_font);

  /* Read button and RTC events */
  if ((main_loop_tick & INPUT_TICK_MASK) == 0) {
    events = (button_poll(&b1) << BTN1_EV_SHIFT) |
             (button_poll(&b2) << BTN2_EV_SHIFT) |
             (button_poll(&b3) << BTN3_EV_SHIFT) |
             (rtc_poll()       << RTC_EV_SHIFT);

    if (events & RTC_SECOND) {
      rtc_get_state(&rtc);
      /* At 02:00:00, do daylight saving time check */
      if (rtc.time.seconds == 0x00 && rtc.time.minutes == 0x00 && rtc.time.hours == 0x02) {
        rtc_dst_transition_check(&rtc);
      }
      ticks_since_last_second = 0;
    } else {
      ticks_since_last_second++;
    }

    /* Button events restart the app timeout if there is one */
    if (events & (BTN1_ANY|BTN2_ANY|BTN3_ANY)) {
      timeout = current_app->timeout();
    }
    if (timeout > 0) {
      timeout--;
      if (timeout == 0) {
        events |= EV_TIMEOUT;
      }
    }

    /* Add flash events */
    events |= (((flash_count >> 3) & 0b11) << 28);

    /* Run current app */
    next_app = current_app->update(events);

    /* Override next app if alarm is going off */
    if (events & RTC_ALARM) {
      next_app = &App_AlarmGoingOff;
    }

    /* Take care of flashing */
    if (events & FLASH) {
      /* If bit 7 in the flash mask is set, preserve decimal points. */
      uint8_t flashmask = current_app->flash_mask();
      display_buffer_and_digits(flashmask, (flashmask & 0x80) ? sP : 0);
    }

    /* Update display */
    display_buffer_commit();

    /* Transition to next app if instructed */
    if (next_app && (next_app != current_app)) {
      current_app = next_app;
      current_app->init();
      current_app->update(EV_INITIAL);
      timeout = current_app->timeout();
    }

    flash_count++;
  }

  main_loop_tick++;
}


int main(void)
{
  syscon_enable_clocks(CLK_GPIO|CLK_SWM|CLK_MRT|CLK_SPI0|CLK_SCT);
  syscon_reset_peripherals(RST_GPIO|RST_MRT|RST_SPI0|RST_SCT);
  pins_init();
  beeper_init();
  spi_init(LPC_SPI0);
  display_init();
  button_init(&b1, PIN_SW1);
  button_init(&b2, PIN_SW2);
  button_init(&b3, PIN_SW3);
  rtc_init();
  rtc_get_state(&rtc);
  rtc_dst_transition_check(&rtc);
  SysTick_Config(__SYSTEM_CLOCK/MAIN_TICK_HZ);
  while (1) { __WFI(); }
}

#ifdef __cplusplus
}
#endif
