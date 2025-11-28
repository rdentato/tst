//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"  // Ensure the tst framework is included

tstsuite("Primary Test Suite")
{
    tstcase("Normal testcase") {
      tstcheck(1==1, "Ok");
    }

    tstcase("Equality Checks") {
      tstcheck(1 == 1, "Mismatch: %d != %d", 1, 1);
      tstcheck(1 != 1, "Failed on purpose");
      tstassert(1 != 1, "Aborted on purpose");
      tstcheck(2 == 2, "Mismatch: %d != %d", 2, 2);
    }
    
    tstnote("SHOULDN'T HAVE BEEN PRINTED!!!");
}
