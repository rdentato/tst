//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

int f(int n, char *s) {
  return 1;
}

tstrun("Sections") {
  tstcase("consume data") {

    tstgroup("Trying Data") {

      struct {int n; char *s;} tstdata[] = {
               {123, "pippo"},
               {431, "pluto"},
               { 93, "topolino"}
      };

      tstsection("verifying <%d,%s>", tstcurdata.n,tstcurdata.s) {
        tstcheck(f(tstcurdata.n , tstcurdata.s));
      }
 
      tstsection("Always skipped") {
        tstcheck(0);
      }

      tstnote("Done with %d",tstcurdata.n);
    }
  }
}
