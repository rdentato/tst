//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

tstsuite("Tag System Test")
{
    tstcase("Always runs") {
        tstcheck(1 == 1, "Basic test");
    }
    
    tstcase("Database test", +RequiresDB) {
        tstcheck(2 == 2, "DB test");
    }
    
    tstcase("Slow test", +SlowTests, +RequiresDB) {
        tstcheck(3 == 3, "Slow DB test");
    }
    
    tstcase("Fast test", -SlowTests) {
        tstcheck(4 == 4, "Fast test");
    }
    
    tstcase("Unit test", +UnitTests) {
        tstcheck(5 == 5, "Unit test");
    }
    
    tstcase("Integration test", +Integration, +RequiresDB) {
        tstcheck(6 == 6, "Integration with DB");
    }
    
    tstcase("Quick test", -SlowTests, -RequiresDB) {
        tstcheck(7 == 7, "Quick offline test");
    }
}
