//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

tsttags(NoDB, FileOnly, SimpleRun);

int main(int argc, char *argv[])
{
    tstsettags(argc,argv);
    tstrun() {
      tstgroup(tsttag(NoDB) && !tsttag(SimpleRun)) {
         // Only if NoDB is enabled and SimpleRun is disabled.
      }
    }
}
