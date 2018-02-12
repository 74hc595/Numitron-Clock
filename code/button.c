#include "button.h"
#include "LPC8xx.h"

#define BUTTON_DEBOUNCE_THRESHOLD     2
#define BUTTON_HOLD_THRESHOLD         40 /* 1 second at a poll rate of 60 Hz */
#define BUTTON_REPEAT_THRESHOLD       41
#define BUTTON_FAST_REPEAT_THRESHOLD  120

/* Repeat periods should be powers of two */
#define BUTTON_REPEAT_PERIOD          8
#define BUTTON_FAST_REPEAT_PERIOD     4

void button_init(struct button *b, uint32_t pin_number)
{
  b->gpio_pin_mask = (1 << pin_number);
  b->counter = 0;
}


button_event_set_t button_events_for_up(struct button *b)
{
  enum button_event ev = 0;
  /* Only treat this as a release if we're past the debounce threshold */
  if (b->counter >= BUTTON_DEBOUNCE_THRESHOLD) {
    ev = BTN_EV_RELEASE;
    /* If this was a short press/release, emit a tap event as well */
    if (b->counter < BUTTON_HOLD_THRESHOLD) {
      ev |= BTN_EV_TAP;
    }
  }
  b->counter = 0;
  return ev;
}


static button_event_set_t button_events_for_down(struct button *b)
{
  b->counter++;
  /* Emit a press event if the button has been held past the debounce threshold */
  if (b->counter == BUTTON_DEBOUNCE_THRESHOLD) {
    return BTN_EV_PRESS;
  }
  /* Emit a hold event if the button has been held down past the hold threshold */
  else if (b->counter == BUTTON_HOLD_THRESHOLD) {
    return BTN_EV_HOLD;
  }
  /* Emit periodic repeat events after the hold threshold is passed */
  else if (b->counter >= BUTTON_FAST_REPEAT_THRESHOLD) {
    return (b-> counter & (BUTTON_FAST_REPEAT_PERIOD-1)) ? 0 : BTN_EV_REPEAT;
  } else if (b->counter >= BUTTON_REPEAT_THRESHOLD) {
    return (b->counter & (BUTTON_REPEAT_PERIOD-1)) ? 0 : BTN_EV_REPEAT;
  }
  return 0;
}


button_event_set_t button_poll(struct button *b)
{
  if (LPC_GPIO_PORT->PIN0 & b->gpio_pin_mask) {
    return button_events_for_up(b);
  } else {
    return button_events_for_down(b);
  }
}
