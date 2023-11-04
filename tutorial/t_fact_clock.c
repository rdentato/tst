#include "tst.h"
#include "functions.h"

tstrun("Check Factorial speed")
{

  clock_t recursive_elapsed = 0;
  clock_t iterative_elapsed = 0;

  int recursive_result = 0;
  int iterative_result = 0;

  tstcase("recursive") {
    tstclock("Recursive") {
      for (int k=0; k<100000; k++)
        recursive_result = fact_recursive(12);
    }
    recursive_elapsed = tstelapsed();
    tstcheck(recursive_result != 0,"Expect non 0 got: %d", recursive_result);
  }

  tstcase("iterative") {
    tstclock("Iterative") {
      for (int k=0; k<100000; k++)
        iterative_result = fact_iterative(12);
    }
    iterative_elapsed = tstelapsed();
    tstcheck(iterative_result != 0);
  }

  tstcase("Check perfomance") {
    tstcheck(recursive_result  == iterative_result);
    tstcheck(recursive_elapsed >= iterative_elapsed);
  }
}
