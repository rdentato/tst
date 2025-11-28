//  SPDX-FileCopyrightText: © 2023 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"

tstsuite("Exit Code Demonstration") {
  tstcase("Passing tests") {
    tstcheck(1 + 1 == 2, "Math works");
    tstcheck(2 * 2 == 4, "Multiplication works");
  }
  
  tstcase("Intentional failure", +demo) {
    // This test will fail to demonstrate --report-error
    tstcheck(1 == 2, "This test fails on purpose");
  }
  
  tstcase("More passing tests") {
    tstcheck(3 + 3 == 6, "Addition works");
  }
}
