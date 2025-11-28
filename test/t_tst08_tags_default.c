//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

// Verify default behavior of tagged vs untagged test cases
// when no command-line tag filters are provided.

static int ran_untagged = 0;
static int ran_plus_tag = 0;
static int ran_minus_tag = 0;

tstsuite("Tag defaults without filters") {
  // Always runs (no tags)
  tstcase("Untagged case") {
    ran_untagged++;
    tstcheck(1, "Untagged case should always run");
  }

  // Tagged with +SlowTests: should be skipped when no filters
  tstcase("Plus tag case", +SlowTests) {
    ran_plus_tag++;
    tstcheck(1, "+SlowTests case should only run when enabled");
  }

  // Tagged with -SlowTests: also disabled by default
  tstcase("Minus tag case", -SlowTests) {
    ran_minus_tag++;
    tstcheck(1, "-SlowTests case should only run when filters are present");
  }

  // Verify which cases actually executed under default settings
  tstcase("Verify default execution set") {
    tstcheck(ran_untagged == 1,
             "Expected untagged case to run exactly once, got %d",
             ran_untagged);

    tstcheck(ran_plus_tag == 0,
             "Expected +SlowTests case to be skipped by default, ran %d times",
             ran_plus_tag);

    tstcheck(ran_minus_tag == 0,
             "Expected -SlowTests case to be skipped by default, ran %d times",
             ran_minus_tag);
  }
}
