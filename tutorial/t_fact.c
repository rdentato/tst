#include "tst.h"
#include "functions.h"

tstrun("Check Factorial") {
  tstcase("Edge case: 0") {
    tstcheck(fact(0) == 1); // 0! = 1
  }

  tstcase("Small input") {
    tstcheck(fact(1) == 1);
    tstcheck(fact(2) == 2);
    tstcheck(fact(3) == 6 );
    tstcheck(fact(5) == 120 );
  }

  tstcase("Edge case: largest input") {
    tstcheck(fact(12) == 479001600);
  }
  
  tstcase("Out of range") {
    tstcheck((fact(-3) == 0) && (errno == ERANGE));
    tstcheck((fact(21) == 0) && (errno == ERANGE));
  }
}
