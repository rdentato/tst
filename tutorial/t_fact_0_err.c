#include "tst.h"
#include "functions.h"

tstrun("Check Factorial") {
  tstcheck(fact_0(0) == 1);
  tstcheck(fact_0(1) == 1);
  tstcheck(fact_0(2) == 2);
  tstcheck(fact_0(3) == 6 );
  tstcheck(fact_0(5) == 120 );
}
