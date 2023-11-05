//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

tstrun("Switching groups on and off",NoDB, FileOnly, SimpleRun)
{
  tstskipif(tsttag(NoDB) && !tsttag(SimpleRun)) {
     tstnote("NoDB && !SimpleRun 1");
     tstcheck("Test 1 (NoDB && !SimpleRun)")
  } 

  tsttag(NoDB,1); // Disable SimpleRun

  tstskipif(tsttag(NoDB) && !tsttag(SimpleRun)) {
     tstnote("NoDB && !SimpleRun 2");
     tstcheck("Test 2 (NoDB && !SimpleRun)")
  } 
 
  tsttag(NoDB,0); // Re-enable SimpleRun

  tstskipif(tsttag(NoDB) && !tsttag(SimpleRun)) {
     tstnote("NoDB && !SimpleRun 2");
     tstcheck("Test 3 (NoDB && !SimpleRun)")
  }  
}
