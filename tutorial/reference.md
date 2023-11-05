# Test Library Reference Manual

## Overview

This reference manual covers the functions and variables provided by a C test library designed to facilitate automated testing of code. The library provides mechanisms to perform tests, check their status, print formatted messages, measure execution time, and manage test data.

## Functions

### `int tst(int test);`
Performs a test and returns its value.

#### Parameters
- `int test`: The test condition to evaluate.

#### Returns
- The result of the test.

### `int tstfailed();`
Returns true (non-zero) if the last test failed.

#### Returns
- Non-zero if the last test failed, zero otherwise.

### `int tstpassed();`
Returns true (non-zero) if the last test passed.

#### Returns
- Non-zero if the last test passed, zero otherwise.

### `int tstskipped();`
Returns true (non-zero) if the last test has been skipped.

#### Returns
- Non-zero if the last test was skipped, zero otherwise.

### `int tstcheck(int test [, char *fmt, ...]);`
Performs a test and prints a formatted message if it fails, similar to `printf()`.

#### Parameters
- `int test`: The test condition to evaluate.
- `const char *fmt`: Optional format string followed by arguments (variadic), used for printing a message if the test fails.

#### Returns
- The result of the test.

### `int tstassert(int test [, char *fmt, ...]);`
Same as `tstcheck` but aborts the program if the test fails.

#### Parameters
- `int test`: The test condition to evaluate.
- `const char *fmt`: Optional format string followed by arguments (variadic), used for printing a message if the test fails.

#### Returns
- The result of the test.

### `clock_t tstelapsed();`
Returns the clock ticks measured by the last `tstclock`.

#### Returns
- The number of clock ticks measured by the last execution of `tstclock`.

### `void tstnote(char *fmt,...);`
Prints a formatted note in the log.

#### Parameters
- `const char *fmt`: Format string followed by arguments (variadic), used for printing a note.

## Control structures

### `tstrun(char *title [, tag1, tag2, ...]) { ... }`
Defines the entry point for running tests and defines an optional set of tags up to eight.

#### Parameters
- `title`: Title of the test suite.
- `...`: Optional tags associated with the test suite (variadic).

### `tstskipif(int test) { ... }`
Skips the tests in its scope if the test evaluates to true.

#### Parameters
- `int test`: The condition that determines if tests should be skipped.

### `tstclock([char *fmt, ...]) { ... }`
Measures the time needed to execute the code in its scope. The clock resolution is system-dependent.

#### Parameters
- `fmt, ...`: A message that will be written in the log.

### `tstcase([char *fmt,...]) { ... }`
Collects the results (fail/pass/skipped) for the tests in its scope.

#### Parameters
- `fmt`: Format string followed by arguments (variadic), used for defining the test case.

### `void tstsection(const char *fmt,...);`
Executes a group of tests and restarts the test case count.

#### Parameters
- `const char *fmt`: Format string followed by arguments (variadic), used for defining the test section.

## Variables

### `some_type_t tstdata[]`
An array that provides data for the `tstsection`s.

#### Type
- `some_type_t[]`: Array of type `some_type_t` containing data for tests.

### `some_type_t tstcurdata`
Allows access to the current element of `tstdata`.

#### Type
- `some_type_t`: Type of the current data element being accessed.

