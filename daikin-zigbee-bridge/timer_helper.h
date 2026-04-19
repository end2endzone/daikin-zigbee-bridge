#pragma once

#include <SoftTimers.h>

static inline void timer_disable(SoftTimer & timer) {
  timer.setTimeOutTime(0);
  timer.reset();
}

static inline bool timer_is_enabled(SoftTimer & timer) {
  return (timer.getTimeOutTime() != 0);
}
