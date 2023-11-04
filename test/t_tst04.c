//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"  // Ensure the tst framework is included

tstrun("Primary Test Suite")
{
    tstcase("Equality Checks %d, %d", 1, 1) {
      tstcheck(1 == 1, "Mismatch: %d != %d", 1, 1);
      tstassert(1 != 1, "Failed on purpose");
      tstcheck(2 == 2, "Mismatch: %d != %d", 2, 2);
    }
    
    tstnote("SHOULDN'T HAVE BEEN PRINTED!!!");
}