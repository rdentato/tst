//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"  // Ensure the tst framework is included

tstsuite("Primary Test Suite")
{
    tstcase("Equality Checks %d, %d", 1, 1) {
      tstcheck(1 == 1, "Mismatch: %d != %d", 1, 1);
      tstcheck(1 != 1, "Failed on purpose");
      tstcheck(1 != 1, "Failed on %d purpose",5);
      tstcheck(1 != 1);
    }
    
    tstcase("Time Complexity Analysis") {
      tstclock("Check counting time") {
        volatile int b = 1;
        // Code to analyze...
        for (int a = 1; a < 100 ; a++) b = a + b;
      }
    }
    
    tstcase("Grouped Checks: Edge Cases") {
      tstskipif(1 == 2) {  // Next tests Will be skipped!
        tstcheck(0 < 1, "0 should be less than 1");
        tstassert(1 >= 1, "1 should be equal to 1");
      } 

      tstskipif(1 != 2) {  // Next tests will be executed!
        tstcheck(0 < 1, "0 should be less than 1");
        tstassert(1 >= 1, "1 should be equal to 1");
      }
    }
    
    tstnote("Testing Complete. Review for any FAIL flags. (One is expected)");
}
