<a id="top"></a>
# Tutorial

Following the examples of other testing framework, this tutorial wiil guide you through the setup
and the steps need to use `tst`.

A word of caution. The functions offered here are simple but rather flexible and allow you to 
create very complicated and intricated test cases. Please don't! Besides its purpose of ensuring
the correctness of the system, the test suite also has a great value as an example on how to use
the various functions. Try to write the tests as cleanly and simply as possible and you'll save 
the day of those that will have to understand the code later.

**Contents**<br>
[Setting tst Up](#setup)<br>
[A Minimal Example](#min-example)<br>
[Handling Failure](#)<br>
[Assertions](#assertions)<br>
[On the Expression to Check](#check-expression)<br>
[Structuring Tests Using `tstcase`](#testcase)<br>
[Sections](#sections)<br>
[Data Driven Tests](#data-driven)<br>
[Conditional Test Execution (and Tagging)](#conditional)<br>
[Disabling tests at compile time](#disabling)<br>
[Split Tests](#split-tests)<br>
[Command Line Options](#command-line)<br>
[Running your tests](#running-tests)<br>

<a id=setup></a>
## Integrating `tst` into Your Project

There is no contstraint on which build system you use as `tst` is a single header library.
You place the file `tst.h` in a location that is in your include path and just `#include` it.

However, if you use `make` as your build tool, you can benefit of the examples provided in the
project directory. By organizing your test files and leveraging the provided `makefile`, you
can seamlessly manage and execute your unit tests. Here's a step-by-step guide to doing this:

### 1. **Set Up a `test` Directory**

To keep your project organized, create a separate `test` directory within your project's root folder. This dedicated directory will house your unit test files, `tst.h` header, and the `makefile`.

```bash
mkdir test
```

### 2. **Copy Essential Files**

- **`tst.h`**: This is the main header file for the `tst` framework. Ensure it is visible to your compiler (e.g. with `-I ../src`)
  
- **`makefile`**: The provided `makefile` will contain rules to compile and run your unit tests. Ensure this is placed within the `test` directory. Modify the flags in the `CFLAGS` variable as you deem appropriate for your project. For example, it assumes that `tst.h` is in the `../src` directory but this might not be the case for you.

```bash
cp path_to_tst_files/src/tst.h path_to_project/src/
cp path_to_tst_files/test/makefile path_to_project/test/
```

### 3. **Naming Convention for Test Files**

To ensure that the `makefile` correctly identifies unit test files:

- Name your test files with the prefix `t_`, followed by a descriptive name, and then the `.c` extension. For example: `t_mathFunctions.c`.

- By sticking to this naming convention, the `makefile` can easily detect which C files in the directory are intended as unit tests.

Please note that the provided `makefile` also assumes that you want to compile your tests both with a C and a C++ compiler.
If this is not the case, comment the line that defines the variable `CXX_AVAILABLE`.

### 4. **Compiling and Running Tests**

- **Compile a Single Test**:
  If you have a test file named `t_xxx.c` and wish to compile it, simply run:

  ```bash
  make t_xxx
  ```

  This will compile the `t_xxx.c` file, creating an executable for the test.

- **Compile and Run All Tests**:
  To compile and execute all unit tests present in the `test` directory:

  ```bash
  make runtest
  ```

  The `makefile` will automatically detect all C files prefixed with `t_`, compile them, and execute each test.

  Note that if you add a new test you don't need to change anything in the makefile, as long as you follow the 
  naming convention, the new test will be picked up automatically.

### Benefits

By setting up a dedicated `test` directory and leveraging the provided `makefile`, you can effortlessly manage and run unit tests using the `tst` framework. This structure not only ensures a clean project layout but also streamlines the testing process, making it easier for developers to maintain and expand upon their test suites.

<a id="min-example"></a>

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
------ FILE ▷ t_fact.c "Check Factorial"
     5 PASS│  fact(1) == 1
     6 PASS│  fact(2) == 2
     7 PASS│  fact(3) == 6
     8 PASS│  fact(5) == 120
^^^^^^ RSLT ▷ 0 FAIL | 4 PASS | 0 SKIP
```
The idea is to have a single executable file which defines a `run` of tests that will cover a
logically related set of functions or will go over a specific use case.

The `tstrun()` function will serve as `main()`: you don't need (and should not) define a `main()` function.

<a id="failures"></a>

## Handling failures
If you looked at the `fact_0()` function in the previous section, you may have noticed that there is, actually, a bug in it.
We didn't detected it becauses we failed to check for one of the edge case: 0! = 1.
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
------ FILE ▷ t_fact_0_err.c "Check Factorial"
     5 FAIL│  fact_0(0) == 1
     6 PASS│  fact_0(1) == 1
     7 PASS│  fact_0(2) == 2
     8 PASS│  fact_0(3) == 6
     9 PASS│  fact_0(5) == 120
^^^^^^ RSLT ▷ 1 FAIL | 4 PASS | 0 SKIP
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
    5 FAIL├┬ fact_0(0) == 1
          │╰ Expected 1 got 0
```

I find it bothersome to specify a message for every check. After all, most of the time
the checks will always pass. I usually only add messages when a check fails and it's
not obviuos why.
Of course, when the message is there I'd leave it for the next run; there's no need
to remove it.

<a id="assertions"></a>

## Assertions
Assertions are a stronger form of checking. For example, if the following test fails:

```C
tstassert(ptr = malloc(n),"Out of memory (requested: %d)",n);
```
the program will be aborted as there's little meaning in continue testing when the memory is exausted.

The section on conditional execution provides more information on how to handle failures that are not
critical and would allow other tests to be executed.

<a id="check-expression"></a>

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
    5 FAIL│  fact(0) == 1
```

is much better than writing:
```c
  n = fact(0);
  tstcheck(n == 1);
```

that would produce:
```
    5 FAIL│  n == 1
```
with no indication of what the test actually was.

On the other hand, when adding the error message you might need to recalculate the
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
    35 FAIL├┬ "Starship orbit intersection" && (n == 0) :35
           │╰ Expected 0 got 23
```
Which contains a clear indication of what the test was about.

<a id="tstcase"></a>

## Structuring tests using `tstcase`

Within a test run, which is supposed to cover a logically meaniful scenario, you 
may want to define multiple *test cases* whose checks are tightely related.

The usefulness of `tstcase` is that it collects partial results and will allow you
to focus on groups of tests rather than having to consider all the tests at once.

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
----- FILE ▷ t_fact.c "Check Factorial"
    5 CASE┬── Edge case: 0
    6 PASS│  fact(0) == 1
    5     ╰── 0 FAIL | 1 PASS | 0 SKIP
    9 CASE┬── Small input
   10 PASS│  fact(1) == 1
   11 PASS│  fact(2) == 2
   12 PASS│  fact(3) == 6
   13 PASS│  fact(5) == 120
    9     ╰── 0 FAIL | 4 PASS | 0 SKIP
   16 CASE┬── Edge case: largest input
   17 PASS│  fact(12) == 479001600
   16     ╰── 0 FAIL | 1 PASS | 0 SKIP
   20 CASE┬── Out of range
   21 PASS│  (fact(-3) == 0) && (errno == ERANGE)
   22 PASS│  (fact(21) == 0) && (errno == ERANGE)
   20     ╰── 0 FAIL | 2 PASS | 0 SKIP
^^^^^ RSLT ▷ 0 FAIL | 8 PASS | 0 SKIP
```
Note that at the end of each `tstcase` the partial results are reported. The line
number is the same as the starting line of the test case so to make it easier to 
check which test case is closed.
Note that, while it is possible to nest test cases, it's better not doing it for
the sake of clarity.

<a id="sections"></a>

## Sections

Within a `tstcase` you may have multiple `tstsection`s.:

```C
int a;
tstcase("Sections") {
  a = 5;
  tstsection("Change to 9") {
    tstcheck(a == 5);
    a = 9;
    tstcheck(a == 9);
  }
  tstsection("Change to 8") {
    tstcheck(a == 5);
    a = 8;
    tstcheck(a == 8);
  }
  tstcheck(a != 5);
}
tstcheck(a == 8);
```

All the tests above will pass. When a section is executed, all the subsequent sections
are ignored. Then the  testcase is re-executed for the next section and so on.

The code after the last section (usually the cleanup code) will be executed and the
test case will end.

This can be useful if you want to ensure that groups of tests are executed starting 
from the same status. Let's give another example:

```C

tstcase("Testing from file") {

   FILE *f = NULL;

   tstassert(f == NULL)
   tstassert(f = fopen("mydata file","rb"));

   tstsection ("First five are uppercase") {
    int c;
    for (int k=0; k<5; k++) {
      tstcheck( isupper(c=fgetc(f)),"Not an uppercase letter '%c'",c);
    }
   }

   tstsection ("First five are in ascending order") {
    int c=0,prev=0;
    for (int k=0; k<5; k++) {
      tstcheck( (c = fgetc(f)) >= prev, "Letter %c in position %d not ordered",c,k);
      prev = c;
    }
   }

   fclose(f);
   f = NULL;
}

```
Before each `tstsection` is executed, the file will be opened and after the `tstsection`
has been completed, the file is closed. 

You can imagine much more complex scenarios involving, for example, allocate and free memory
with `malloc()`/`free()`, or connecting to a Database or to a network server.

<a id="data-driven"></a>

## Data driven tests

Another feature of `tstsection`s is that they can be executed on a given array of data.
You define an array named `tstdata` within your `tstcase` andaccess the current data
element with `tstcurdata`. For example:

``` C
  tstcase("Data as static array") {

    struct {int n; char *s;} tstdata[] = {
             {123, "pippo"},
             {431, "pluto"},
             { 93, "topolino"}
    };

    tstsection("My check") {
      tstnote("Checking <%d,%s>",tstcurdata.n,tstcurdata.s);
      tstcheck(f(tstcurdata.n , tstcurdata.s));
    }
  }
```

Note that, considering how `tstsection`s are executed, you can do something like this:

``` C
  tstcase("Data as static array") {

    struct {int n; char *s;} tstdata[] = {
             {123, "pippo"},
             {431, "pluto"},
             { 93, "topolino"}
    };

    tstsection("First check") {
      tstnote("Checking <%d,%s>",tstcurdata.n,tstcurdata.s);
      tstcheck(first_check(tstcurdata.n , tstcurdata.s));
    }

    tstsection("Second check") {
      tstnote("Checking <%d,%s>",tstcurdata.n,tstcurdata.s);
      tstcheck(second_check(tstcurdata.n , tstcurdata.s));
    }
  }
```
The `"first check"` section will be executed for each element of the `tstdata` array
and the the `"Second check"` section will be executed for each element of the array.

This can also be used for *fuzzing* (i.e. execute many tests with random data):

``` C
  tstcase("Data as static array") {

    int tstdata[4];
    
    for(int cycle = 0; cycle < 100; cycle++) { // Do it 100 times

      for (int k=0; k<4; k++)
        tstdata[k] = 8-(rand() & 0x0F);  // Generate some random data

      // Execute the section for each element in the tstdata array
      tstsection("First check") {
        tstcheck(fuzzy_check(tstcurdata));
      }
    }
  }
```
<a id="conditional"></a>

## Conditional test execution (and tagging)

Let's say you have a set of tests that use a DB but currently you
can't get access to it. There are other tests in the test suite that you want
to perform. How can you do it?

For cases like this you can rely on `tstskipif()`. It makes so that if the 
condition is true, all the tests in its scope will be disabled (and skipped on 
execution):

``` C
  tstcase("Read from FILE") {
    FILE *f = NULL;

    tstcheck(f = fopen("datafile.dat",rb), "Unable to open data file");
    tstskipif(f == NULL) {
      // A bunch of checks that should read from the file
      // They will be skipped the f == NULL
    } 

    // other test cases you do want to execute regardless.
  }
```

The `tstskipif` function is also the basis for handling tags. Say you have a set of 
tests that are very expensive to run (e.g. too slow) and you want to be able to
exclude them for certain runs. For this you can create up to eight tags per
run and switch them on/off.

You specify the tags you want to use (up to eight) as additional parameter
to the `tstrun()` function and check them with the `tsttag()` function:

```C
tstrun("Do a bunch of tests",TestDB, DeepTest, SimpleRun)
{
  tstcase() {
    tstskipif(tsttag(TestDB) && !tsttag(SimpleRun)) {
       // Only if TestDB is enabled and SimpleRun is disabled.
       tstcheck(db.connection != NULL);
    }
  }

  // You can enclose full testcases if you want!
  tstskipif(!tsttag(DeepTest)) {
     testcase("Full tests") {
        // Many checks here
     }

     testcase("Full tests twice") {
        // Even more checks here
     }
  }
}
```

By default all tags are disabled. You can disable them when launching the test run.
See the section ["Command line options"](#command-line) for more details.


You can also set the tag on and off in the code using the `tsttag()` function:

```C
  tsttag(SimpleRun, 0); // Disable the testst guarded by the SimpleRun tag
  tsttag(SimpleRun, 1); // Re-enable the testst guarded by the SimpleRun tag
```

### Using make

If you are using the makefile provided in the `test` directory to run your tests,
you can easily specify which tags to enable by setting the `TSTTAGS` variable:

  ```
    $ TSTTAGS=-NODB make -B runtest
  ```
<a id="disabling"></a>

## Disabling tests at compile time

There are cases when you may want to remove some test cases form your test suite
but you do not want to remove them from the code because they might be useful 
later.

A typical example is if some feature is undergoing some major rewriting that would
make your tests useless until all the new code is completed.

Another one is when you want to focus on certain tests for debugging purpose and
want to create a smaller log for making easeier understanding what went wrong.

You might handle this with some `#ifdef` in your code or by defining ad hoc tags
(see previous section) but this seems pretty annoying to me.

A much easier way is to use a different form of the `tst` functions: you just
add an underscore after `tst` and that function will behave as if it was not there.
For example if you have this test case:
```C
   tstcase ("Check for 0") {

   }
```
and you want to leave it out during compilation, you just change it into:
```
   tst_case ("Check for 0") {

   }
```
note the underscore `_` after the `tst` part.

Similarly:

```C
   tstcheck(x<0,"Too small! %d", x);    // Check enabled
   tst_check(x==0,"Not zero! %d", x);   // Check disabled
```

You can also disable an entire test scenario changing `tstrun` into `tst_run()`.


<a id="split-tests"></a>

## Split tests
Usually the `tstcheck()` function is enough to handle the test results but there might be cases when you want to perform some more actions depending on the fact that the test passed or not.

For this there are the following functions:

- `tst()` Just perform the test.
- `tstpassed()` Returns true if the previous test check (with `tst()` or `tstcheck()`) passed.
- `tstfailed()` Returns true if the previous test check (with `tst()` or `tstcheck()`) failed.
- `tstskipped()` Returns true if the previous test check (with `tst()` or `tstcheck()`) had been skipped.

Example:
```C
   tstcheck(x != 3); // Perform the test
   if (tstpassed()) {
      // Do domething
   }

   tst(z > 0); // Perform the test but does not report it in the log
   if (tstfailed()) {

   }
```
Note that `tstpassed()` and `tstfailed()` report the result of the latest check.

<a id="clock"></a>
## Checking times

It may be useful, sometimes, to get an idea of how much time is spent in one particular piece of code.
For example, you may want to understand which implementation of a given function performs better.
The function `tstclock()` can help you in that measuring the processor time spent between the start and
the end of a block of code.

Here an example to check that the recursive implementation of the factorial is slower than the 
iterative implementation:

```C

```

<a id="command-line"></a>
## Command line options

When a `tstrun` is compiled, it will define a main function that will accept the
the following options.

### Help
Specifing `/h` as argument, you'll get a short help.

If no tag is specified you'll get something similar to this:

```
  $ mytest ?
  Test Scenario: "A run for my tests"
  ./mytest /help | [/color-off] [/report-off]
```

The *Test Scenario* is the title you provided in the `tstrun()` function.

See below for more details on when there is any tag specified.

### Not Returning Errors

By default, if a test fails, it will return 1 to signal that there has been one or more errors.
This might be undesirable if, for example, the test is run in a script that could interrupt
the execution of all the tests.

You can avoid this by specifiying `=` as the first argument:

```bash
  $ t_test /return-off
```
so that `t_test` will always return 0 even if some tests may have failed.

### Handling tags

If you specified one or more tag, you will receive a help message like this:

```
  $ mytest ?
  Test Scenario: "Switching groups on and off"
  ./mytest [/help | [/color-off] [/report-off] [+/-]tag ... ]
  tags: TestDB DeepTest SimpleRun
```
that helps you remember which tags you defined.

By default all tags are *off*, to turn it on you can pass its name to the test program.
For example, to run `mytest` with the "DeepTest" enabled you can execute it this way:

```
  $ mytest +DeepTest
```

You can also switch all the tags on/off using `*`. For example, to enable
all tags except `SimpleRun`, you can execute the test as follows:

```
  $ mytest +* -SimpleRun
```
<a id="running-tests"></a>

## Running your tests

There is no limitation on how you organize and run your tests. Once you have compiled
the test program, you can launch it on its own or add to a CI pipeline or wathever is 
most appropriate for you.

As an example I'll describe here how I set up `tst` for self-testing. You may use the 
same conventions or define your own.

I've decided to adopt the following conventions:

-  Tests are grouped in dedicated directories (at least one)
-  Tests start with `t_*`
-  Tests are run from the shell (bash)

I've then created a bash script (`src/tstrun`) which provides more flexibility in launching
the tests. It will also provide the total of failed/passed/skipped tests.

Let's look at its `usage()`:

```
Usage:
   tstrun [options] [wildcard] [tags]

OPTIONS
  -h | --help                 this help
  -l | --list                 prints the list of available tests
  -c | --color-off            turns off coloured messages
  -d | --test-directory dir   cd to the directory dir with tests
  -w | --wildcard '*x[yz]'    specify a file pattern to match the tests to execute
  -o | --output filename      the name of the generated logfile

WILDCARD
  A filter to select which tests to run ('*' by default). Note that it MUST be
  single quoted to prevent shell expansion. The initial 't_' is implied.

TAGS
  [+/-]tagname to turn the tag on/off
```
The options are self explanotory and their use case should be pretty intuitive.
The `tags` are just passed to each test program as specified.

The `wildcard`, instead, is more interesting as it introduce a furter degree of 
flexibility in how you launch you tests. On top of organizing them in directories
and defining tags for including/excluding certain tests, you can define some
naming convention to finely select which test to run.

For example, say you have two tests directories (an "old test suite" and a "new test suite"):
```
  test_oldsuite
     t_login_prod.c
     t_login_devel.c
     t_zoom_prod.c
     t_zoom_devel.c

  test_newsuite
     t_login_prod.c
     t_login_devel.c
     t_zoom_prod.c
     t_zoom_devel.c
```

and your test program understand the tags `Interactive` and `LinearScale`.

You can run all the tests from the "old suite" meant for production (`_prod_`)
with the tag `Interactive` enabled:

```
tstrun -w '*_prod_*' -d test_oldsuite +Interactive
```

Or run all the login tests (`_login_`) for the "new suite":

```
tstrun -w '*_login_*' -d test_newsuite +Interactive +LinearScale
```

Since everything is based on naming conventions, you are free to complicate
(or simplify) this example at will so that using `tst` fits best your
workflow.

You can also pass the wildcard as first argument:

```
tstrun -d test_newsuite '*_login_*' +Interactive +LinearScale
```
Remember to always include it in single quotes to avoid premature shell expansion.

[Top](#top)
