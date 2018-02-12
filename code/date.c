#include "date.h"
#include "dst_dates.h"

bool is_leap_year(bcd_t year)
{
  year = bcd_to_byte(year);
  return (year & 3) == 0; /* leap year if divisble by 4 */
}


static const bcd_t days_in_month_table[] = {
  0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30
};

bcd_t days_in_month_for_year(bcd_t month, bcd_t year)
{
  /* let's just get this out of the way */
  if (is_leap_year(year) && month == 2) { return 0x29; }
  /* hasty bcd to binary conversion */
  if (month & 0x10) { month -= 6; }
  if (month > 11) { return 0x31; }
  return days_in_month_table[month-1];
}


static uint32_t to_month_date_hour(const rtc_time_t *t)
{
  return (t->months << 16) | (t->days << 8) | t->hours;
}


/* The was_in_dst is used to disambiguate in the case of the 01:00:00 hour */
/* on the day of a fall daylight saving time transition. */
bool is_in_dst(const rtc_time_t *t, bool was_in_dst)
{
  uint8_t bin_year = bcd_to_byte(t->years);
  if (bin_year >= 100) { return false; } /* shouldn't happen... */
  struct dst_date_pair indexes = dst_dates_for_year[bin_year];

  uint32_t now = to_month_date_hour(t);
  /* need to check against spring or fall? */
  if (t->months <= 4) {
    /* convert month/day/hour to integer for comparison */
    uint32_t other = 0x000002 | (spring_dst_dates[indexes.spring]<<8);
    return now >= other;
  } else {
    /* Need to handle one exception. If the hour is 1 on the day of the fall */
    /* DST transition, it is ambiguous whether or not you are in DST. */
    /* For that, you need to know if you're previously in DST. */
    /* If so, the 01:00 hour should be considered DST. */
    /* If not, the 01:00 hour should be considered standard time. */
    uint32_t ambiguous_hour = 0x000001 | (fall_dst_dates[indexes.fall]<<8);
    if (now == ambiguous_hour) {
      return was_in_dst;
    } else {
      uint32_t other = ambiguous_hour + 1; /* 02:00:00 */
      return now < other;
    }
  }
}


void date_add_hour(rtc_time_t *t)
{
  /* Straightforward. Increment the smallest component. If it wraps around, */
  /* increment the next larger component, etc. */
  if ((t->hours = bcd_increment_wrap(t->hours, 0x00, 0x23)) != 0x00) {
    return;
  }

  bcd_t max_date = days_in_month_for_year(t->months, t->years);
  if ((t->days = bcd_increment_wrap(t->days, 0x01, max_date)) != 0x01) {
    return;
  }

  if ((t->months = bcd_increment_wrap(t->months, 0x01, 0x12)) != 0x01) {
    return;
  }
  t->years = bcd_increment_wrap(t->years, 0x00, 0x99);
}


void date_subtract_hour(rtc_time_t *t)
{
  if ((t->hours = bcd_decrement_wrap(t->hours, 0x00, 0x23)) != 0x23) {
    return;
  }

  /* Things are more complex if we need to roll back to the previous month */
  if (t->days == 0x01) {
    /* First, roll back one month */
    t->months = bcd_decrement_wrap(t->months, 0x01, 0x12);
    /* Roll back one year if we wrapped around */
    if (t->months == 0x12) {
      t->years = bcd_decrement_wrap(t->years, 0x00, 0x99);
    }
    /* Now set the date to the maximum value for the new month/year */
    t->days = days_in_month_for_year(t->months, t->years);
  }
  /* If not, just roll back one day. */
  else {
    t->days = bcd_decrement(t->days);
  }
}
