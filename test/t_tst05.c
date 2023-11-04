//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

tstrun("Switching groups on and off",NoDB, FileOnly, SimpleRun)
{
  tstcheck("Test NoDB 0 (always)");

  tstskipif(!tsttag(NoDB)) {
    tstcheck("Test NoDB 1");
    tstcheck("Test NoDB 2");
    tstcheck("Test NoDB 3");
    tstcheck("Test NoDB 4");
  } 
  
  tstcheck("Test NoDB 9 (always)");

  tstcase("Simplerun") {
    tstcheck("Test SimpleRun 0 (always)");
  
    tstskipif(!tsttag(SimpleRun)) {
      tstcheck("Test SimpleRun 1");
      tstcheck("Test SimpleRun 2");
      tstcheck("Test SimpleRun 3");
      tstcheck("Test SimpleRun 4");
    } 

    tstcheck("Test SimpleRun 9 (always)");
  }

}
