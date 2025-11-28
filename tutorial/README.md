<a id="top"></a>
# Tutorial

Following the examples of other testing frameworks, this tutorial will guide you through the setup
and the steps needed to use `tst`.

A word of caution: The functions offered here are simple but rather flexible and allow you to 
create very complicated and intricate test cases. Please don't! Besides its purpose of ensuring
the correctness of the system, the test suite also has great value as an example of how to use
the various functions. Try to write the tests as cleanly and simply as possible and you'll save 
the day for those who will have to understand the code later.

**Contents**<br>
[Setting tst Up](#setup)<br>
[A Minimal Example](#min-example)<br>
[Handling Failure](#failures)<br>
[Assertions](#assertions)<br>
[On the Expression to Check](#check-expression)<br>
[Structuring Tests Using `tstcase`](#tstcase)<br>
[Sections](#sections)<br>
[Data Driven Tests](#data-driven)<br>
[Conditional Test Execution (and Tagging)](#conditional)<br>
[Disabling Tests at Compile Time](#disabling)<br>
[Split Tests](#split-tests)<br>
[Checking Times](#clock)<br>
[Informational Output](#info-output)<br>
[Using --report-error for CI/CD Integration](#report-error)<br>
[Command Line Options](#command-line)<br>
[Advanced Example: Putting It All Together](#advanced-example)<br>
[Running Your Tests](#running-tests)<br>

<a id=setup></a>
## Integrating `tst` into Your Project

There is no constraint on which build system you use as `tst` is a single header library.
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

## A minimal example

Let's define a scenario for our examples: you have a library of functions defined in file `functions.c`
whose prototypes are in `functions.h` and you want to properly test them to ensure their correctness.

You'll find the example code in this `tutorial` directory.

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
all the checks you want to perform on the function:

```c
#include "tst.h"
#include "functions.h"

tstsuite("Factorials") {
  tstcase("Basic tests") {
    tstcheck(fact_0(1) == 1);
    tstcheck(fact_0(2) == 2);
    tstcheck(fact_0(3) == 6);
    tstcheck(fact_0(5) == 120);
  }
}
```

Once you compile and link it with the file where the `fact_0()` function is defined, you'll get
an executable, say `t_fact_0`, that when run will execute all the tests and report the results:

```
----- SUIT / t_fact_0.c "Factorials" 2025-11-27 12:00:00
    6 CASE,--Basic tests
    7 PASS|  fact_0(1) == 1
    8 PASS|  fact_0(2) == 2
    9 PASS|  fact_0(3) == 6
   10 PASS|  fact_0(5) == 120
    6     `--- 0 FAIL | 4 PASS | 0 SKIP
^^^^^ RSLT \ 0 FAIL | 4 PASS | 0 SKIP 2025-11-27 12:00:00
```
The idea is to have a single executable file which defines a "run" of tests that will cover a
logically related set of functions or will go over a specific use case.

The `tstsuite()` macro will generate `main()`: you don't need (and should not) define a `main()` function.

**Important**: Notice that all `tstcheck()` calls are wrapped inside a `tstcase()` block. This is required - 
you cannot use `tstcheck()` directly in a `tstsuite()` without a `tstcase()`. The `tstcase()` provides 
structure and allows the framework to group related tests and report partial results.

**Understanding the output**: The numbers on the left (6, 7, 8, 9, 10) are the line numbers in your test file 
where each check or case is located. This makes it easy to find which test failed when you need to debug.

<a id="failures"></a>

## Handling failures
If you looked at the `fact_0()` function in the previous section, you may have noticed that there is, actually, a bug in it.
We didn't detect it because we failed to check for one of the edge cases: 0! = 1.
Let's do it:

```c
#include "tst.h"
#include "functions.h"

tstsuite("Factorials") {
  tstcase("Edge case: 0") {
    tstcheck(fact_0(0) == 1); // Test edge case
  }
  
  tstcase("Basic tests") {
    tstcheck(fact_0(1) == 1);
    tstcheck(fact_0(2) == 2);
    tstcheck(fact_0(3) == 6);
    tstcheck(fact_0(5) == 120);
  }
}
```
We would have got:
```
----- SUIT / t_fact_0_err.c "Factorials" 2025-11-27 12:00:00
    5 CASE,--Edge case: 0
    6 FAIL|  fact_0(0) == 1
    5     `--- 1 FAIL | 0 PASS | 0 SKIP
    9 CASE,--Basic tests
   10 PASS|  fact_0(1) == 1
   11 PASS|  fact_0(2) == 2
   12 PASS|  fact_0(3) == 6
   13 PASS|  fact_0(5) == 120
    9     `--- 0 FAIL | 4 PASS | 0 SKIP
^^^^^ RSLT \ 1 FAIL | 4 PASS | 0 SKIP 2025-11-27 12:00:00
```
Note how failures are reported as the first number in the *results* line. That's because,
most probably, the first thing we want to know is if everything went right.

When a check fails, we might want to print a message to better understand what went wrong.
We can do it as follows:

```C
tstsuite("Factorials") {
  tstcase("Edge case: 0") {
    tstcheck(fact_0(0) == 1, "Expected 1 got %d", fact_0(0));
  }
}
```
and we would have got:
```
----- SUIT / t_fact_0_err.c "Factorials" 2025-11-27 12:00:00
    5 CASE,--Edge case: 0
    6 FAIL|  fact_0(0) == 1 "Expected 1 got 0"
    5     `--- 1 FAIL | 0 PASS | 0 SKIP
^^^^^ RSLT \ 1 FAIL | 0 PASS | 0 SKIP 2025-11-27 12:00:00
```

I find it bothersome to specify a message for every check. After all, most of the time
the checks will always pass. I usually only add messages when a check fails and it's
not obvious why.
Of course, when the message is there I'd leave it for the next run; there's no need
to remove it.

<a id="assertions"></a>

## Assertions
Assertions are a stronger form of checking. For example, if the following test fails:

```C
tstassert(ptr = malloc(n),"Out of memory (requested: %d)",n);
```
the program will be aborted as there's little meaning in continuing to test when the memory is exhausted.

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

Within a test run, which is supposed to cover a logically meaningful scenario, you 
may want to define multiple *test cases* whose checks are tightly related.

The usefulness of `tstcase` is that it collects partial results and will allow you
to focus on groups of tests rather than having to consider all the tests at once.

For example, let's write a full test run for a more complete version of the factorial:

```c
#include "tst.h"
#include "functions.h"

tstsuite("Check Factorial") {
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
----- SUIT ▷ t_fact.c "Check Factorial"
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
You define an array named `tstdata` within your `tstcase` and access the current data
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
The `"First check"` section will be executed for each element of the `tstdata` array,
and then the `"Second check"` section will be executed for each element of the array.

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
exclude them for certain runs. You can tag individual test cases and then control
which ones run from the command line.

### Tagging Test Cases

You add tags directly to `tstcase()` declarations using `+tag` for positive tags (opt-in tests)
and `-tag` for negative tags (opt-out tests):

```C
#include "tst.h"

tstsuite("Database Tests") {
  tstcase("Quick validation") {
    // This test always runs (no tags)
    tstcheck(validate_config());
  }
  
  tstcase("Full database test", +database) {
    // This test only runs when you specify +database
    tstcheck(db_connect() != NULL);
    tstcheck(db_query("SELECT 1") == 1);
  }
  
  tstcase("Slow performance test", +slow, +database) {
    // This test needs both +slow and +database to run
    tstcheck(run_long_query() < 1000);
  }
  
  tstcase("Interactive test", -ci) {
    // This test runs by default but is skipped in CI
    tstcheck(prompt_user() == OK);
  }
}
```

### Tag Behavior Rules

1. **Untagged tests** always run regardless of command-line filters
2. **Any tagged test** (whether `+tag` or `-tag`) is **skipped by default** without command-line filters
3. **Command-line filters activate tagged tests:**
   - `+tag`: Enables tests with matching `+tag`
   - `-tag`: Enables tests with matching `-tag`  
   - `+*`: Enables all tests with any `+tag` (but not `-tag` tests)
4. **Multiple tags on a test:** Test runs if ANY tag matches the filter

### Command-Line Tag Filtering

Control which tests run using command-line arguments:

```bash
# Run only untagged tests (tagged tests are skipped by default)
./mytest

# Enable tests tagged with +database
./mytest +database

# Enable all tests with positive tags (+tag)
./mytest +*

# Enable tests tagged with -ci
./mytest -ci

# Combine filters: enable database tests AND ci-incompatible tests
./mytest +database -ci

# Enable all positive tags except slow ones
./mytest +* -slow
```

**Important**: When you provide ANY filter on the command line, only tests matching that filter
(plus untagged tests) will run. This is why `-tag` tests need `-tag` on the command line to run.

### Listing Available Tests and Tags

Use `--list` to see all test cases and their tags:

```bash
$ ./mytest --list
"Quick validation"
"Full database test" +database
"Slow performance test" +slow, +database
"Interactive test" -ci
```

This helps you understand which tests are available and how to filter them.

### Tag Filtering Examples

Given the test suite above, here's what runs with different filters:

| Command | Runs |
|---------|------|
| `./mytest` | "Quick validation" only (all tagged tests skipped) |
| `./mytest +database` | "Quick validation", "Full database test", "Slow performance test" |
| `./mytest +*` | "Quick validation", "Full database test", "Slow performance test" |
| `./mytest -ci` | "Quick validation", "Interactive test" |
| `./mytest +database -ci` | "Quick validation", "Full database test", "Slow performance test", "Interactive test" |

**Key insight**: Think of tags as requiring "activation" from the command line. By default, only
untagged tests run. You use command-line filters to activate specific groups of tagged tests.

### Practical Use Cases

**Quick smoke tests (default - untagged tests only):**
```bash
# Runs only fast, essential tests with no tags
./mytest
```

**Development workflow:**
```bash
# Run full test suite including optional tests
./mytest +*

# Database tests only
./mytest +database

# Everything except slow tests
./mytest +* -slow
```

**CI/CD pipeline:**
```bash
# Skip interactive/manual tests in automated environments
./mytest +* -manual

# Run only integration tests
./mytest +integration

# Full suite excluding platform-specific tests
./mytest +* -windows
```

**Debugging specific subsystem:**
```bash
# Focus on network tests
./mytest +network

# Database and network, but not slow tests
./mytest +database +network -slow
```

### Best Practices for Tags

1. **Use positive tags (`+tag`) for:**
   - Optional tests that shouldn't run by default (e.g., `+slow`, `+database`, `+network`)
   - Tests requiring external resources
   - Platform-specific tests (e.g., `+windows`, `+linux`)
   - Deep/exhaustive tests (e.g., `+full`, `+stress`)

2. **Use negative tags (`-tag`) for:**
   - Tests you want to explicitly exclude in certain environments (e.g., `-ci`, `-manual`)
   - Tests that conflict with automation
   - Experimental or unstable tests (e.g., `-experimental`, `-flaky`)

3. **Design your tag strategy:**
   ```c
   // Untagged = core smoke tests (always run)
   tstcase("Basic validation") { ... }
   
   // +tag = opt-in (run when explicitly requested)
   tstcase("Comprehensive test", +full) { ... }
   
   // -tag = opt-out (skip when explicitly excluded)
   tstcase("Manual verification", -automated) { ... }
   ```

4. **Keep tag names simple and meaningful:**
   - Good: `slow`, `database`, `network`, `ci`, `manual`, `windows`, `linux`
   - Avoid: `test1`, `group_a`, `temp`, `foo`

5. **Document your tags:**
   Add a comment at the top of your test file:
   ```c
   // Tags used in this file:
   //   +database : Tests requiring database connection
   //   +slow     : Tests taking >1 second
   //   -ci       : Tests incompatible with CI (interactive, etc.)
   ```
<a id="disabling"></a>

## Disabling tests at compile time

There are cases when you may want to remove some test cases from your test suite
but you do not want to remove them from the code because they might be useful 
later.

A typical example is if some feature is undergoing some major rewriting that would
make your tests useless until all the new code is completed.

Another one is when you want to focus on certain tests for debugging purposes and
want to create a smaller log to make it easier to understand what went wrong.

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

You can also disable an entire test scenario changing `tstsuite` into `tst_suite()`.


<a id="split-tests"></a>

## Split tests
Usually the `tstcheck()` function is enough to handle the test results, but there might be cases when you want to perform additional actions depending on whether the test passed or not.

For this there are the following functions:

- `tst()` Just perform the test.
- `tstpassed()` Returns true if the previous test check (with `tst()` or `tstcheck()`) passed.
- `tstfailed()` Returns true if the previous test check (with `tst()` or `tstcheck()`) failed.
- `tstskipped()` Returns true if the previous test check (with `tst()` or `tstcheck()`) had been skipped.

Example:
```C
   tstcheck(x != 3); // Perform the test
   if (tstpassed()) {
      // Do something specific on pass
      printf("Test passed, x = %d\n", x);
   }

   tst(z > 0); // Perform the test but does not report it in the log
   if (tstfailed()) {
      // Handle the failure
      cleanup_resources();
   }
```
Note that `tstpassed()` and `tstfailed()` report the result of the latest check.

**Tip**: Use `tst()` instead of `tstcheck()` when you want to test a condition but handle 
the result manually without automatic logging. This is useful for complex conditional logic.

<a id="clock"></a>
## Checking times

It may be useful, sometimes, to get an idea of how much time is spent in one particular piece of code.
For example, you may want to understand which implementation of a given function performs better.
The `tstclock()` macro can help you by measuring the processor time spent between the start and
the end of a block of code.

Here's an example to check that the recursive implementation of the factorial is slower than the 
iterative implementation:

```C
#include "tst.h"
#include "functions.h"

tstsuite("Check Factorial Speed")
{
  clock_t recursive_elapsed = 0;
  clock_t iterative_elapsed = 0;

  int recursive_result = 0;
  int iterative_result = 0;

  const int times = 100000;

  tstclock("Recursive") {
    for (int k=0; k<times; k++)
      recursive_result = fact_recursive(12);
  }
  recursive_elapsed = tstelapsed();
  tstcheck(recursive_result != 0,"Expect non 0 got: %d", recursive_result);

  tstclock("Iterative") {
    for (int k=0; k<times; k++)
      iterative_result = fact_iterative(12);
  }
  iterative_elapsed = tstelapsed();
  tstcheck(iterative_result != 0);

  tstcase("Check performance") {
    tstcheck(recursive_result  == iterative_result);
    tstcheck(recursive_elapsed >= iterative_elapsed, 
             "Recursive (%ld) should be slower than iterative (%ld)", 
             recursive_elapsed, iterative_elapsed);
  }
}
```

The `tstclock()` block will automatically print the elapsed time when the block ends.
You can also retrieve the elapsed time using `tstelapsed()` to perform additional checks or comparisons.

**Note**: The timing uses `clock()` from the standard library, which measures CPU time, not wall-clock time.
The time unit displayed (nanoseconds, microseconds, or milliseconds) is automatically determined based on
`CLOCKS_PER_SEC`.

<a id="info-output"></a>
## Informational Output

While running tests, you may want to print additional information to help with debugging or to document
what the test is doing. `tst` provides several macros for this purpose.

### `tstnote` - Simple Notes

Use `tstnote()` to print informational messages during test execution:

```C
tstnote("Testing Complete. Review for any FAIL flags.");
tstnote("Checking data: n=%d, s=%s", data.n, data.s);
```

This is useful for adding context to your tests without affecting the test results.

### `tstprintf` - Direct Output

For more control, you can use `tstprintf()` which is equivalent to `fprintf(stderr, ...)`:

```C
tstprintf("Debug: x=%d, y=%d\n", x, y);
```

### `tstouterr` - Delimited Output Blocks

Use `tstouterr()` to print a block of output with clear delimiters (`<<<<<` and `>>>>>`):

```C
tstouterr("Generated data:") {
  for (int k=0; k<4; k++) {
    tstdata[k] = rand() & 0x0F;
    tstprintf("[%d] = %d\n", k, tstdata[k]);
  }
}
```

This is particularly useful when you want to capture multi-line output for later analysis.

### `tstexpect` - Silent Passing Checks

Sometimes you have many checks where you only care about failures. Use `tstexpect()` instead of `tstcheck()`:

```C
// This will only print output if it fails
tstexpect(x > 0, "Expected positive value, got %d", x);

// Compare with tstcheck which always prints
tstcheck(y > 0, "Expected positive value, got %d", y);
```

The `tstexpect()` macro still counts PASS/FAIL/SKIP like `tstcheck()`, but only produces output on failure.
This keeps your test logs cleaner when you have hundreds or thousands of assertions.

<a id="report-error"></a>
## Using `--report-error` for CI/CD Integration

By default, TST test programs always return exit code `0`, even when tests fail. This design
choice prevents a single failing test from stopping an entire chain of tests when running
multiple test suites in sequence.

However, in automated environments like CI/CD pipelines, you typically want the build to **fail**
when tests fail. The `--report-error` flag changes the exit behavior to return a non-zero exit
code when failures occur.

### Exit Code Behavior

| Scenario | Default (no flag) | With `--report-error` |
|----------|-------------------|----------------------|
| All tests pass | Exit 0 | Exit 0 |
| One or more tests fail | Exit 0 | Exit 1 |
| Tests skipped (no failures) | Exit 0 | Exit 0 |

### Basic Usage

```bash
# Default behavior - always exits 0
$ ./mytest
^^^^^ RSLT \ 2 FAIL | 5 PASS | 0 SKIP 2025-11-27 12:00:00
$ echo $?
0

# With --report-error - exits 1 on failure
$ ./mytest --report-error
^^^^^ RSLT \ 2 FAIL | 5 PASS | 0 SKIP 2025-11-27 12:00:00
$ echo $?
1

# No failures - exits 0 even with flag
$ ./mytest --report-error
^^^^^ RSLT \ 0 FAIL | 5 PASS | 0 SKIP 2025-11-27 12:00:00
$ echo $?
0
```

### CI/CD Integration Examples

#### GitHub Actions

```yaml
name: Run Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Build tests
        run: |
          cd test
          make all
      
      - name: Run test suite
        run: |
          cd test
          ./t_mytest --report-error
      
      # This step only runs if tests pass
      - name: Deploy
        if: success()
        run: echo "Tests passed, deploying..."
```

#### GitLab CI

```yaml
test:
  stage: test
  script:
    - cd test && make all
    - ./t_mytest --report-error --color
  artifacts:
    when: always
    paths:
      - test/test.log
```

#### Jenkins

```groovy
pipeline {
    agent any
    stages {
        stage('Test') {
            steps {
                sh '''
                    cd test
                    make all
                    ./t_mytest --report-error
                '''
            }
        }
    }
    post {
        always {
            archiveArtifacts artifacts: 'test/*.log'
        }
    }
}
```

#### Travis CI

```yaml
language: c
compiler: gcc

script:
  - make test
  - cd test && ./t_mytest --report-error
```

### Combining with Tag Filters

The `--report-error` flag works seamlessly with tag filtering:

```bash
# Run only fast tests in CI, fail on error
./mytest -slow --report-error

# Run database tests, fail on error
./mytest +database --report-error

# Run all tests except manual ones, fail on error  
./mytest +* -manual --report-error
```

### Makefile Integration

Add `--report-error` to your test targets:

```makefile
# Development - don't stop on failures
test:
	cd test && ./runtest

# CI target - fail on any test failure
test-ci:
	cd test && ./runtest --report-error

# Quick smoke tests for CI
test-quick:
	cd test && ./runtest -slow --report-error

# Full test suite for nightly builds
test-full:
	cd test && ./runtest +* --report-error --color
```

Then in your CI configuration:

```bash
# Fast feedback during development
make test

# Strict validation in CI
make test-ci
```

### Shell Script Integration

Use `--report-error` in test runner scripts:

```bash
#!/bin/bash
# run_tests.sh

FAILED=0

echo "Running unit tests..."
./t_unit --report-error || FAILED=1

echo "Running integration tests..."
./t_integration +database --report-error || FAILED=1

echo "Running performance tests..."
./t_performance +slow --report-error || FAILED=1

if [ $FAILED -eq 1 ]; then
    echo "❌ Some tests failed"
    exit 1
else
    echo "✅ All tests passed"
    exit 0
fi
```

### Docker Integration

```dockerfile
FROM gcc:latest

WORKDIR /app
COPY . .

RUN cd test && make all

# Run tests as part of build - will fail build if tests fail
RUN cd test && ./t_mytest --report-error

# Or as healthcheck
HEALTHCHECK CMD cd test && ./t_mytest --report-error || exit 1
```

### Pre-commit Hook Integration

Use `--report-error` in git hooks to prevent commits with failing tests:

```bash
#!/bin/bash
# .git/hooks/pre-commit

echo "Running tests before commit..."

cd test && make all
./t_mytest --report-error

if [ $? -ne 0 ]; then
    echo "❌ Tests failed. Commit aborted."
    echo "Fix failing tests or use 'git commit --no-verify' to skip."
    exit 1
fi

echo "✅ Tests passed. Proceeding with commit."
exit 0
```

### Debugging CI Failures

When a test fails in CI with `--report-error`, you get:

1. **Full test output** showing which tests failed
2. **Non-zero exit code** that stops the pipeline
3. **Exact line numbers** of failing assertions

Example CI output:

```
----- SUIT / t_database.c "Database Tests" 2025-11-27 14:32:10
   15 CASE,--Connection tests
   17 PASS|  db_connect() != NULL
   18 FAIL|  db_ping() == 0 "Connection timeout"
   15     `--- 1 FAIL | 1 PASS | 0 SKIP
^^^^^ RSLT \ 1 FAIL | 1 PASS | 0 SKIP 2025-11-27 14:32:11

Error: Process completed with exit code 1.
```

You can immediately see:
- Which test file failed (`t_database.c`)
- Which test case failed ("Connection tests")  
- Exact line number (`18`)
- What failed (`db_ping() == 0`)
- Why it failed ("Connection timeout")

### Best Practices

1. **Always use in CI/CD pipelines:**
   ```bash
   ./mytest --report-error    # CI builds
   ```

2. **Optional during development:**
   ```bash
   ./mytest                   # Keep working on other tests
   ```

3. **Combine with environment variables:**
   ```bash
   # .bashrc or .zshrc
   export TSTOPTIONS="--color"
   
   # CI configuration
   export TSTOPTIONS="--color --report-error"
   ```

4. **Document in README:**
   ```markdown
   ## Running Tests
   
   Development:
   ```bash
   make test
   ```
   
   CI (strict mode):
   ```bash
   make test-ci    # Uses --report-error
   ```
   ```

5. **Use with appropriate tag filters:**
   ```bash
   # CI quick check - skip slow tests
   ./mytest -slow --report-error
   
   # Nightly build - run everything
   ./mytest +* --report-error
   ```

### Troubleshooting

**Problem:** CI passes but tests actually failed

**Solution:** Make sure you're using `--report-error`:
```bash
# Wrong - CI won't fail
./mytest

# Correct - CI will fail on test failures
./mytest --report-error
```

**Problem:** Want different behavior for different test suites

**Solution:** Use separate commands:
```bash
# Unit tests - strict
./t_unit --report-error

# Experimental tests - don't fail build
./t_experimental
```

**Problem:** Tests pass locally but fail in CI

**Solution:** Run locally with same flags:
```bash
# Replicate CI environment
./mytest +* --report-error

# Or use CI tag
./mytest -ci --report-error
```

<a id="command-line"></a>
## Command line options

When a `tstsuite` is compiled, it creates an executable that accepts several command-line options
to control test execution and output formatting.

### Listing Tests and Tags

Use `--list` to see all test cases in your suite along with their tags:

```bash
$ ./mytest --list
"Quick validation"
"Full database test" +database
"Slow performance test" +slow, +database
"Interactive test" -ci
```

This shows you:
- The name of each test case
- Any tags associated with each test (see [Conditional test execution](#conditional) for details)

This is useful for:
- Understanding what tests are available
- Seeing which tags you can use for filtering
- Documenting your test suite

### Returning Errors on Test Failure

By default, test programs return exit code `0` even when tests fail. This prevents a single failing
test from stopping an entire chain of tests in a script.

However, in CI/CD pipelines, you typically want the build to fail when tests fail. Use the 
`--report-error` flag to make the test program return exit code `1` if any test fails:

```bash
# Returns 0 even if tests fail (default)
$ ./mytest
^^^^^ RSLT \ 2 FAIL | 5 PASS | 0 SKIP
$ echo $?
0

# Returns 1 if any test fails
$ ./mytest --report-error
^^^^^ RSLT \ 2 FAIL | 5 PASS | 0 SKIP
$ echo $?
1
```

**CI/CD example:**
```yaml
# GitHub Actions
- name: Run tests
  run: ./mytest --report-error

# Jenkins, GitLab CI, etc.
script:
  - make test ARGS="--report-error"
```

The `--report-error` flag can be combined with tag filters:
```bash
$ ./mytest +database --report-error    # Run database tests, fail on errors
$ ./mytest +* -slow --report-error     # Run all but slow tests, fail on errors
```

### Tag Filtering

Control which tests run using `+tag` and `-tag` command-line arguments:

```bash
# Run tests tagged with +database
$ ./mytest +database

# Enable all tests with positive tags (+tag)
$ ./mytest +*

# Run tests tagged with -ci (opt-out tests)
$ ./mytest -ci

# Combine filters
$ ./mytest +database -ci    # Database tests, but skip CI-incompatible ones
```

See the [Conditional test execution](#conditional) section for complete documentation on tagging.

### Colored Output

Enable colored output using the `--color` option:

```bash
$ ./mytest --color
```

This will display:
- **Red**: Number of failed tests
- **Green**: Number of passed tests  
- **Yellow**: Number of skipped tests

Colors are off by default. You can make them the default by setting the `TSTOPTIONS` 
environment variable (see below).

### Setting Default Options

Use the `TSTOPTIONS` environment variable to set default command-line arguments:

```bash
# Always use colors and enable database tests
export TSTOPTIONS="--color +database"

# Now running ./mytest uses these defaults
$ ./mytest
```

You can override environment defaults by specifying different options:

```bash
$ TSTOPTIONS="--color +database"
$ ./mytest -database    # Override: disable database tests
```

### Combining Options

All command-line options can be combined in any order:

```bash
# Run all tests with colors and fail on error
$ ./mytest +* --color --report-error

# List tests (ignores other options)
$ ./mytest --list

# Complex CI scenario
$ ./mytest +integration -slow -manual --report-error --color
```

<a id="running-tests"></a>

## Running your tests

There is no limitation on how you organize and run your tests. Once you have compiled
the test program, you can launch it on its own or add to a CI pipeline or whatever is 
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
  -c | --color                turns on/off coloured messages
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

<a id="advanced-example"></a>

## Advanced Example: Putting It All Together

The `t_advanced_example.c` file demonstrates how multiple TST features work 
together in a realistic testing scenario. It tests a simple key-value store 
implementation and shows you how to structure a comprehensive test suite.

### What the Example Demonstrates

The example includes:

- **Multiple test cases** - Each testing a specific aspect of functionality
- **Tag-based organization** - `+slow`, `+stress`, `+valgrind`, `-ci` tags
- **Data-driven testing** - Using `tstdata` arrays for parameterized tests
- **Setup/teardown patterns** - Using `tstsection()` for organized test structure
- **Performance measurement** - Timing operations with `tstclock()` and `tstelapsed()`
- **Conditional execution** - Skipping tests with `tstskipif()`
- **Error messages** - Clear, formatted diagnostic messages
- **Memory management** - Proper cleanup and leak detection patterns

### The Key-Value Store Implementation

The example implements a simple hash-free key-value store (~200 lines):

```c
typedef struct {
    char* key;
    char* value;
} KVPair;

typedef struct {
    KVPair* pairs;
    int count;
    int capacity;
} KVStore;

KVStore* kv_create(int capacity);
void kv_destroy(KVStore* store);
int kv_set(KVStore* store, const char* key, const char* value);
const char* kv_get(KVStore* store, const char* key);
int kv_delete(KVStore* store, const char* key);
```

This is intentionally simple but realistic enough to demonstrate comprehensive
testing practices.

### Test Case Breakdown

#### 1. Basic Functionality (Always Runs)

```c
tstcase("Store creation and destruction") {
    KVStore* store = NULL;
    
    tstsection("Create store with valid capacity") {
        store = kv_create(10);
        tstcheck(store != NULL, "Store should be created");
        tstcheck(store->count == 0, "New store should be empty");
        tstcheck(store->capacity == 10, "Capacity should be 10, got %d", 
                 store->capacity);
    }
    
    tstsection("Create store with zero capacity") {
        store = kv_create(0);
        tstcheck(store != NULL, "Should handle zero capacity");
        if (store) tstcheck(store->capacity == 0);
    }
    
    if (store) kv_destroy(store);
}
```

**Key features:**
- Uses `tstsection()` to organize related checks
- Format strings in `tstcheck()` provide diagnostic values
- Cleanup happens after all sections complete
- Tests both normal and edge cases

#### 2. Data-Driven Testing

```c
tstcase("Data-driven test: Multiple key-value pairs") {
    KVStore* store = kv_create(10);
    tstassert(store != NULL);
    
    struct { const char* key; const char* value; } tstdata[] = {
        {"username", "alice123"},
        {"email", "alice@example.com"},
        {"age", "25"},
        {"city", "New York"},
        {"country", "USA"}
    };
    
    tstsection("Insert and verify each pair") {
        tstnote("Testing key='%s', value='%s'", 
                tstcurdata.key, tstcurdata.value);
        
        // Insert
        tstcheck(kv_set(store, tstcurdata.key, tstcurdata.value) == 0,
                 "Failed to insert key='%s'", tstcurdata.key);
        
        // Immediately verify
        const char* retrieved = kv_get(store, tstcurdata.key);
        tstcheck(retrieved != NULL, "Key '%s' not found", tstcurdata.key);
        tstcheck(strcmp(retrieved, tstcurdata.value) == 0,
                 "Expected '%s', got '%s'", tstcurdata.value, retrieved);
    }
    
    // After all data items processed, verify count
    tstcheck(store->count == 5, "Expected 5 items, got %d", store->count);
    
    kv_destroy(store);
}
```

**Key features:**
- Define test data as an array named `tstdata`
- Each section runs once per data element
- Access current data via `tstcurdata.field_name`
- Use `tstnote()` to document which data is being tested
- Code after sections runs once, after all data processed

#### 3. Performance Testing with Tags

```c
tstcase("Performance test: Large dataset", +slow) {
    const int N = 1000;
    KVStore* store = kv_create(N);
    tstassert(store != NULL, "Failed to create large store");
    
    clock_t elapsed;
    
    tstclock("Insert %d items", N) {
        for (int i = 0; i < N; i++) {
            char key[32], value[32];
            sprintf(key, "key_%d", i);
            sprintf(value, "value_%d", i);
            tstassert(kv_set(store, key, value) == 0);
        }
    }
    elapsed = tstelapsed();
    
    tstcheck(store->count == N, "Expected %d items, got %d", N, store->count);
    tstnote("Insert rate: %.2f items/ms", (double)N / elapsed);
    
    kv_destroy(store);
}
```

**Key features:**
- Tagged with `+slow` - skipped by default, run with `./t_advanced_example +slow`
- Uses `tstclock()` to time operations
- Gets elapsed time with `tstelapsed()`
- Reports performance metrics with `tstnote()`
- Uses `tstassert()` inside loops for early exit on failure

#### 4. Conditional Execution

```c
tstcase("Stress test: Maximum capacity", +stress, -ci) {
    const int MAX = 10000;
    KVStore* store = NULL;
    
    store = kv_create(MAX);
    tstskipif(store == NULL) {
        tstnote("Testing with %d capacity store", MAX);
        
        // Fill to capacity
        for (int i = 0; i < MAX; i++) {
            char key[32], value[64];
            sprintf(key, "k%d", i);
            sprintf(value, "This is a longer value for key %d", i);
            tstcheck(kv_set(store, key, value) == 0,
                     "Failed at item %d", i);
        }
        
        tstcheck(store->count == MAX, "Expected full capacity");
        tstnote("Successfully tested %d items", MAX);
    }
    
    if (store) kv_destroy(store);
}
```

**Key features:**
- Tagged `+stress` and `-ci` (excluded from CI by default)
- Uses `tstskipif()` to handle allocation failures gracefully
- Block after `tstskipif()` runs only if condition is false
- Reports progress with `tstnote()`
- Handles resource cleanup in all cases

### Running the Advanced Example

**Build and run all tests:**
```bash
cd tutorial
make t_advanced_example
./t_advanced_example
```

This runs only the basic tests (untagged). The output shows:
```
  • Store creation and destruction
  • Basic operations
  • Edge cases and error handling
  • Data-driven test: Multiple key-value pairs
```

**Run with slow tests:**
```bash
./t_advanced_example +slow
```

Adds the performance test to the run:
```
  • Performance test: Large dataset
    ○ Insert 1000 items ... 5 ms
    Insert rate: 200.00 items/ms
    ○ Retrieve 1000 items ... 3 ms
    Retrieval rate: 333.33 items/ms
```

**Run only stress tests:**
```bash
./t_advanced_example +stress
```

**Run everything except CI-excluded tests:**
```bash
./t_advanced_example +*
```

This runs all tests with `+tag`, but still skips tests with `-ci`.

**Run memory leak detection:**
```bash
valgrind --leak-check=full ./t_advanced_example +valgrind
```

The `+valgrind` tagged test creates and destroys 100 stores to exercise
memory management.

### Testing Patterns Demonstrated

#### Setup/Teardown Pattern

```c
tstcase("My test") {
    Resource* res = create_resource();
    tstassert(res != NULL);  // Abort if setup fails
    
    tstsection("Test part 1") {
        // Tests using res
    }
    
    tstsection("Test part 2") {
        // More tests using res
    }
    
    // Cleanup runs after all sections
    cleanup_resource(res);
}
```

#### Error Context Pattern

```c
tstcheck(condition, "Expected %s, got %s (index=%d)", 
         expected, actual, i);
```

Always include enough context to diagnose failures without debugging.

#### Early Exit Pattern

```c
tstassert(critical_operation() == 0, "Setup failed");
// Only continue if critical operation succeeded
```

Use `tstassert()` when continuing would cause crashes or meaningless results.

#### Resource Allocation Pattern

```c
Resource* res = allocate();
tstskipif(res == NULL) {
    // Tests that need the resource
}
if (res) cleanup(res);  // Always cleanup if allocated
```

Handles allocation failures gracefully without aborting the entire test run.

### Key Takeaways

1. **Structure matters** - Use sections to organize related checks
2. **Tags enable flexibility** - Slow/stress/platform-specific tests can be optional
3. **Data-driven tests reduce duplication** - One test, multiple inputs
4. **Timing is easy** - `tstclock()` and `tstelapsed()` for performance checks
5. **Error messages are critical** - Include actual values and context
6. **Cleanup is important** - Always free resources, even in test code
7. **Skip gracefully** - `tstskipif()` for conditions outside your control

The complete example is in `t_advanced_example.c` (~300 lines including the 
implementation). Study it to see how these patterns work together in a 
realistic test suite.

[Top](#top)
