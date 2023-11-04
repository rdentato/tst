//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include <tst.h>
#include <iostream>

#define datasize(t) (int)(sizeof(t)/sizeof(t[0]))

int f(int n, const char *s) {
  return 1;
}

tstrun("Data driven tests") {
  tstcase("Use static data") {

    struct {int n; const char *s;} data[] = {
             {123, "pippo"},
             {431, "pluto"},
             { 93, "topolino"}
    };

    for(int k =0; k < datasize(data) ; k++) {
      tstnote("Checking <%d,%s>",data[k].n, data[k].s);
      tstcheck(f(data[k].n , data[k].s));
    }
  }

  tstcase("A static integer array in the range [-10 10]") {
    int data[] = {-1,3,4,5};
    for(int k=0; k < datasize(data); k++) {
      tstnote("Checking: %d", data[k]);
      tstcheck (-10 <= data[k] && data[k] <= 10);
    }
  }

  tstcase("A random integer array in the range [-10 10]") {
    srand(time(0));
    int data[4]; // array size must be specified
    for (int k=0; k<4; k++) data[k] = 8-(rand() & 0x0F);
    for(int k=0; k < datasize(data); k++) {
      tstnote("Checking: %d", data[k]);
      tstcheck (-10 <= data[k] && data[k] <= 10);
    }
  }
}
