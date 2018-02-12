/**
 * Date manipulation functions.
 */
#ifndef DATE_H_
#define DATE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "bcd.h"
#include "pcf2129.h"

bool is_leap_year(bcd_t year);

bcd_t days_in_month_for_year(bcd_t month, bcd_t year);

/* The was_in_dst is used to disambiguate in the case of the 01:00:00 hour */
/* on the day of a fall daylight saving time transition. */
bool is_in_dst(const rtc_time_t *t, bool was_in_dst);

void date_subtract_hour(rtc_time_t *t);
void date_add_hour(rtc_time_t *t);

#ifdef __cplusplus
}
#endif
#endif