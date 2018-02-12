#ifndef BCD_H_
#define BCD_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint8_t bcd_t;

bcd_t bcd_increment(bcd_t n);
bcd_t bcd_increment_wrap(bcd_t n, bcd_t minval, bcd_t maxval);

bcd_t bcd_decrement(bcd_t n);
bcd_t bcd_decrement_wrap(bcd_t n, bcd_t minval, bcd_t maxval);

uint8_t bcd_to_byte(bcd_t n);

bcd_t suppress_leading_zero(bcd_t n);
bcd_t suppress_leading_zero_left_align(bcd_t n);

#ifdef __cplusplus
}
#endif
#endif