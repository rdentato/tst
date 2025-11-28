//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

// Exercise tst(), tstpassed(), tstfailed(), tstskipped() and case counters.

tstsuite("Result state and counters") {
  tstcase("tst() and result queries") {
    // Passing test
    tst(1 == 1);
    tstcheck(tstpassed(), "Expected last test to pass");
    tstcheck(!tstfailed(), "Did not expect failure");
    tstcheck(!tstskipped(), "Did not expect skip");

    // Failing test
    tst(1 == 2);
    tstcheck(tstfailed(), "Expected last test to fail");
    tstcheck(!tstpassed(), "Did not expect pass");
    tstcheck(!tstskipped(), "Did not expect skip");

    // Skipped test via tstskipif
    tstskipif(1) {
      tstcheck(1, "This check will be reported as SKIP");
    }
    tstcheck(tstskipped(), "Expected last test to be skipped");
    tstcheck(!tstpassed(), "Skipped test is not a pass");
  }

  tstcase("Per-case counters") {
    // Start with known state
    tstcheck(1, "First pass");           // pass = 1, fail = 0, skip = 0
    tstcheck(0, "Intentional failure");  // pass = 1, fail = 1, skip = 0

    tstskipif(1) {
      tstcheck(1, "Will be skipped");    // pass = 1, fail = 1, skip = 1
    }

    tstcheck(tst_case_pass == 1,  "Expected 1 pass, got %d",  tst_case_pass);
    tstcheck(tst_case_fail == 1,  "Expected 1 fail, got %d",  tst_case_fail);
    tstcheck(tst_case_skip == 1,  "Expected 1 skip, got %d",  tst_case_skip);
  }

  // Case with no explicit checks: should run and not crash
  tstcase("Empty body") {
  }
}
