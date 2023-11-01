<a id="top"></a>
# Tutorial

Following the examples of other testing framework, this tutorial wiil guide you through the setup
and the first steps to use `tst`.

**Contents**<br>
[Setting tst up](#setting-`tst`-up)<br>


## Setting `tst` up

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
int fact_0(int n)
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

## Handling failures
If you looked at the `fact_0()` function you may have noticed that there is, actually, a bug in it.
We didn't detected it becauses we failed to check for one of the cases case: 0! = 1.
Let's do it:

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
Note how failures are reported as the first number in the *results* line. That's because,
most probably, the first thing we want to know if everything went right.

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

I found it bothersome to specify a message for every check. After all, most of the time
the checks will always pass. I usually only add messages when a check fails and it's
not obviuos why.
Of course, when the message is there I'd leave it for the next run; there's no need
to remove it.

## On the expression to check

The example above gives us the opportunity to talk about the expressions used in the 
checks. The log produced during a run will contain those expression to make easier
to identify what went wrong.

Writing:
```c
  tstcheck(fact(0) == 1);
```

that, assuming failure, will produce:
```
FAIL│  fact(0) == 1 :5
```

is much better than writing:
```c
  n = fact(0);
  tstcheck(n == 1);
```

that would produce:
```
FAIL│  n == 1 :5
```
with no indication of what the test actually was.

On the other and, when adding the error message you might need to recalculate the
function again which, in some case, might not be advisable.

A simple way to avoid that is to assign the result to a temporary variable directly
in the `tstcheck()` function:

```c
  tstcheck((n=fact(0)) == 1, "Expected 1 got %d", n);

```

If this is note feasible, for example because the result comes from a complex
computation that you don't want to perform twice, you may add a string to remind
what that test was about using this idiomatic form:

```c
  ... long calculation to compute n ...
  tstcheck("Starship orbit intersection" && (n == 0), "Expected 0 got %d", n);


```
If this fails, it will produce:

```
FAIL├┬ "Starship orbit intersection" && (n == 0) :35
    │╰ Expected 0 got 23
```
Which contains a clear indication on what the test was about.

## Structuring tests

Within a test run, which is supposed to cover a logically meaniful scenario, you 
may want to define multiple *test cases* whose checks are tightely related.

For example, let's write a full test run for a more complete version of the factorial:

```c
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
    tstcheck(fact(12) == 479001600); // 12! is the largest number to fit a 32bit int.
  }

  tstcase("Out of range") {
    tstcheck((fact(-3) == 0) && (errno == ERANGE));
    tstcheck((fact(21) == 0) && (errno == ERANGE));
  }
}
```

This will produce the following result:
```
FILE ▷ t_fact.c "Check Factorial"
CASE┬── Edge case: 0 :5
PASS│  fact(0) == 1 :6
    ╰── 0 KO | 1 OK | 0 SKIP
CASE┬── Small input :9
PASS│  fact(1) == 1 :10
PASS│  fact(2) == 2 :11
PASS│  fact(3) == 6 :12
PASS│  fact(5) == 120 :13
    ╰── 0 KO | 4 OK | 0 SKIP
CASE┬── Edge case: largest input :16
PASS│  fact(12) == 479001600 :17
    ╰── 0 KO | 1 OK | 0 SKIP
CASE┬── Out of range :20
PASS│  (fact(-3) == 0) && (errno == ERANGE) :21
PASS│  (fact(21) == 0) && (errno == ERANGE) :22
    ╰── 0 KO | 2 OK | 0 SKIP
RSLT ▷ 0 KO | 8 OK | 0 SKIP
```



[Home](Readme.md#top)
