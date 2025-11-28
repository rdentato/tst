//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

tstsuite("Sections") {
  tstcase("Keeping 5 starting from -1") {
    int a = -1;

    tstnote("Performing setup with a = %d",a);
    tstcheck(a==-1);
    a = 5;
    tstcheck(a==5);

    tstsection("Changing to 8 stating from a = %d",a) {
      tstcheck(a==5);
      a = 8;
      tstcheck(a==8);
    }
    tstsection("Changing to 9 stating from a = %d",a) {
      tstcheck(a==5);
      a = 9;
      tstcheck(a==9);
    }

    tstnote("Cleanup from a = %d",a);
  }

  tstcase("Empty test") {
    int a = -1;
    a = 5;
    tstcheck(a==5);
    a = 0;
    tstcheck(a==0);
  }

  tstcase("Sections with value changes") {
    int a = 5;
    tstcheck(a==5);

    tstsection("Set to 10") {
      a=10;
      tstcheck(a==10);
    }

    tstsection("Set to 100 then 200") {
      a = 100;
      tstcheck(a==100);
      a = 200;
    }

    tstsection("Set to 100 then 400") {
      a = 100;
      tstcheck(a==100);
      a = 400;
    }

    tstcheck(a==400,"Expected 400, got %d", a);
  }
}
