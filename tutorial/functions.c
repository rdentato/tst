#include "functions.h"

unsigned int fact_0(unsigned int n)
{
  if (n<=1) return n;
  return n * fact(n-1);
}

unsigned int fact(unsigned int n)
{
  if (n<=1) return 1;
  return n * fact(n-1);
}

