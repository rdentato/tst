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

If this is not feasible, or even just inconvenient, for example because the result 
comes from a complex computation that you don't want to perform twice, you may add
a string to remind what that test was about using this idiomatic form:

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

The usefulness of `tstcase` is that it collects partial results and will allow you
to focus on sets of tests rather than having to consider all the tests at once.

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

While it is possible to nest test cases, it would be much clearer to 
just write test cases one after the other.

## Conditional test execution (and tagging)

Let's say that you just have a set of tests to perform that use a DB but you
can't get access to it. It would be useless to perform them, for cases like this
you can rely on `tstif()`:

``` C
  tstcase("Read from FILE") {
    FILE *f = NULL;

    tstcheck(f = fopen("datafile.dat",rb), "Unable to open data file");
    tstif(f,"Skipping file tests") {
      // A bunch of checks that should read from the file
    } else {
      // Alterantive checks (or any appropriate action)
    }
  }
```
The `testif` function is also the basis for create tags. Say you have a set of 
tests that are very expensive to run (e.g. too slow) and you want to be able to
exclude them. For this you can create up to eight tags per run and switch them
on/off.

To check if a *tag* is enabled, you use the `tsttag()` function:

```C
tstrun("Do a bunch of tests",TestDB, DeepTest, SimpleRun)
{
  tstcase() {
    tstif(tsttag(TestDB) && !tsttag(SimpleRun)) {
       // Only if TestDB is enabled and SimpleRun is disabled.
       tstcheck(db.connection != NULL);
    }
    tstcheck(4 > p);
  }

  // You can enclose full testcases if you want!
  tstif(tsttag(DeepTest)) {
     testcase("Full tests") {
        // checks here
     }

     testcase("Full tests twice") {
        // checks here
     }
  }
}
```

By default all tags are enabled. You can disable them when launching the test run.
For example, to run `mytest` with the "DeepTest" disabled you can execute it this way:
```
  $ mytest -DeepTest
```
To know which tags are defined you can use the `?` command line option:

```
  $ mytest ?
  Test Scenario: "Switching groups on and off"
  ./mytest [? | [=] [+/-]tag ... ]
  tags: TestDB DeepTest SimpleRun
```

You can also switch all the tags on/off using `*`. For example, to disable
all tags except `SimpleRun`, you can execute the test as follows:

```
  $ mytest -* +SimpleRun
```

You can also set the tag on and off in the code using the `tsttag()` function:

```C
  tsttag(SimpleRun, 0); // Disable the testst guarded by the SimpleRun tag
  tsttag(SimpleRun, 1); // Re-enable the testst guarded by the SimpleRun tag
```



## Data driven tests

It might be useful to repeatedly perform a set of tests on a given set of data.

For this you can use the `tstdata` functions.

This is an example on how to do it:

``` C
  tstcase("Use static data") {

    struct {int n; char *s;} tstdata[] = {
             {123, "pippo"},
             {431, "pluto"},
             { 93, "topolino"}
    };

    tstdatafor("Verify edge cases") {
      tstnote("Checking <%d,%s>",tstcurdata.n,tstcurdata.s);
      tstcheck(f(tstcurdata.n , tstcurdata.s));
    }
  }
```

You first define the `tstdata` array and then use `tstdatafor()` to loop over the array.
To access the current element of the array, you use `tstcurdata`.

You are free to choose whatever type suits you most for the array. It can be a `struct` as in 
the example above or could be a basic type like an int:

```c
  tstcase("A static integer array") {
    int tstdata[] = {-1,3,4,5};
    tstdatafor("Integers in the range [-10 10]") {
      tstnote("Checking: %d", tstcurdata);
      tstcheck (-10 <= tstcurdata && tstcurdata <= 10);
    }
  }
```

This is very helpful if you want to extract a sample of the test data from a database or
just randomly generate them like in this example:

```c
  tstcase("A random integer array") {
    srand(time(0));
    int tstdata[4]; // array size must be specified
    for (int k=0; k<4; k++) tstdata[k] = 8-(rand() & 0x0F);
    tstdatafor("Integers in the range [-10 10]") {
      tstnote("Checking: %d", tstcurdata);
      tstcheck (-10 <= tstcurdata && tstcurdata <= 10);
    }
  }
```

[Home](Readme.md#top)
