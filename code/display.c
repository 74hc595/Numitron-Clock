#include "display.h"
#include "spi.h"
#include "pins.h"
#include "delay.h"

#include <string.h>

/* STP16CP05 can do 30 MHz */
#define DISPLAY_SERIAL_BIT_RATE 30000000
#define DISPLAY_SPI_DIV         ((__SYSTEM_CLOCK/DISPLAY_SERIAL_BIT_RATE)-1)
/**
 * Defines the duty cycle at which "off" segments are kept "preheated"
 * to improve illumination responsiveness.
 * The preheat duty cycle is determined by
 * (DISPLAY_NUM_PREHEAT_STEPS / DISPLAY_MAX_BRIGHTNESS).
 */
#define DISPLAY_NUM_PREHEAT_STEPS 1

/**
 * Internal state
 */
static uint64_t raw_segment_buffer = 0;
static uint32_t display_tick = 0;

/**
 * User-exposed state
 */
static uint32_t display_brightness = DISPLAY_MAX_BRIGHTNESS;
uint16_t display[DISPLAY_NUM_DIGITS] = {0};

uint8_t display_set_brightness(uint8_t brightness)
{
  if (brightness < DISPLAY_MIN_BRIGHTNESS) {
    display_brightness = DISPLAY_MIN_BRIGHTNESS;
  } else if (brightness > DISPLAY_MAX_BRIGHTNESS) {
    display_brightness = DISPLAY_MAX_BRIGHTNESS;
  } else {
    display_brightness = brightness;
  }
  return brightness;
}


uint8_t display_increase_brightness(void)
{
  if (display_brightness < DISPLAY_MAX_BRIGHTNESS) {
    display_brightness++;
  }
  return display_brightness;
}


uint8_t display_increase_brightness_wrap(void)
{
  if (display_brightness >= DISPLAY_MAX_BRIGHTNESS) {
    return display_set_brightness(DISPLAY_MIN_BRIGHTNESS);
  } else {
    return display_increase_brightness();
  }
}


uint8_t display_decrease_brightness(void)
{
  if (display_brightness > DISPLAY_MIN_BRIGHTNESS) {
    display_brightness--;
  }
  return display_brightness;
}


uint8_t display_decrease_brightness_wrap(void)
{
  if (display_brightness <= DISPLAY_MIN_BRIGHTNESS) {
    return display_set_brightness(DISPLAY_MAX_BRIGHTNESS);
  } else {
    return display_decrease_brightness();
  }
}


void display_off(void)
{
  display_brightness = 0;
}


/**
 * Loads a new segment pattern into the display shift registers, updating the
 * visible display.
 */
static void display_shift_out(uint64_t segments)
{
  LPC_SPI0->DIV = DISPLAY_SPI_DIV;
  spi_send64_no_ss(LPC_SPI0, segments);
  pin_high(PIN_LATCH);
  pin_low(PIN_LATCH);
}


void display_init(void)
{
  display_tick = 0;
  raw_segment_buffer = 0;
  display_shift_out((1<<0)|(1<<5)|(1<<19));
  display_buffer_clear();
}


void display_update(void)
{
  /* To minimize SPI activity and take advantage of the fact that the display */
  /* drivers, you know, actually have memory, only shift out a new segment */
  /* pattern when it changes. */
  /* For DISPLAY_MAX_BRIGHTNESS=8 and DISPLAY_NUM_PREHEAT_STEPS=1, */
  /* brightness=0: 00 -- -- -- -- -- -- -- */
  /* brightness=1: dd 00 -- -- -- -- -- -- */
  /* brightness=2: 11 dd 00 -- -- -- -- -- */
  /* brightness=3: 11 dd -- 00 -- -- -- -- */
  /* brightness=4: 11 dd -- -- 00 -- -- -- */
  /* brightness=5: 11 dd -- -- -- 00 -- -- */
  /* brightness=6: 11 dd -- -- -- -- 00 -- */
  /* brightness=7: 11 dd -- -- -- -- -- 00 */
  /* brightness=8: 11 dd -- -- -- -- -- -- */
  /* (00 = shift out all zeros) */
  /* (11 = shift out all ones) */
  /* (dd = shift out segment pattern) */
  /* (-- = do not shift out anything) */
  /* (TODO: blank display once when brightness is set to 0 and do nothing in this function) */
  uint32_t pwm_step = display_tick & (DISPLAY_MAX_BRIGHTNESS-1);
  if (pwm_step == display_brightness) {
    display_shift_out(0ULL);
  } else if (display_brightness >= DISPLAY_NUM_PREHEAT_STEPS && pwm_step == DISPLAY_NUM_PREHEAT_STEPS) {
    display_shift_out(0); /* seems to fix a glitch that occasionally causes segments in the left 3 digits to be misaligned... */
    display_shift_out(raw_segment_buffer); /* actual display */
  } else if (pwm_step == 0) {
    display_shift_out(0xFFFFFFFFFFFFFFFFUL); /* segment preheat */
  }

  display_tick++;
}


void display_buffer_clear(void)
{
  /* we don't need no stinkin memset */
  display[0]=display[1]=display[2]=display[3]=display[4]=display[5]=0;
}


static uint32_t three_digits_packed(uint16_t *ds)
{
  uint32_t result = 0;
  uint32_t tmp;
  asm(
    ".syntax unified\n"
    "lsrs %[tmp], %[d3], #5\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d3], #6\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d3], #1\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d3], #2\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d3], #8\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d2], #5\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d2], #6\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d2], #1\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d2], #3\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d2], #9\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d2], #4\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d3], #7\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d3], #10\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d3], #3\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d3], #9\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d3], #4\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d2], #2\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d2], #8\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d1], #5\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d1], #6\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d1], #1\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d1], #2\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d1], #8\n"
    "adcs %[result], %[result]\n"
    "lsls %[result], %[result], #2\n"
    "lsrs %[tmp], %[d1], #7\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d1], #10\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d1], #3\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d1], #9\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d1], #4\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d2], #7\n"
    "adcs %[result], %[result]\n"
    "lsrs %[tmp], %[d2], #10\n"
    "adcs %[result], %[result]\n"

    : [result] "=&r" (result), [tmp] "=&r" (tmp)
    : [d3] "r" (ds[2]), [d2] "r" (ds[1]), [d1] "r" (ds[0])
    :
  );

  return result;
}


void display_buffer_commit(void)
{
  raw_segment_buffer = three_digits_packed(display)
    | ((uint64_t)three_digits_packed(display+3) << 32);
}


void display_buffer_write_hex(uint8_t pos, uint8_t value)
{
  display[pos]   = hex_digit_patterns[(value >> 4) & 0xF];
  display[pos-1] = hex_digit_patterns[value & 0xF];
}


void display_buffer_write_bcd(uint8_t pos, bcd_t value)
{
  display[pos]   = bcd_digit_patterns[(value >> 4) & 0xF];
  display[pos-1] = bcd_digit_patterns[value & 0xF];
}


void display_buffer_and_digits(uint8_t digit_mask, uint16_t pattern)
{
  for (int pos = 0; pos < DISPLAY_NUM_DIGITS; pos++, digit_mask >>= 1) {
    if (digit_mask & 1) { display[pos] &= pattern; }
  }
}


void display_buffer_set_digits(const uint16_t *patterns)
{
  /* no memcpy :( */
  for (int i = 0; i < DISPLAY_NUM_DIGITS; i++) {
    display[i] = patterns[i];
  }
}



const uint16_t hex_digit_patterns[16] = {
  c_0, c_1, c_2, c_3, c_4, c_5, c_6, c_7,
  c_8, c_9, c_a, c_b, c_c, c_d, c_e, c_f
};

const uint16_t bcd_digit_patterns[16] = {
  c_0, c_1, c_2, c_3, c_4, c_5, c_6, c_7,
  c_8, c_9, 0,   0,   0,   0,   0,   0
};
