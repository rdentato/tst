//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

// Ensure the "disabled" variants of macros do not execute their bodies
// or evaluate their expressions, while still compiling correctly.

static int side_effects = 0;

tstsuite("Disabled macro variants") {
  tstcase("Disabled case macro") {
    side_effects = 0;

    // Body must never execute
    tst_case("This case must be disabled") {
      side_effects++;
      tstcheck(0, "Disabled case body should not run");
    }

    tstcheck(side_effects == 0,
             "tst_case body executed unexpectedly (%d)", side_effects);
  }

  tstcase("Disabled section macro") {
    side_effects = 0;

    // Section body must never execute
    tst_section("This section must be disabled") {
      side_effects++;
      tstcheck(0, "Disabled section body should not run");
    }

    tstcheck(side_effects == 0,
             "tst_section body executed unexpectedly (%d)", side_effects);
  }

  tstcase("Disabled check/skip/clock macros") {
    int value = 0;

    // None of these should evaluate their arguments or change value
    tst_check((value = 1), "Disabled tst_check should not evaluate");
    tst_expect((value = 2), "Disabled tst_expect should not evaluate");
    tst_assert((value = 3), "Disabled tst_assert should not evaluate");
    tst_note("Disabled tst_note should not run: %d", value);

    tst_skpif(1) {
      value = 4;
    }

    tst_clock("Disabled tst_clock should not run") {
      value = 5;
    }

    tstcheck(value == 0,
             "Disabled macros must not change value (got %d)", value);
  }
}
