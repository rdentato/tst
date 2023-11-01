#include "functions.h"

 int fact_0(int n)
{
  if (n<=1) return n; // BUG: the return for 0 will be wrong!
  return n * fact(n-1);
}

int fact_recursive(int n)
{
  if (n<=1) return 1;
  return n * fact_recursive(n-1);
}

int fact(int n)
{
  if (n < 0 || 12 < n) {
    errno = ERANGE;
    return 0;
  }
  errno = 0;
  return fact_recursive(n);
}

