//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

tstrun("Sections")
{
  tstcase("All sections") {
    int a = -1;

    tstsetup() {
      a = 5;
      tstcheck(a==5);
    }
    tstsection() {
      tstcheck(a==5);
      a = 8;
    }
    tstsection() {
      tstcheck(a==5);
      a = 9;
    }
    tstcleanup() {
      a = 0;
    }

    tstcheck(a==0);
  }

  tstcase("no sections") {
    int a = -1;

    tstsetup() {
      a = 5;
      tstcheck(a==5);
    }
    tstcleanup() {
      a = 0;
    }

    tstcheck(a==0);
  }
}
