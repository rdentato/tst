//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include <tst.h>
#include <iostream>

tstrun("Switching groups on and off",NoDB, FileOnly, SimpleRun)
{
  tstif(tsttag(NoDB) && !tsttag(SimpleRun)) {
     tstnote("NoDB && !SimpleRun 1");
  } 

  tsttag(SimpleRun,0); // Disable SimpleRun

  tstif(tsttag(NoDB) && !tsttag(SimpleRun)) {
     tstnote("NoDB && !SimpleRun 2");
  } 
 
  tsttag(SimpleRun,1); // Re-enable SimpleRun

  tstif(tsttag(NoDB) && !tsttag(SimpleRun)) {
     tstnote("NoDB && !SimpleRun 2");
  } 
}
