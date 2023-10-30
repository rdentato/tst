<img height="150" src="https://github.com/rdentato/tst/assets/48629/248f5856-13bd-4e35-8d9f-0b74a0ecb010"> <br/>
# tst
A minimalistic unit test framework for C (and C++). (Join us on [Discord](https://discord.gg/BqsZjDaUxg)!)

## Introduction
`tst` is a lightweight unit testing framework designed for C programs. It provides a suite of functionalities to define, group, and validate test cases, while offering utilities for expressive reporting and diagnostic messaging. With minimal syntax, `tst` fosters easy test integration into C projects.

I never understood why some of the most common unit test frameworks had so many files and complex build.
If you want to use `tst`, just include `tst.h` and you're ready to write your test cases.

## Core Functions Overview
1. **tstrun(title, ... )**
   - **Purpose**: Groups test cases, outputs the title and defines the group tags. Implies `main()`.
   - **Example**:
     ```c
     tstrun("Running tests", noDB, simple) {
       // Your test cases here...
     }
     ```
   
2. **tstcase( ... )**
   - **Purpose**: Defines a test case, with a formatted title.
   - **Example**:
     ```c
     tstcase("Testing equality of %d and %d", 1, 1) {
       // Your checks/assertions here...
     }
     ```
   
3. **tstcheck(int test, ... )**
   - **Purpose**: Validates whether `test` is true and outputs either "PASS" or "FAIL" alongside an optional message.
   - **Example**:
     ```c
     tstcheck(1 == 1, "Value mismatch! Expected equality.");
     ```
   
4. **tstassert(int test, ... )**
   - **Purpose**: Similar to `tstcheck` but halts program execution upon failure.
   - **Example**:
     ```c
     tstassert(1 == 1, "Critical: Value mismatch!");
     ```
   
5. **tstgroup(int test, ... )**
   - **Purpose**: Conditionally executes a block of checks; skips if `test` is false.
   - **Example**:
     ```c
     tstgroup(1 == 1) {
       tstcheck(2 == 2, "Secondary check failed!");
     }
     ```
   
6. **tstclock( ... )**
   - **Purpose**: Measures and displays elapsed time for specified instructions.
   - **Example**:
     ```c
     tstclock("Measuring time") {
       // Code block to measure...
     }
     ```
   
7. **tstdata( ... )**
   - **Purpose**: Attaches data blocks for enhanced test contextualization.
   - **Example**:
     ```c
     tstdata("Relevant test data", dataPointer);
     ```
   
8. **tstnote( ... )**
   - **Purpose**: Allows for the insertion of contextual notes within test output.
   - **Example**:
     ```c
     tstnote("Note: This test is pivotal for module X integrity.");
     ```
   
## Comprehensive Example
```c
#include "tst.h"  // Ensure the tst framework is included

void myData(FILE *f)
{
  fprintf(f,"\nMYDATA MYDATA MYDATA MYDATA MYDATA\n MYDATA MYDATA MYDATA MYDATA \n");
}

tstrun("Primary Test Suite")
{
    
    tstcase("Equality Checks %d, %d", 1, 1) {
      tstcheck(1 == 1, "Mismatch: %d != %d", 1, 1);
      tstcheck(1 != 1, "Failed on purpose");
    }
    
    tstcase("Time Complexity Analysis") {
      tstclock("Check counting time") {
        volatile int b = 1;
        // Code to analyze...
        for (int a = 1; a < 100 ; a++) b = a + b;
      }
    }
    
    tstcase("Grouped Checks: Edge Cases") {
      tstgroup(1 == 2, "Inequality" ) {  // Will be skipped!
        tstcheck(0 < 1, "0 should be less than 1");
        tstassert(1 >= 1, "1 should be equal to 1");
      }

      tstgroup(1 != 2,"Equality") {  // Will be executed!
        tstcheck(0 < 1, "0 should be less than 1");
        tstassert(1 >= 1, "1 should be equal to 1");
      }
    }
    
    tstdata("Useful Debug Data") {
       myData(stderr);
    }
    
    tstnote("Testing Complete. Review for any FAIL flags.");
}
```
## Test Results:
```
FILE ▷ tst_test.c "Primary Test Suite"
CASE┬── Equality Checks 1, 1 » tst_test.c:12
PASS│  1 == 1 » tst_test.c:13
FAIL├┬ 1 != 1 » tst_test.c:14
    │╰ Failed on purpose
    ╰── 1 KO | 1 OK | 0 SKIP
CASE┬── Time Complexity Analysis » tst_test.c:17
CLCK│⚑ 0.001000 ms. Check counting time » tst_test.c:18
    ╰── 0 KO | 0 OK | 0 SKIP
CASE┬── Grouped Checks: Edge Cases » tst_test.c:25
SKIP├┬ 1 == 2 » tst_test.c:26
    │╰ Inequality
PASS│  1 != 2 » tst_test.c:31
PASS│  0 < 1 » tst_test.c:32
PASS│  1 >= 1 » tst_test.c:33
    ╰── 0 KO | 2 OK | 1 SKIP
DATA│ ▽▽▽ Useful Debug Data » tst_test.c:37

MYDATA MYDATA MYDATA MYDATA MYDATA
 MYDATA MYDATA MYDATA MYDATA 
``````
DATA│ △△△
NOTE 🗎 Testing Complete. Review for any FAIL flags. » tst_test.c:41
RSLT ▷ 1 KO | 3 OK | 1 SKIP
```
## Temporary disabling
There are cases when you want to switch off a test case, a check, a group, and so on. For cases like these, you just add an underscore after `tst`. 
For example if we have this test case:
```
   tstcase ("Check for 0") {

   }
```
we can leave it out this way (has an underscore after `tst`):
```
   tst_case ("Check for 0") {

   }
```

Similarly:
```
   tstcheck(x<0,"Too small! %d", x);    // Check enabled
   tst_check(x==0,"Not zero! %d", x);   // Check disabled
```

You can also disable an entire test scenario changing `tstrun` into `tst_run()`.

This can be useful when you have test cases that you might no longer need to be executed all the time but still want to keep them in the test suite because they can be useful at a later stage (and you are against using too many `#ifdef` :) )


## Tagging and Grouping in `tst`: Selective Test Execution

In the `tst` framework, the ability to tag and group tests is a powerful feature that allows developers to selectively execute specific sets of tests. This is especially useful in scenarios where you might want to run only a subset of your tests, such as when you're working on a specific feature or debugging a particular module.

### How to Tag and Group Tests:

1. **Defining Tags**: 
   - Add the tags to the `tstrun` function at the beginning of your test file to define all the tags you plan to use.
   - **Example**:
     ```c
     tstrun("Title", Group1, Group2, Group3)
     ```
   
2. **Grouping Tests Using Tags**:
   - Within `tstrun()`, use the `tstgroup` function in combination with the `tsttag` function to conditionally run specific blocks of tests based on the tags that are active.
   - `tsttag(TagName)` returns a boolean value indicating whether a specific tag is active.
   - **Examples**:
     ```c
     tstrun("Title", Group1, Group2, Group3) {
       tstgroup(tsttag(Group2) || tsttag(Group3)) {
         // This block will run only if either Group2 or Group3 is enabled.
       }
     }
     ```

     ```c
     tstrun() {
       tstgroup(!tsttag(Group1) && tsttag(Group3)) {
         // This block will run only if Group1 is disabled and Group3 is enabled.
       }
     }
     ```

### Selectively Running Tests from the Command-Line:

When executing your tests from the command line, you can enable or disable specific groups of tests by referencing their tags.

- **Syntax**:
  - Use a `-` prefix to disable a group.
  - Use a `+` prefix to enable a group.
  - Use `-*` to disable all tagged tests.

- **Examples**:
  - To run only the tests tagged as "Group3", excluding tests tagged as "Group1" and "Group2":
    ```
    my_tests -Group1 -Group2
    ```
    
  - To disable all tagged tests:
    ```
    my_tests -*
    ```
    
  - To run only the tests tagged as "Group2":
    ```
    my_tests -* +Group2
    ```

  - To know which tags are defined:
    ```
    my_tests ?
    ```
If you are using the `makefile` provided in the `test` directory (which I reccomend to look at), you can drive the 
execution of groups of tests via the `TSTTAGS` environment variable. For example to exclude the group `NODB`:

  ```
    $ TSTTAGS=-NODB make -B runtest
  ```

### Full example:

```c
#include "tst.h"

tstrun("Grouped tests",NoDB, FileOnly, SimpleRun) {
  tstgroup(tsttag(NoDB) && !tsttag(SimpleRun)) {
     // Only if NoDB is enabled and SimpleRun is disabled.
  }
}
```

```bash
  $ my_tests ?
  ./test/t_tst01 [? | [+/-]tag ...]
  tags: NoDB FileOnly SimpleRun

  $ my_tests -* +FileOnly
  FILE ▷ t_tst01.c 
  SKIP├┬ tsttag(NoDB) && !tsttag(SimpleRun) » t_tst01.c:9
      │╰ 
  RSLT ▷ 0 KO | 0 OK | 1 SKIP
```    
### Benefits:

Tagging and grouping tests offer the flexibility to narrow down the testing focus, which can lead to quicker debugging and development cycles. This feature is especially advantageous in large projects with numerous test cases or in continuous integration environments where only a subset of tests might be relevant to run in certain scenarios.


## Integrating `tst` into Your Project: A Simple Guide

Integrating `tst` into your C project is straightforward. By organizing your test files and leveraging the provided `makefile`, you can seamlessly manage and execute your unit tests. Here's a step-by-step guide to doing this:

### 1. **Set Up a `test` Directory**

To keep your project organized, create a separate `test` directory within your project's root folder. This dedicated directory will house your unit test files, `tst.h` header, and the `makefile`.

```bash
mkdir test
```

### 2. **Copy Essential Files**

- **`tst.h`**: This is the main header file for the `tst` framework. Ensure this is present in the `test` directory to include it in your test files.
  
- **`makefile`**: The provided `makefile` will contain rules to compile and run your unit tests. Ensure this is also placed within the `test` directory.

```bash
cp path_to_tst_files/src/tst.h path_to_project/test/
cp path_to_tst_files/test/makefile path_to_project/test/
```

### 3. **Naming Convention for Test Files**

To ensure that the `makefile` correctly identifies unit test files:

- Name your test files with the prefix `t_`, followed by a descriptive name, and then the `.c` extension. For example: `t_mathFunctions.c`.

- By sticking to this naming convention, the `makefile` can easily detect which C files in the directory are intended as unit tests.

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

### Returning error

By default, if a test fails, it will return 1 to signal it as an error. This might be undesirable if the
tests are run in a script that could interepret this as signal to interrupt the execution.

You can avoid this by specifiying `=` as the first argument:

```bash
  $ t_test =
```
so that it will always return 0.

### Benefits

By setting up a dedicated `test` directory and leveraging the provided `makefile`, you can effortlessly manage and run unit tests using the `tst` framework. This structure not only ensures a clean project layout but also streamlines the testing process, making it easier for developers to maintain and expand upon their test suites.

## Filename path
By default only the source file name is printed in the log. For example:

```
PASS│  1 == 1 » t_tst00.c:16
```

If you want, instead, to have the full path printed, like in:
```
PASS│  1 == 1 » test/t_tst00.c:16
```

 you can define the `TSTFULLPATH` symbol before including `tst.h`:
```c
  #define TSTFULLPATH
  #include "tst.h"
```
or using `make`'s options:
```bash
  make TSTFULLPATH=1 test/t_tst00
```
or compiler's options:
```bash
  gcc -DTSTFULLPATH -o test/t_tst00 test/t_tst00.c
```

## Split tests
Usually the `tstcheck()` function is enough to handle the test results but there might be cases when you want to perform some more actions depending on the fact that the test passed or not.

For this there are three functions:

- `tst()` Just perform the test.
- `tstpassed()` Returns true if the previous test check (with `tst()` or `tstcheck()`) passed.
- `tstfailed()` Returns true if the previous test check (with `tst()` or `tstcheck()`) failed.

## Conclusion
`tst` offers a user-friendly syntax to facilitate streamlined testing without exhaustive setup or dependencies. Developers may swiftly integrate, run, and diagnose tests, ensuring the robustness and reliability of their C code.

**Note**: Always consider possible improvements or expansions to the tool to match the specific needs of your project, and feel free to contribute to its development a this is an open-source tool.


