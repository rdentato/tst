//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

int f(int n, const char *s) {
  return 1;
}

tstsuite("Data driven tests") {
  tstcase("Use static data") {

    struct {int n; const char *s;} tstdata[] = {
             {123, "pippo"},
             {431, "pluto"},
             { 93, "topolino"}
    };

    tstsection() {
      tstnote("Checking <%d,%s>",tstcurdata.n,tstcurdata.s);
      tstcheck(f(tstcurdata.n , tstcurdata.s));
    }
  }

  tstcase("A static integer array in the range [-10 10]") {
    int tstdata[] = {-1,3,4,5};
    tstsection() {
      tstnote("Checking: %d", tstcurdata);
      tstcheck (-10 <= tstcurdata && tstcurdata <= 10);
    }
  }

  tstcase("A random integer array in the range [-10 10]") {
    srand(time(0));
    int tstdata[4]; // array size must be specified
    for (int k=0; k<4; k++) tstdata[k] = 8-(rand() & 0x0F);
    tstsection() {
      tstnote("Checking: %d", tstcurdata);
      tstcheck (-10 <= tstcurdata && tstcurdata <= 10);
    }
  }
  tstcase("A random integer array in the range [-10 10] (multiple times)") {
    srand(time(0));
    int tstdata[4]; // array size must be specified
    for (int k=0; k<3; k++) {
      for (int k=0; k<4; k++) tstdata[k] = 8-(rand() & 0x0F);
      tstsection() {
        tstnote("Checking: %d", tstcurdata);
        tstcheck (-10 <= tstcurdata && tstcurdata <= 10);
      }
    }
  }

  tstcase("Multiple sections") {
    struct {int n; const char *s;} tstdata[] = {
             {123, "pippo"},
             {431, "pluto"},
             { 93, "topolino"}
    };

    tstsection() {
      tstnote("1st check <%d,%s>",tstcurdata.n,tstcurdata.s);
      tstcheck(f(tstcurdata.n , tstcurdata.s));
    }

    tstsection() {
      tstnote("2nd check <%d,%s>",tstcurdata.n,tstcurdata.s);
      tstcheck(f(tstcurdata.n , tstcurdata.s));
    }
  }
}
