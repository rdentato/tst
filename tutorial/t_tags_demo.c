//  SPDX-FileCopyrightText: © 2023 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"

tstsuite("Tag Demonstration") {
  tstcase("Quick validation") {
    // This test always runs (no tags)
    tstcheck(1 + 1 == 2, "Basic math works");
  }
  
  tstcase("Database connection test", +database) {
    // This test only runs when you specify +database
    tstcheck(1 == 1, "Simulating database connection");
  }
  
  tstcase("Slow performance test", +slow, +database) {
    // This test needs explicit enabling
    tstcheck(2 + 2 == 4, "Simulating slow test");
  }
  
  tstcase("Interactive prompt", -ci) {
    // This test runs by default but is skipped in CI
    tstcheck(3 + 3 == 6, "Simulating user interaction");
  }
  
  tstcase("Multiple conditions", +network, -ci) {
    // Runs if +network OR if -ci is specified
    tstcheck(4 + 4 == 8, "Simulating network test");
  }
}
