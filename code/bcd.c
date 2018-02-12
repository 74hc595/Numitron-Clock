#include "bcd.h"

bcd_t bcd_increment(bcd_t n)
{
  return n + (((n & 0xF) == 9) ? 7 : 1);
}


bcd_t bcd_increment_wrap(bcd_t n, bcd_t minval, bcd_t maxval)
{
  return (n == maxval) ? minval : bcd_increment(n);
}


bcd_t bcd_decrement(bcd_t n)
{
  return n - (((n & 0xF) == 0) ? 7 : 1);
}


bcd_t bcd_decrement_wrap(bcd_t n, bcd_t minval, bcd_t maxval)
{
  return (n == minval) ? maxval : bcd_decrement(n);
}


uint8_t bcd_to_byte(bcd_t n)
{
  return 10*(n >> 4) + (n & 0xF);
}


bcd_t suppress_leading_zero(bcd_t n)
{
  return (n & 0xF0) ? n : n|0xF0;
}


bcd_t suppress_leading_zero_left_align(bcd_t n)
{
  return (n >= 0x10) ? n : ((n << 4) | 0xF);
}
