//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"


tstsuite("Data driven tests") {
  srand(time(0));
  tstcase("A random integer array in the range [-10 10]") {
    int tstdata[4]; // array size must be specified
    tstouterr("Generated data:") {
       for (int k=0; k<4; k++) {
          tstdata[k] = 8-(rand() & 0x0F);
          tstprintf("[%d] = %d\n",k,tstdata[k]);
       }
    }
    tstsection( "Check 1") {
      tstcheck (-10 <= tstcurdata && tstcurdata <= 10);
    }
    tstsection( "Check 2") {
      tstcheck (-10 <= tstcurdata && tstcurdata <= 10);
    }
  }
}
