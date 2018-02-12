#ifndef BUTTON_H_
#define BUTTON_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

enum button_event {
  BTN_EV_PRESS   = (1<<0), /* sent on initial button down */
  BTN_EV_RELEASE = (1<<1), /* sent on button up */
  BTN_EV_TAP     = (1<<2), /* sent after a short button down/up */
  BTN_EV_HOLD    = (1<<3), /* sent after the button has been down past a duration threshold */
  BTN_EV_REPEAT  = (1<<4)  /* sent repeatedly after the button has been down past a duration threshold */
};
typedef uint8_t button_event_set_t;

struct button {
  uint32_t gpio_pin_mask;
  uint32_t counter;
};

void button_init(struct button *b, uint32_t pin_number);

button_event_set_t button_poll(struct button *b);

#ifdef __cplusplus
}
#endif
#endif