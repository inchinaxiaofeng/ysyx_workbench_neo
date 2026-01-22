#include <am.h>
#include <nemu.h>
#include <sys/types.h>

void __am_timer_init() {}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  u_int32_t rtc2 = inl(RTC_ADDR + 4);
  u_int32_t rtc1 = inl(RTC_ADDR);

  uptime->us = ((u_int64_t)rtc2 << 32) + (u_int64_t)rtc1;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour = 0;
  rtc->day = 0;
  rtc->month = 0;
  rtc->year = 1900;
}
