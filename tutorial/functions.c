#include "functions.h"

 int fact_0(int n)
{
  if (n<=1) return n; // BUG: the return for 0 will be wrong!
  return n * fact_0(n-1);
}

volatile int  x = 0; // To ensure GCC won't optimize the tail recursion
int fact_rec(int n)
{
  if (n<=1) return 1;
  return (x+fact_rec(n-1)) * n;
}

int fact_recursive(int n)
{
  if (n < 0 || 12 < n) {errno = ERANGE; return 0; }
  errno = 0;
  return fact_rec(n);
}

int fact_iterative(int n)
{
  if (n < 0 || 12 < n) {errno = ERANGE; return 0; }
  errno = 0;
  int res = 1;
  while (n>1) res *= n--;
  return res;
}
