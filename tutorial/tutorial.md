<a id="top"></a>
# Tutorial

Following the examples of other testing framework, this tutorial wiil guide you through the setup
and the first steps to use `tst`.

**Contents**<br>
[Getting tst](#getting-tst)<br>
[Writing tests](#writing-tests)<br>
[Test cases and sections](#test-cases-and-sections)<br>
[BDD style testing](#bdd-style-testing)<br>
[Data and Type driven tests](#data-and-type-driven-tests)<br>
[Next steps](#next-steps)<br>


## Setting `tst`` up

There is no contstraint on which build system you use as `tst` is a single header library.
You place the file `tst.h` in a location that is in your include path and just `#include` it.

## A minimal example.

Let's define a scenario for our examples: you have a library of functions defined in file `functions.c`
whose prototypes are also in `functions.h` and you want to properly test them to ensure their correctness.

You'll find the code in the `test` directory.

The first function we'll look at is the one to calculate the factorial of a number:
(Note: There will be further versions of this function along the tutorial, they will be numbered
with `fact_0`, `fact_1`, etc.)


```c
unsigned int fact_0(unsigned int n)
{
  if (n<=1) return n;
  return n * fact_0(n-1);
}
```

To write test cases for this function, you will create a separate file called, say, `t_fact_0.c` with
all the checks you wannt to perform on the function:

```c
#include "tst.h"
#include "functions.h"

tstrun("Factorials") {
  tstcheck(fact_0(1) == 1);
  tstcheck(fact_0(2) == 2);
  tstcheck(fact_0(3) == 6);
  tstcheck(fact_0(5) == 120);
}
```
Once you compile and link it with the file where the `Factorial()` function is defined, you'll get
an executable, say `t_fact` that, when runn, will execute all the tests. report the results:

```
FILE ▷ t_fact.c "Check Factorial"
PASS│  fact(1) == 1 :5
PASS│  fact(2) == 2 :6
PASS│  fact(3) == 6 :7
PASS│  fact(5) == 120 :8
RSLT ▷ 0 KO | 4 OK | 0 SKIP
```
The idea is to have a single executable file which defines a `run` of tests that will cover a
logically related set of functions or will go over a specific use case.

The `tstrun()` function will serve as `main()`: you don't need (and should not) define a `main()` function.

## Test failures
If you looked carfefully at the `fact_0()` function you may have noticed that there is, actually, a bug in it.
We didn't detected it becauses we failed to check for one of the boundary case: 0! = 1.
If we did it:

```c
#include "tst.h"
#include "functions.h"

tstrun("Factorials") {
  tstcheck(fact_0(0) == 1); // Test edge case
  tstcheck(fact_0(1) == 1);
  tstcheck(fact_0(2) == 2);
  tstcheck(fact_0(3) == 6);
  tstcheck(fact_0(5) == 120);
}
```
We would have got:
```
FILE ▷ t_fact_0_err.c "Check Factorial"
FAIL│  fact_0(0) == 1 :5
PASS│  fact_0(1) == 1 :6
PASS│  fact_0(2) == 2 :7
PASS│  fact_0(3) == 6 :8
PASS│  fact_0(5) == 120 :9
RSLT ▷ 1 KO | 4 OK | 0 SKIP
```

When a check fails, we might want to print a message to better understand what went wrong.
We can do it as follows:

```C
  tstcheck(fact_0(0) == 1, "Expected 1 got %d", fact_0(0));
```
and we would have got:
```
FAIL├┬ fact_0(0) == 1 :5
    │╰ Expected 1 got 0
```

## Structuring tests

Within a *test run*, you will define *test cases*.



[Home](Readme.md#top)
