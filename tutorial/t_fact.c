#include "tst.h"
#include "functions.h"

tstrun("Check Factorial") {
  tstcheck(fact(1) == 1);
  tstcheck(fact(2) == 2);
  tstcheck(fact(3) == 6 );
  tstcheck(fact(10) == 3628800 );
}
