#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <errno.h>
#include <math.h>

int fact_0(int n);
int fact_recursive(int n);
int fact_iterative(int n);

#define fact(n) fact_recursive(n)

#endif