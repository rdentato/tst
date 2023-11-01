//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include <tst.h>
#include <iostream>

tstrun("Switching groups on and off",NoDB, FileOnly, SimpleRun)
{
  tstif(tsttag(NoDB) && !tsttag(SimpleRun)) {
     // Only if NoDB is enabled and SimpleRun is disabled.
  }
}
