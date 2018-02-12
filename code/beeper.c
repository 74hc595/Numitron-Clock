#include "beeper.h"
#include "LPC8xx.h"

#define BEEP_FREQ_HZ  2000

#define SCT_CONFIG_UNIFY            (1<<0)

#define SCT_CTRL_HALT_L             (1 << 2)
#define SCT_CTRL_CLRCTR_L           (1 << 3)

#define SCT_EV_STATE_STATEMASK0     (1 << 0)
#define SCT_EV_STATE_STATEMASK1     (1 << 1)

#define SCT_EV_CTRL_MATCHSEL(n)     (((n) & 15) << 0)
#define SCT_EV_CTRL_COMBMODE(n)     (((n) & 3) << 12)
#define SCT_EV_CTRL_COMBMODE_OR     SCT_EV_CTRL_COMBMODE(0)
#define SCT_EV_CTRL_COMBMODE_MATCH  SCT_EV_CTRL_COMBMODE(1)
#define SCT_EV_CTRL_COMBMODE_IO     SCT_EV_CTRL_COMBMODE(2)
#define SCT_EV_CTRL_COMBMODE_AND    SCT_EV_CTRL_COMBMODE(3)

/* Output complementary square waves on CTOUT0 and CTOUT1 */
/* for extra volume. */
void beeper_init(void)
{
  /* 32-bit counter */
  LPC_SCT->CONFIG = SCT_CONFIG_UNIFY;
  /* Use match mode */
  LPC_SCT->REGMODE_L = 0;
  /* Set up match and reload registers for beep frequency at 50% duty cycle */
  LPC_SCT->MATCH[0].U = 0;
  LPC_SCT->MATCHREL[0].U = __SYSTEM_CLOCK/BEEP_FREQ_HZ;
  LPC_SCT->MATCHREL[1].U = (__SYSTEM_CLOCK/BEEP_FREQ_HZ)/2;
  /* Send event 0 when value matches match register 0 */
  LPC_SCT->EVENT[0].CTRL = SCT_EV_CTRL_MATCHSEL(0) | SCT_EV_CTRL_COMBMODE_MATCH;
  LPC_SCT->EVENT[0].STATE = SCT_EV_STATE_STATEMASK0;
  /* Event 0 clears counter */
  LPC_SCT->LIMIT_L = (1 << 0);
  /* Event 0 clears output 0 and sets output 1 */
  LPC_SCT->OUT[0].CLR = (1 << 0);
  LPC_SCT->OUT[1].SET = (1 << 0);
  /* Send event 1 when value matches match register 1 */
  LPC_SCT->EVENT[1].CTRL = SCT_EV_CTRL_MATCHSEL(1) | SCT_EV_CTRL_COMBMODE_MATCH;
  LPC_SCT->EVENT[1].STATE = SCT_EV_STATE_STATEMASK0;
  /* Event 1 sets output 0 and clears output 1 */
  LPC_SCT->OUT[0].SET = (1 << 1);
  LPC_SCT->OUT[1].CLR = (1 << 1);
  /* Don't use states */
  LPC_SCT->STATE_L = 0;
}


void beeper_on(void)
{
  /* Un-halt the counter */
  LPC_SCT->CTRL_U &= (~SCT_CTRL_HALT_L);
}


void beeper_off(void)
{
  /* Halt and reset the counter */
  LPC_SCT->CTRL_U |= SCT_CTRL_HALT_L;
  LPC_SCT->CTRL_U |= SCT_CTRL_CLRCTR_L;
  /* Set both outputs low */
  LPC_SCT->OUTPUT = 0;
}


void set_beeper(bool beep)
{
  bool beeping = !(LPC_SCT->CTRL_U & SCT_CTRL_HALT_L);
  if (beeping && !beep) {
    beeper_off();
  } else if (!beeping && beep) {
    beeper_on();
  }
}
