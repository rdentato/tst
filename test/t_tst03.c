//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

#define datasize(t) (int)(sizeof(t)/sizeof(t[0]))

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

    for(int k =0; k < datasize(tstdata) ; k++) {
      tstnote("Checking <%d,%s>",tstdata[k].n,tstdata[k].s);
      tstcheck(f(tstdata[k].n , tstdata[k].s));
    }
  }

  tstcase("A static integer array in the range [-10 10]") {
    int tstdata[] = {-1,3,4,5};
    for(int k=0; k < datasize(tstdata); k++) {
      tstnote("Checking: %d", tstdata[k]);
      tstcheck (-10 <= tstdata[k] && tstdata[k] <= 10);
    }
  }

  tstcase("A random integer array in the range [-10 10]") {
    srand(time(0));
    int tstdata[4]; // array size must be specified
    for (int k=0; k<4; k++) tstdata[k] = 8-(rand() & 0x0F);
    for(int k=0; k < datasize(tstdata); k++) {
      tstnote("Checking: %d", tstdata[k]);
      tstcheck (-10 <= tstdata[k] && tstdata[k] <= 10);
    }
  }
}
