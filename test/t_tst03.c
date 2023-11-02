//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

int f(int n, const char *s) {
  return 1;
}

tstrun("Data driven tests") {
  tstcase("Use static data") {

    struct {int n; const char *s;} tstdata[] = {
             {123, "pippo"},
             {431, "pluto"},
             { 93, "topolino"}
    };

    tstdatafor("Verify edge cases") {
      tstnote("Checking <%d,%s>",tstcurdata.n,tstcurdata.s);
      tstcheck(f(tstcurdata.n , tstcurdata.s));
    }
  }

  tstcase("A static integer array") {
    int tstdata[] = {-1,3,4,5};
    tstdatafor("Integers in the range [-10 10]") {
      tstnote("Checking: %d", tstcurdata);
      tstcheck (-10 <= tstcurdata && tstcurdata <= 10);
    }
  }

  tstcase("A random integer array") {
    srand(time(0));
    int tstdata[4]; // array size must be specified
    for (int k=0; k<4; k++) tstdata[k] = 8-(rand() & 0x0F);
    tstdatafor("Integers in the range [-10 10]") {
      tstnote("Checking: %d", tstcurdata);
      tstcheck (-10 <= tstcurdata && tstcurdata <= 10);
    }
  }

  tstcase("No data!") {
    tstdatafor("Not sure what") {
      tstnote("Checking: %d", tstcurdata);
      tstcheck (-10 <= tstcurdata && tstcurdata <= 10);
    }
  }

}
