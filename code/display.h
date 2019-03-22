#ifndef DISPLAY_H_
#define DISPLAY_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "bcd.h"
#include <stdbool.h>

/**
 * Number of digits in the display.
 */
#define DISPLAY_NUM_DIGITS      6

/**
 * Number of distinct brightness levels. Must be a power of two.
 */
#define DISPLAY_MAX_BRIGHTNESS  8

/**
 * Minimum usable display brightness.
 */
#define DISPLAY_MIN_BRIGHTNESS  2

/**
 * Initializes and blanks the numitron display.
 */
void display_init(void);

/**
 * Must be called once per main loop iteration.
 */
void display_update(void);

/**
 * Display back-buffer.
 * Changes to bits in this buffer are not reflected on the display until
 * display_buffer_commit() is called.
 */
extern uint16_t display[DISPLAY_NUM_DIGITS];

/**
 * Clears the display buffer.
 */
void display_buffer_clear(void);

/**
 * Packs the display buffer into the 64-bit representation required by the
 * hardware and updates the visible display.
 */
void display_buffer_commit(void);

/**
 * Displays the byte as two hexadecimal characters at positions pos (msd) and
 * pos-1 (lsd).
 */
void display_buffer_write_hex(uint8_t pos, uint8_t value);

/**
 * Displays the pair of BCD digits at positions pos (msd) and pos-1 (lsd).
 * A blank is shown for digits greater than 9.
 */
void display_buffer_write_bcd(uint8_t pos, bcd_t value);

/**
 * Performs a logical AND of the segment pattern with the patterns of the digits
 * indicated by 1 bits in the given digit mask.
 */
void display_buffer_and_digits(uint8_t digit_mask, uint16_t pattern);

void display_buffer_set_digits(const uint16_t *patterns);

void display_set_brightness(uint8_t brightness);

void display_set_digit_font(bool alt_font);

uint16_t pattern_for_bcd_digit(bcd_t digit);

/**
 * Turns off display completely. (disables segment preheating)
 */
void display_off();


enum segment_bits {
  sA = (1<<0),
  sB = (1<<1),
  sC = (1<<2),
  sD = (1<<3),
  sE = (1<<4),
  sF = (1<<5),
  sG = (1<<6),
  sH = (1<<7), /* upper diagonal */
  sI = (1<<8), /* lower diagonal */
  sP = (1<<9)  /* decimal point */
};

/* Segment patterns */
#define c_0 (sA|sB|sC|sD|sE|sF)
#define c_1 (sB|sC|sH)
#define c_2 (sA|sB|sD|sI)
#define c_3 (sA|sG|sH|sI)
#define c_4 (sB|sC|sF|sG)
#define c_5 (sA|sC|sD|sF|sG)
#define c_6 (sC|sD|sE|sG|sH)
#define c_7 (sA|sE|sH)
#define c_8 (sA|sB|sC|sD|sE|sF|sG)
#define c_9 (sA|sB|sF|sG|sI)

#define c_0alt  c_0
#define c_1alt  (sB|sC)
#define c_2alt  (sA|sB|sD|sE|sG)
#define c_3alt  (sA|sB|sC|sD|sG)
#define c_4alt  c_4
#define c_5alt  c_5
#define c_6alt  (sC|sD|sE|sF|sG)
#define c_7alt  (sA|sB|sC)
#define c_8alt  c_8
#define c_9alt  (sA|sB|sC|sF|sG)
#define c_Aalt  (sA|sB|sC|sE|sF|sG)

#define c_A (sB|sC|sE|sG|sH)
#define c_B (sA|sC|sD|sE|sF|sG|sH)
#define c_C (sA|sD|sE|sF)
#define c_D (sA|sB|sE|sF|sI)
#define c_E (sA|sD|sE|sF|sG)
#define c_F (sA|sE|sF|sG)
#define c_G (sA|sC|sE|sF|sI)
#define c_H (sB|sC|sE|sF|sG)
#define c_I (sB|sC)
#define c_J (sB|sC|sD|sE)
#define c_K (sC|sE|sF|sG|sH)
#define c_L (sD|sE|sF)
#define c_N (sB|sC|sE|sF|sH)
#define c_O (sA|sB|sC|sD|sE|sF)
#define c_P (sA|sB|sE|sF|sG)
#define c_Q (sA|sB|sD|sE|sF|sI)
#define c_R (sA|sC|sE|sF|sG|sH)
#define c_S (sA|sC|sD|sF|sG)
#define c_U (sB|sC|sD|sE|sF)
#define c_V (sB|sE|sF|sI)
#define c_Y (sB|sF|sG|sI)
#define c_Z (sA|sD|sE|sH)

#define c_a (sC|sE|sG|sI)
#define c_b (sE|sF|sG|sI)
#define c_c (sD|sE|sG)
#define c_d (sB|sC|sD|sI)
#define c_e (sD|sE|sG|sI)
#define c_f (sE|sG|sH)
#define c_l (sD|sE)
#define c_n (sC|sE|sG)
#define c_o (sC|sD|sE|sG)
#define c_r (sE|sG)
#define c_t (sD|sE|sF|sG)
#define c_v (sE|sI)
#define c_z (sD|sG|sI)

#define c_arrow (sG|sI)
#define c_am    (sB|sG|sH)
#define c_pm    (sE|sG|sI)

/* Compile-time conversion of a digit to a segment pattern */
#define PASTE(x,y) x##y
#define DIGIT_TO_PATTERN(digit) PASTE(c_,digit)

#ifdef __cplusplus
}
#endif
#endif
