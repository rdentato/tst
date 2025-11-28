#include "tst.h"

tst_main(tst, "Semantic Test") {
    tstcase("Fast test 1") {
        tstcheck(1 == 1);
    }
    
    tstcase("Slow test", +SlowTests) {
        tstcheck(1 == 1);
    }
    
    tstcase("Fast test 2") {
        tstcheck(1 == 1);
    }
}
