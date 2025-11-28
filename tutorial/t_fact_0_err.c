#include "tst.h"
#include "functions.h"

tstsuite("Factorials") {
  tstcase("Edge case: 0") {
    tstcheck(fact_0(0) == 1, "Expected 1 got %d", fact_0(0));
  }
  
  tstcase("Basic tests") {
    tstcheck(fact_0(1) == 1);
    tstcheck(fact_0(2) == 2);
    tstcheck(fact_0(3) == 6);
    tstcheck(fact_0(5) == 120);
  }
}
