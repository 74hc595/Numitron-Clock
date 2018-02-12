#ifndef BEEPER_H_
#define BEEPER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

void beeper_init(void);

void beeper_off(void);
void beeper_on(void);
void set_beeper(bool beep);

#ifdef __cplusplus
}
#endif
#endif
