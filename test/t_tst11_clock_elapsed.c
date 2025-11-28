//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

// Additional coverage for tstclock() and tstelapsed().

#include <time.h>

static void busy_loop(int n)
{
  volatile int x = 0;
  for (int i = 0; i < n; i++) {
    x += i;
  }
}

tstsuite("Clock and elapsed behavior") {
  tstcase("Basic elapsed non-negative") {
    tstclock("Short busy loop") {
      busy_loop(1000);
    }

    clock_t first = tstelapsed();
    tstcheck(first >= 0, "tstelapsed should be non-negative (got %ld)",
             (long)first);

    tstclock("Another busy loop") {
      busy_loop(2000);
    }

    clock_t second = tstelapsed();
    tstcheck(second >= 0, "tstelapsed should be non-negative (got %ld)",
             (long)second);
  }

  tstcase("Elapsed usable in further checks") {
    tstclock("Do almost nothing") {
      // Intentional no-op body
    }

    clock_t t = tstelapsed();
    tstcheck(t >= 0, "Elapsed ticks should be >= 0 (got %ld)", (long)t);

    // Use elapsed value inside another tstcheck expression
    tstcheck(("Checking elapsed" && tstelapsed() >= 0),
             "tstelapsed macro usable in expressions");
  }
}
