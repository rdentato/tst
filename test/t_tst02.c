//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

tstrun("Sections")
{
  tstcase("All sections") {
    int a = -1;

    tstsetup("Preparing for 5 starting from %d",a) {
      a = 5;
      tstcheck(a==5);
    }
    tstsection("Changing to 8 stating from %d",a) {
      tstcheck(a==5);
      a = 8;
    }
    tstsection("Changing to 9 stating from %d",a) {
      tstcheck(a==5);
      a = 9;
    }
    tstcleanup("Cleanup with %d",a) {
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
