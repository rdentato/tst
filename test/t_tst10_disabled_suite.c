//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

// Verify that tst_suite defines a completely disabled test suite:
// the body is compiled but never executed.

// If this suite starts running, it will immediately abort with a failure.

tst_suite("Compile-only disabled suite") {
  tstcase("This case must never run") {
    tstassert(0,
              "Disabled suite body executed unexpectedly (tst_suite regression)");
  }
}
