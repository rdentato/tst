# tst.h

- [tst.h](#tsth)
  - [Overview](#overview)
  - [Suite Definition](#suite-definition)
    - [`tstsuite` / `tst_suite`](#tstsuite--tst_suite)
  - [Test Cases and Sections](#test-cases-and-sections)
    - [`tstcase`](#tstcase)
    - [`tstsection`](#tstsection)
  - [Assertions and Checks](#assertions-and-checks)
    - [`tstcheck`](#tstcheck)
    - [`tstassert`](#tstassert)
  - [Conditional Execution](#conditional-execution)
    - [`tstskipif`](#tstskipif)
    - [Tags: `tsttag`](#tags-tsttag)
  - [Test State Queries](#test-state-queries)
  - [Timing and Notes](#timing-and-notes)
    - [`tstclock`](#tstclock)
    - [`tstelapsed`](#tstelapsed)
    - [`tstnote`](#tstnote)
    - [`tstouterr`](#tstouterr)
  - [Data-Driven Tests](#data-driven-tests)
  - [Command-Line Options](#command-line-options)


## Overview

`tst.h` is a single-header C testing framework that lets you define and run test suites with minimal boilerplate. It provides:

- **Test suites** and **cases**  
- **Assertions** and **checks**  
- **Conditional skips** and **tag-based filtering**  
- **Sections** and **data-driven tests**  
- **Timing**
- **Notes**  
- **Formatted output** with optional ANSI colors  

All macros and functions are defined in `tst.h`. Below is a reference for the public API.

---

## Suite Definition

### `tstsuite` / `tst_suite`

```c
tstsuite(title, [Tag1, Tag2, …]);
tst_suite(title, [Tag1, Tag2, …]); // **disabled** (compile-time skip)  
```

- **Purpose**: Declares the test suite and prints suite header.  To be used in place of `mani()`
- **Parameters**:  
  - `title` – string shown as the suite name.  
  - Optional up to eight tag identifiers for conditional execution.  
- **Behavior**:  
  - `tstsuite` → suite **enabled**  
  - `tst_suite` → suite **disabled** (compile-time skip)  
- **Example**:
  ```c
  tstsuite("Math Library Tests", Slow, DB);
  ```


---

## Test Cases and Sections

### `tstcase`

```c
tstcase(description);
tst_case(...); // **disabled** (compile-time skip)  
```

- **Purpose**: Starts a new test case within the suite.  
- **Parameters**:  
  - `description` – textual label for grouping related checks.  
- **Behavior**:  
  - Prints a “CASE” header.  
  - Reports partial PASS/FAIL/SKIP counts at the end of the block.  
  - `tst_case` → case **disabled** (compile-time skip)  
- **Example**:
  ```c
  tstcase("Edge Conditions") {
    tstcheck(x == 0);
    tstcheck(y != NULL);
  }
  ```

---

### `tstsection`

```c
tstsection(description);
tst_section(description); // **disabled** (compile-time skip)  
```

- **Purpose**: Defines a subsection inside a `tstcase`, isolating a subset of checks.  
- **Parameters**:  
  - `description` – label printed before the section’s checks.  
- **Behavior**:  
  - Runs setup code before each section.  
  - Supports iterating over a `tstdata` array for data-driven tests.  
- **Example**:
  ```c
  int a = 5;
  tstcase("Value Updates") {
    tstsection("Set to 9") {
      tstcheck(a == 5);
      a = 9;
      tstcheck(a == 9);
    }
    tstsection("Set to 8") {
      a = 8;
      tstcheck(a == 8);
    }
  }
  ```


---

## Assertions and Checks

### `tstcheck`

```c
tstcheck(expr, fmt, ...);
tst_check(...); // **disabled** (compile-time skip)  
```

- **Purpose**: Evaluate `expr`; report PASS, FAIL, or SKIP.  
- **Parameters**:  
  - `expr` – boolean expression to test.  
  - `fmt`, `...` – optional `printf`-style message on failure.  
- **Behavior**:  
  - **PASS** if `expr` is true.  
  - **FAIL** if false (prints `expr` and formatted message).  
  - **SKIP** if within a `tstskipif` block.  
- **Example**:
  ```c
  tstcheck(fact(0) == 1, "Expected 1, got %d", fact(0));
  ```


---

### `tstassert`

```c
tstassert(expr, fmt, ...);
tst_assert(expr, fmt, ...); // **disabled** (compile-time skip)  
```

- **Purpose**: Same as `tstcheck`, but **aborts** the entire suite on failure.  
- **Use Case**: Unrecoverable errors (e.g., out-of-memory).  
- **Example**:
  ```c
  tstassert(ptr = malloc(n), "Allocation failed for %d bytes", n);
  ```


---

## Conditional Execution

### `tstskipif`

```c
tstskipif(expr) {
  /* checks… */
}
tst_skpif(expr) { // **disabled** (compile-time skip)  
  /* This will not be executer */
}
```

- **Purpose**: Skip all checks in its block if `expr` is true.  
- **Behavior**:  
  - Marks skipped checks as **SKIP** (does not count as FAIL).  
- **Example**:
  ```c
  tstskipif(db_conn == NULL) {
    tstcheck(query() == SUCCESS);
  }
  ```


---

### Tags: `tsttag`

```c
int tsttag(Tag,[0|1]);
```

- **Purpose**: Enable/query named tags for selective test runs.  
- **Usage**:  
  - `tsttag(Tag)` → returns non-zero if `Tag` is enabled.  
  - `tsttag(Tag, 0)` → disable at runtime.  
  - `tsttag(Tag, 1)` → enable at runtime.  
- **Tags** declared via the varargs on `tstsuite`.  
- **Example**:
  ```c
  if (tsttag(SlowTests)) {
    tstcheck(run_full_benchmark());
  }
  ```

---

## Test State Queries

```c
#define tst(expr)            /* perform expr as test */
int tstfailed(void);
int tstpassed(void);
int tstskipped(void);
```

- **`tst(expr)`** – record `expr` as the most recent test (no PASS/FAIL log).  
- **`tstpassed()`** – non-zero if last `tst`/`tstcheck` passed.  
- **`tstfailed()`** – non-zero if it failed.  
- **`tstskipped()`** – non-zero if it was skipped.  
- **Example**:
  ```c
  tst(ptr != NULL);
  if (tstfailed()) {
    // handle failure
  }
  ```


---

## Timing and Notes

### `tstclock`

```c
tstclock(fmt, ...);
tst_clock(fmt, ...);
```

- **Purpose**: Measure CPU time for the enclosed code block.  
- **Behavior**:  
  - On block exit, prints elapsed clocks with unit (ns/µs/ms) and optional message.  
- **Example**:
  ```c
  tstclock("Sorting %d items", n) {
    sort(array, n);
  }
  ```


---

### `tstelapsed`

```c
clock_t tstelapsed(void);
```

- **Purpose**: Retrieve the last measured clock ticks from `tstclock`.  
- **Example**:
  ```c
  clock_t clk = tstelapsed();
  ```


---

### `tstnote`

```c
tstnote(fmt, ...);
```

- **Purpose**: Print an informational note during test execution.  
- **Example**:
  ```c
  tstnote("Testing input: %s", input_str);
  ```


---

### `tstouterr`

```c
tstouterr(fmt, ...);
```

- **Purpose**: Emit a block of diagnostic output delimited by markers.  
- **Use Case**: Capturing multi-line text for comparison.  
- **Example**:
  ```c
  tstouterr("Output was:\n%s", result_buf);
  ```


---

## Data-Driven Tests

Within a `tstcase`, you can define a static array:

```c
Type tstdata[] = { … };
```

- **`tstcurdata`** – current element inside `tstsection` loops.  
- **`tst_data_size`** – number of elements in `tstdata`.  

Sections automatically iterate over all `tstdata` elements. 

---

## Command-Line Options

The generated test executable accepts:

- `--help`       – show usage and tags  
- `--color`      – toggle ANSI colors  
- `--report-error` – return non-zero on failures  
- `--list`       – list suite name and tags  
- `+Tag` / `-Tag` – enable/disable specific tags  
- `*`            – toggle all tags  

Defaults can be set via the `TSTOPTIONS` environment variable. 