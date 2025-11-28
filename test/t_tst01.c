//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

tstsuite("Basic skipping tests")
{
  tstcase("Skipping") {
    int skip_condition = 0;
    
    tstskipif(skip_condition) {
       tstcheck(1, "Test 1 (should run)")
    } 
  
    skip_condition = 1;
  
    tstskipif(skip_condition) {
       tstcheck(1, "Test 2 (should be skipped)")
    } 
   
    skip_condition = 0;
  
    tstskipif(skip_condition) {
       tstcheck(1, "Test 3 (should run)")
    }  
  }
}
