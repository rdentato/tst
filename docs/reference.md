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
    - [`tstexpect`](#tstexpect)
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

- **Purpose**: Declares the test suite, defines `main()`, and prints suite header
- **Parameters**:  
  - `title` – string shown as the suite name.  
  - Optional up to eight tag identifiers for conditional execution.  
- **Behavior**:  
  - `tstsuite` → suite **enabled**  
  - `tst_suite` → suite **disabled** (compile-time skip)  
  - Creates a `main()` function that parses command-line options and runs tests.
  - Prints suite start/end with timestamps and final PASS/FAIL/SKIP counts.
- **Examples**:
  ```c
  // Simple suite without tags
  #include "tst.h"
  
  tstsuite("String Functions") {
    tstcase("strlen tests") {
      tstcheck(strlen("hello") == 5);
      tstcheck(strlen("") == 0);
    }
    
    tstcase("strcmp tests") {
      tstcheck(strcmp("abc", "abc") == 0);
      tstcheck(strcmp("abc", "def") < 0);
    }
  }
  
  // Suite with tags for selective execution
  #include "tst.h"
  
  tstsuite("Database Tests", SlowTests, RequiresDB, Integration) {
    tstskipif(tsttag(RequiresDB) && !db_available()) {
      tstcase("Connection test") {
        tstcheck(db_connect() == 0);
        tstcheck(db_ping() == 0);
      }
    }
    
    tstskipif(tsttag(SlowTests)) {
      tstcase("Large dataset") {
        tstclock("Insert 10000 records") {
          for (int i = 0; i < 10000; i++)
            db_insert(i);
        }
        tstcheck(db_count() == 10000);
      }
    }
  }
  
  // Disabled suite (useful during development)
  tst_suite("Work in Progress Tests") {
    // This entire suite is disabled at compile time
    tstcase("Experimental feature") {
      tstcheck(new_feature() == 0);
    }
  }
  ```


---

## Test Cases and Sections

### `tstcase`

```c
tstcase(description, ...);
tst_case(...); // **disabled** (compile-time skip)  
```

- **Purpose**: Starts a new test case within the suite.  
- **Parameters**:  
  - `description` – textual label for grouping related checks (supports `printf`-style formatting).  
  - Optional additional arguments for formatting.  
- **Behavior**:  
  - Prints a "CASE" header.  
  - Reports partial PASS/FAIL/SKIP counts at the end of the block.  
  - `tst_case` → case **disabled** (compile-time skip)  
  - **Can be nested** inside other `tstcase` blocks for hierarchical organization.  
- **Example**:
  ```c
  tstcase("Edge Conditions") {
    tstcheck(x == 0);
    tstcheck(y != NULL);
  }
  
  tstcase("Testing value %d", test_val) {
    tstcheck(test_val > 0);
  }
  
  // Nested cases
  tstcase("Outer case") {
    int a = 5;
    tstcase("Inner case with a=%d", a) {
      tstcheck(a == 5);
    }
  }
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
tstsection(description, ...);
tst_section(description); // **disabled** (compile-time skip)  
```

- **Purpose**: Defines a subsection inside a `tstcase`, isolating a subset of checks.  
- **Parameters**:  
  - `description` – label printed before the section's checks (supports `printf`-style formatting).  
  - Optional additional arguments for formatting.  
- **Behavior**:  
  - Runs setup code before each section.  
  - Supports iterating over a `tstdata` array for data-driven tests.  
  - Each section re-executes the setup code from the beginning of the enclosing `tstcase`.  
- **Example**:
  ```c
  int a = 5;
  tstcase("Value Updates") {
    tstsection("Set to 9") {
      tstcheck(a == 5);  // Setup runs before each section
      a = 9;
      tstcheck(a == 9);
    }
    tstsection("Set to 8") {
      tstcheck(a == 5);  // a is reset to 5 again
      a = 8;
      tstcheck(a == 8);
    }
  }
  
  // With formatting
  tstsection("Testing with value=%d", test_val) {
    tstcheck(process(test_val) == expected);
  }
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
  - **Can be used outside `tstcase`** blocks (at suite level).  
- **Example**:
  ```c
  tstcheck(fact(0) == 1, "Expected 1, got %d", fact(0));
  
  // Can be used at suite level
  tstsuite("My Suite") {
    tstcheck(initialization() == 0);  // Suite-level check
    
    tstcase("Test 1") {
      tstcheck(test1() == 1);  // Case-level check
    }
  }
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

### `tstexpect`

```c
tstexpect(expr, fmt, ...);
tst_expect(expr, fmt, ...); // **disabled** (compile-time skip)  
```

- **Purpose**: Similar to `tstcheck`, but only prints output on **failure** (silent on PASS).  
- **Use Case**: High-volume assertions where you only want to see failures.  
- **Behavior**:  
  - **PASS** is counted but not printed.  
  - **FAIL** prints the expression and formatted message.  
  - **SKIP** if within a `tstskipif` block.  
- **Example**:
  ```c
  tstexpect(x > 0, "Expected positive value, got %d", x);
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
#define tstelapsed() tstelapsed
```

- **Purpose**: Access the last measured clock ticks from `tstclock`.  
- **Type**: `clock_t` variable (accessed as a macro).  
- **Example**:
  ```c
  tstclock("Benchmark") {
    // ... code to time ...
  }
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
tstouterr(fmt, ...) {
  // code block
}
```

- **Purpose**: Emit a block of diagnostic output delimited by markers (`<<<<<` and `>>>>>`).  
- **Use Case**: Capturing multi-line text for comparison or displaying generated data.  
- **Behavior**: Prints opening marker, executes the block, then prints closing marker.  
- **Examples**:
  ```c
  tstouterr("Output was:\n%s", result_buf);
  
  // With code block
  tstouterr("Generated data:") {
    for (int k=0; k<4; k++) {
      tstdata[k] = rand() & 0x0F;
      tstprintf("[%d] = %d\n", k, tstdata[k]);
    }
  }
  ```

---

### `tstprintf`

```c
tstprintf(fmt, ...);
```

- **Purpose**: Print formatted output to stderr (same as `fprintf(stderr, ...)`).  
- **Use Case**: Custom diagnostic messages during test execution.  
- **Example**:
  ```c
  tstprintf("Debug: x=%d, y=%d\n", x, y);
  ```


---

## Data-Driven Tests

Within a `tstcase`, you can define an array named `tstdata`:

```c
Type tstdata[] = { … };
```

- **`tstcurdata`** – expands to `tstdata[tst_data_count]`, the current element inside `tstsection` loops.  
- **`tst_data_size`** – expands to `(int)(sizeof(tstdata)/sizeof(tstdata[0]))`, the number of elements.  

Each `tstsection` automatically iterates over all `tstdata` elements. The iteration variable `tst_data_count` goes from `0` to `tst_data_size - 1`.

**Notes**:
- The array name **must** be `tstdata`.
- Can be any type (int, struct, etc.).
- The `volatile` keyword is optional (used internally by the framework, not required in user code).
- For dynamic/random data, populate the array before the section.

**Examples**:

Static integer data:
```c
tstcase("Factorial Tests") {
  int tstdata[] = {0, 1, 5, 10, 100};
  
  tstsection("Non-negative inputs") {
    int n = tstcurdata;
    tstcheck(factorial(n) >= n, "factorial(%d) should be >= %d", n, n);
  }
}
```

Struct data:
```c
tstcase("String tests") {
  struct {int n; const char *s;} tstdata[] = {
    {123, "hello"},
    {456, "world"},
    {789, "test"}
  };
  
  tstsection("Process data") {
    tstnote("Checking <%d,%s>", tstcurdata.n, tstcurdata.s);
    tstcheck(process(tstcurdata.n, tstcurdata.s) == 0);
  }
}
```

Random data:
```c
tstcase("Random values in range") {
  srand(time(0));
  int tstdata[4];
  for (int k=0; k<4; k++) tstdata[k] = rand() % 100;
  
  tstsection("Check range") {
    tstnote("Checking: %d", tstcurdata);
    tstcheck(tstcurdata >= 0 && tstcurdata < 100);
  }
}
``` 

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