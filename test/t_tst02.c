//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

tstsuite("Sections") {
  tstcase("All sections") {
    int a = -1;

    tstcase("Keeping 5 starting from %d",a) {
      
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
      a = -1;
    }

    tstcheck(a==-1);
  }

  tstcase("No sections") {
    int a = -1;

    tstcase("Empty") {
      a = 5;
      tstcheck(a==5);

      a = 0;
    }

    tstcheck(a==0);
  }

  tstcase("Nested sections") {
    int a = -1;

    tstcase("keep 100") {

      a = 5;
      tstcheck(a==5);

      tstsection("Set to 10") {
        a=10;
        tstcheck(a==10);
      }

      tstsection("Set to 100") {
        tstcase("keep a = %d",a) {
          a = 100;
   
          tstsection("Set to 200") {
            tstcheck(a==100);
            a = 200;
          }

          tstsection("Set to 400") {
            tstcheck(a==100);
            a = 400;
          }
        }
        tstcheck(a==400,"Expected 400, got %d", a);
      }

      a = -1;
    }

    tstcheck(a==-1);
  }
}
