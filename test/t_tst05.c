//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

tstsuite("Conditional skipping tests")
{
  tstcase("Conditional skip test 1") {
    int skip_db_tests = 0;
    
    tstcheck(1, "Test DB 0 (always)");
  
    tstskipif(skip_db_tests) {
      tstcheck(1, "Test DB 1");
      tstcheck(1, "Test DB 2");
      tstcheck(1, "Test DB 3");
      tstcheck(1, "Test DB 4");
    } 
    
    tstcheck(1, "Test DB 9 (always)");
  }
    
  tstcase("Conditional skip test 2") {
    int skip_simple_tests = 0;
    
    tstcheck(1, "Test Simple 0 (always)");
  
    tstskipif(skip_simple_tests) {
      tstcheck(1, "Test Simple 1");
      tstcheck(1, "Test Simple 2");
      tstcheck(1, "Test Simple 3");
      tstcheck(1, "Test Simple 4");
    } 

    tstcheck(1, "Test Simple 9 (always)");
  }
}
