# TST Library Reference Manual

**Version:** 0.8.1-beta  
**License:** MIT  
**Last Updated:** November 2025

---

## Table of Contents

1. [Introduction](#introduction)
2. [Library Components](#library-components)
   - [tst.h - Testing Framework](#tsth---testing-framework)
   - [t2h - HTML Report Generator](#t2h---html-report-generator)
3. [tst.h API Reference](#tsth-api-reference)
   - [Suite Definition](#suite-definition)
   - [Test Organization](#test-organization)
   - [Assertions and Checks](#assertions-and-checks)
   - [Control Flow](#control-flow)
   - [Tag-Based Filtering](#tag-based-filtering)
   - [Output and Diagnostics](#output-and-diagnostics)
   - [Performance Measurement](#performance-measurement)
   - [Data-Driven Testing](#data-driven-testing)
   - [Test State Queries](#test-state-queries)
   - [Global Variables](#global-variables)
4. [t2h Utility Reference](#t2h-utility-reference)
   - [Command-Line Interface](#command-line-interface)
   - [Features and Capabilities](#features-and-capabilities)
   - [Output Format](#output-format)
5. [Command-Line Options](#command-line-options)
6. [Environment Variables](#environment-variables)
7. [Output Format Specification](#output-format-specification)
8. [Best Practices](#best-practices)
9. [Platform Support](#platform-support)

---

## Introduction

The TST library provides a complete testing solution for C and C++ projects, consisting of two main components:

1. **tst.h** - A lightweight, single-header unit testing framework with zero dependencies
2. **t2h** - An HTML report generator that converts test logs into interactive dashboards

### Design Philosophy

- **Simplicity**: Single header file, no build configuration required
- **Zero Dependencies**: Uses only standard C library functions
- **Minimal Syntax**: Clear, intuitive macros for common testing patterns
- **Flexibility**: Tag-based filtering, data-driven tests, and conditional execution
- **Portability**: Works with gcc, clang, MSVC (cl), g++, and mingw-gcc on Linux, macOS, and Windows

### Key Features

- Hierarchical test organization (suites, cases, sections)
- Multiple assertion types with formatted messages
- Tag-based selective test execution
- Data-driven testing with automatic iteration
- Built-in performance timing
- Conditional test skipping
- Structured, parseable log output
- Interactive HTML reports with source code extraction

---

## Library Components

### tst.h - Testing Framework

A header-only testing framework that provides macros and functions for defining and running tests. Include `tst.h` in your test files to access all testing facilities. The framework automatically generates a `main()` function, so test files can be compiled and executed directly.

**Usage:**
```c
#include "tst.h"

tstsuite("My Test Suite") {
    tstcase("Basic Tests") {
        tstcheck(1 + 1 == 2);
    }
}
```

**Compilation:**
```bash
gcc -o test_program test.c
./test_program
```

### t2h - HTML Report Generator

A standalone utility that converts TST log output into interactive HTML dashboards with visual statistics, syntax highlighting, and source code extraction.

**Usage:**
```bash
# Single file
./test_program | t2h > report.html

# Multiple files
t2h test1.log test2.log > consolidated.html
```

**Building t2h:**
```bash
cd src
make t2h
```

---

## tst.h API Reference

### Suite Definition

#### `tstsuite(title, ...)`

Defines an enabled test suite and generates the `main()` function.

**Syntax:**
```c
tstsuite(title)
tstsuite(title, Tag1, Tag2, ...)
```

**Parameters:**
- `title` - String literal describing the test suite (displayed in output)
- Optional tag identifiers (up to 8) for selective execution

**Behavior:**
- Creates a `main()` function that parses command-line arguments
- Prints suite header with filename, title, and timestamp
- Executes all test cases and sections
- Prints final results with FAIL/PASS/SKIP counts and timestamp
- Returns 0 on success, or non-zero if `--report-error` flag is set and tests failed

**Example:**
```c
tstsuite("String Library Tests") {
    tstcase("strlen") {
        tstcheck(strlen("hello") == 5);
        tstcheck(strlen("") == 0);
    }
}
```

**With Tags:**
```c
tstsuite("Database Tests", SlowTests, RequiresDB) {
    tstcase("Connection") {
        tstcheck(db_connect() == 0);
    }
}
```

**Notes:**
- Only one `tstsuite` per source file
- Do not define your own `main()` function
- Tags are bare identifiers (no quotes)

#### `tst_suite(title, ...)`

Defines a disabled test suite (compile-time skip).

**Syntax:**
```c
tst_suite(title)
tst_suite(title, Tag1, Tag2, ...)
```

**Behavior:**
- Generates `main()` but skips all test execution
- Prints suite header marked as "(disabled)"
- Useful for work-in-progress tests or debugging

**Example:**
```c
tst_suite("Experimental Features") {
    // Code is compiled but not executed
    tstcheck(experimental_api() == 0);
}
```

---

### Test Organization

#### `tstcase(description, ...)`

Starts a new test case within a suite.

**Syntax:**
```c
tstcase(description)
tstcase(description, +Tag1, -Tag2, ...)
```

**Parameters:**
- `description` - String literal describing the test case
- Optional tag filters: `+Tag` (requires tag enabled), `-Tag` (requires tag disabled)

**Behavior:**
- Prints "CASE,--" marker with description
- Executes all checks within the block
- Prints partial results (FAIL/PASS/SKIP counts) at block end
- Tags control whether case executes (see [Tag-Based Filtering](#tag-based-filtering))

**Example:**
```c
tstcase("Edge Cases") {
    tstcheck(func(0) == 0);
    tstcheck(func(INT_MAX) > 0);
}
```

**With Tags:**
```c
tstcase("Database Tests", +RequiresDB) {
    tstcheck(db_query("SELECT 1") == 0);
}

tstcase("Fast Tests", -SlowTests) {
    tstcheck(quick_check() == 1);
}
```

**Notes:**
- Test cases cannot be nested
- Tags are processed left-to-right (later tags override earlier ones)
- Untagged test cases always run

#### `tst_case(description)`

Defines a disabled test case (compile-time skip).

**Syntax:**
```c
tst_case(description)
```

**Behavior:**
- Code is compiled but not executed
- No output generated

**Example:**
```c
tst_case("Broken Test") {
    tstcheck(buggy_function() == 0);
}
```

#### `tstsection(description, ...)`

Defines a subsection within a test case with setup/teardown isolation.

**Syntax:**
```c
tstsection(description)
tstsection(format, arg1, arg2, ...)
```

**Parameters:**
- `description` or `format` - String describing the section (supports printf-style formatting)
- Optional formatting arguments

**Behavior:**
- Prints "SCTN|,--" marker with description
- Re-executes enclosing `tstcase` setup code before each section
- Iterates over `tstdata` array if defined (see [Data-Driven Testing](#data-driven-testing))
- Provides isolation between sections

**Example:**
```c
tstcase("File Operations") {
    FILE *f = fopen("test.txt", "r");
    tstassert(f != NULL);
    
    tstsection("Read first line") {
        char buf[100];
        tstcheck(fgets(buf, sizeof(buf), f) != NULL);
    }
    
    tstsection("Read second line") {
        // f is reopened (setup code re-executed)
        char buf[100];
        tstcheck(fgets(buf, sizeof(buf), f) != NULL);
    }
    
    fclose(f);
}
```

**With Formatting:**
```c
int test_val = 42;
tstsection("Testing value %d", test_val) {
    tstcheck(process(test_val) == test_val);
}
```

**Notes:**
- Each section re-runs the enclosing case from the beginning
- Cleanup code (after sections) runs after each section
- Sections cannot be nested

#### `tst_section(description, ...)`

Defines a disabled section (compile-time skip).

**Syntax:**
```c
tst_section(description)
```

**Behavior:**
- Code is compiled but not executed

---

### Assertions and Checks

#### `tst(expression)`

Silently records a test result without printing output.

**Syntax:**
```c
tst(expression)
```

**Parameters:**
- `expression` - Boolean expression to evaluate

**Behavior:**
- Sets internal test result flag
- No output generated
- Use with `tstpassed()`, `tstfailed()`, or `tstskipped()` to query result

**Example:**
```c
tst(ptr != NULL);
if (tstfailed()) {
    cleanup();
    return;
}
```

#### `tstcheck(condition, message, ...)`

Standard assertion with output.

**Syntax:**
```c
tstcheck(condition)
tstcheck(condition, message, ...)
```

**Parameters:**
- `condition` - Boolean expression to test
- `message` - Optional printf-style format string
- `...` - Optional formatting arguments

**Behavior:**
- Evaluates `condition`
- Prints PASS, FAIL, or SKIP marker with line number and expression
- On failure, prints optional message
- Continues execution after failure

**Output:**
```
   42 PASS|  x > 0
   43 FAIL|  y == 5 "Expected 5, got 7"
```

**Example:**
```c
tstcheck(strlen("hello") == 5);
tstcheck(result == expected, "Expected %d, got %d", expected, result);
```

**Notes:**
- Can be used outside `tstcase` (at suite level)
- Expression is displayed exactly as written

#### `tst_check(condition, message, ...)`

Disabled check (compile-time skip).

**Syntax:**
```c
tst_check(condition)
tst_check(condition, message, ...)
```

**Behavior:**
- No code generated
- Useful for temporarily disabling expensive checks

#### `tstassert(condition, message, ...)`

Critical assertion that aborts suite on failure.

**Syntax:**
```c
tstassert(condition)
tstassert(condition, message, ...)
```

**Parameters:**
- Same as `tstcheck`

**Behavior:**
- Evaluates `condition`
- On failure: prints message, prints partial results, exits suite immediately
- On success: prints PASS marker and continues

**Output on Failure:**
```
   15 FAIL|  ptr != NULL "Out of memory"
   12     `--- 1 FAIL | 2 PASS | 0 SKIP
^^^^^ ABRT \ 1 FAIL | 2 PASS | 0 SKIP 2025-11-27 10:30:45
```

**Example:**
```c
void *ptr = malloc(1024);
tstassert(ptr != NULL, "Failed to allocate memory");
// If malloc fails, suite exits here
```

**Use Cases:**
- Unrecoverable errors (out of memory, file not found)
- Prerequisites for subsequent tests
- Critical setup failures

#### `tst_assert(condition, message, ...)`

Disabled assert (compile-time skip).

#### `tstexpect(condition, message, ...)`

Assertion that only prints on failure (silent on pass).

**Syntax:**
```c
tstexpect(condition)
tstexpect(condition, message, ...)
```

**Parameters:**
- Same as `tstcheck`

**Behavior:**
- On pass: increments pass counter, no output
- On failure: prints FAIL marker and message
- On skip: prints SKIP marker

**Example:**
```c
// Reduce log clutter for high-volume checks
for (int i = 0; i < 1000; i++) {
    tstexpect(array[i] >= 0, "Negative value at index %d", i);
}
```

**Use Cases:**
- High-volume assertions (loops over large datasets)
- Tests where only failures are interesting
- Reducing log size

#### `tst_expect(condition, message, ...)`

Disabled expect (compile-time skip).

---

### Control Flow

#### `tstskipif(condition)`

Conditionally skips enclosed tests.

**Syntax:**
```c
tstskipif(condition) {
    // tests to skip
}
```

**Parameters:**
- `condition` - Boolean expression

**Behavior:**
- If `condition` is true, all enclosed checks are marked as SKIP
- Prints skip condition message
- Skipped tests count toward skip total, not failures

**Output:**
```
   15 SKPT|,-(condition)
   16 SKIP|  test1()
   17 SKIP|  test2()
        |`---
```

**Example:**
```c
int db_available = connect_database();
tstskipif(!db_available) {
    tstcheck(query_user(1) != NULL);
    tstcheck(query_user(2) != NULL);
}
```

**Nested Skip Conditions:**
```c
tstskipif(!feature_enabled) {
    tstskipif(!debug_mode) {
        // Only runs if feature_enabled AND debug_mode
        tstcheck(debug_feature());
    }
}
```

#### `tst_skpif(condition)`

Disabled skip (compile-time skip).

---

### Tag-Based Filtering

Test cases can be tagged with identifiers that control selective execution. Tags are specified in `tstcase` declarations and controlled via command-line arguments.

**Tag Syntax:**
```c
tstcase("Test name", +Tag1, +Tag2)   // Requires Tag1 AND Tag2 enabled
tstcase("Test name", -SlowTests)     // Requires SlowTests disabled
```

**Tag Semantics:**

| Syntax | Meaning |
|--------|---------|
| `+Tag` | Test requires this tag to be enabled |
| `-Tag` | Test requires this tag to be disabled |
| No tags | Test always runs (immune to filters) |

**Command-Line Filters:**

| Filter | Effect |
|--------|--------|
| `+TAG` | Enable tests with `+TAG`, disable tests with `-TAG` |
| `-TAG` | Disable tests with `+TAG`, enable tests with `-TAG` |
| `+*` | Enable all tests with any `+TAG` (not `-TAG` tests) |

**Filter Processing:**
- Filters are processed left-to-right
- Later filters override earlier ones
- Untagged tests always run

**Truth Table:**

| Test Tags | No Filter | `+*` | `+TAG` | `-TAG` |
|-----------|-----------|------|--------|--------|
| (no tags) | ✅ RUN | ✅ RUN | ✅ RUN | ✅ RUN |
| `+TAG` | ❌ SKIP | ✅ RUN | ✅ RUN | ❌ SKIP |
| `-TAG` | ❌ SKIP | ❌ SKIP | ❌ SKIP | ✅ RUN |
| `+OTHER` | ❌ SKIP | ✅ RUN | ❌ SKIP | ❌ SKIP |

**Example:**
```c
tstsuite("API Tests") {
    // Always runs
    tstcase("Basic Tests") {
        tstcheck(api_version() == 1);
    }
    
    // Only runs with +RequiresDB
    tstcase("Database Test", +RequiresDB) {
        tstcheck(db_connect() == 0);
    }
    
    // Only runs with -SlowTests (i.e., when slow tests disabled)
    tstcase("Fast Test", -SlowTests) {
        tstcheck(quick_check() == 1);
    }
}
```

**Command-Line Usage:**
```bash
# Run untagged tests only
./test_program

# Run database tests
./test_program +RequiresDB

# Run all tests except slow ones
./test_program +* -SlowTests

# List tests with their tags
./test_program --list
```

---

### Output and Diagnostics

#### `tstnote(message, ...)`

Prints an informational note during test execution.

**Syntax:**
```c
tstnote(format, ...)
```

**Parameters:**
- `format` - printf-style format string
- `...` - formatting arguments

**Output:**
```
   42 NOTE: Testing configuration: production
```

**Example:**
```c
tstnote("Testing with user_id=%d", user_id);
tstnote("Current state: %s", get_state_name());
```

#### `tst_note(message, ...)`

Disabled note (compile-time skip).

#### `tstprintf(format, ...)`

Prints directly to stderr.

**Syntax:**
```c
tstprintf(format, ...)
```

**Parameters:**
- `format` - printf-style format string
- `...` - formatting arguments

**Behavior:**
- Direct output to stderr (no formatting by framework)
- No line numbers or markers added

**Example:**
```c
tstprintf("Debug: x=%d, y=%d\n", x, y);
```

#### `tstouterr(message, ...)`

Delimited output block for multi-line content.

**Syntax:**
```c
tstouterr(message, ...) {
    // code that prints output
}
```

**Parameters:**
- `message` - printf-style header message
- `...` - formatting arguments

**Output:**
```
   42 <<<<< API Response:
Status: 200
Body: {"result": "success"}
   42 >>>>>
```

**Example:**
```c
tstouterr("Generated data:") {
    for (int i = 0; i < 5; i++) {
        tstprintf("[%d] = %d\n", i, data[i]);
    }
}
```

---

### Performance Measurement

#### `tstclock(description, ...)`

Measures CPU time for a code block.

**Syntax:**
```c
tstclock(description, ...) {
    // code to time
}
```

**Parameters:**
- `description` - printf-style format string
- `...` - formatting arguments

**Behavior:**
- Records CPU time before block entry
- Executes block
- Calculates elapsed time and prints with appropriate unit (ns/µs/ms)

**Output:**
```
   42 CLCK:  1250 ms Sorting 10000 elements
```

**Example:**
```c
tstclock("Sort %d elements", n) {
    sort_array(data, n);
}
```

**Important:** Measures CPU time, not wall-clock time. Sleep calls show ~0ms.

#### `tst_clock(description, ...)`

Disabled clock (compile-time skip).

#### `tstelapsed()`

Accesses the last measured clock ticks.

**Syntax:**
```c
clock_t ticks = tstelapsed();
```

**Returns:**
- `clock_t` value representing CPU ticks from last `tstclock` block

**Example:**
```c
tstclock("Benchmark") {
    compute_intensive_task();
}
clock_t elapsed = tstelapsed();
tstcheck(elapsed < 1000000, "Too slow: %ld ticks", elapsed);
```

---

### Data-Driven Testing

Test sections can iterate over an array of test data, allowing the same test logic to run with different inputs.

**Array Declaration:**
```c
Type tstdata[] = { ... };
```

**Requirements:**
- Array must be named `tstdata` (exact name required)
- Must be declared within a `tstcase` block
- Can be any type (int, struct, pointer, etc.)

**Access Macros:**

| Macro | Description |
|-------|-------------|
| `tstcurdata` | Current element (`tstdata[tst_data_count]`) |
| `tst_data_size` | Number of elements (`sizeof(tstdata)/sizeof(tstdata[0])`) |
| `tst_data_count` | Current iteration index (0 to size-1) |

**Behavior:**
- Each `tstsection` iterates over all `tstdata` elements
- `tstcurdata` changes each iteration
- Setup/teardown code runs for each element

**Example - Integer Array:**
```c
tstcase("Factorial Tests") {
    int tstdata[] = {0, 1, 2, 3, 4, 5};
    
    tstsection("Test factorial") {
        int n = tstcurdata;
        int result = factorial(n);
        tstcheck(result >= 1, "factorial(%d) = %d", n, result);
    }
}
```

**Example - Struct Array:**
```c
tstcase("String Validation") {
    struct {
        const char *input;
        int expected_length;
    } tstdata[] = {
        {"hello", 5},
        {"world", 5},
        {"", 0},
        {"test123", 7}
    };
    
    tstsection("Validate string") {
        tstnote("Testing: \"%s\"", tstcurdata.input);
        tstcheck(strlen(tstcurdata.input) == tstcurdata.expected_length);
    }
}
```

**Example - Random Data:**
```c
tstcase("Random Input Tests") {
    int tstdata[100];
    
    // Generate random data
    srand(time(NULL));
    for (int i = 0; i < 100; i++) {
        tstdata[i] = rand() % 1000;
    }
    
    tstsection("Check bounds") {
        tstexpect(tstcurdata >= 0 && tstcurdata < 1000,
                 "Value %d out of bounds", tstcurdata);
    }
}
```

**Multiple Sections:**
```c
tstcase("Comprehensive Tests") {
    struct {const char *str; int len;} tstdata[] = {
        {"abc", 3}, {"hello", 5}, {"x", 1}
    };
    
    tstsection("Check length") {
        tstcheck(strlen(tstcurdata.str) == tstcurdata.len);
    }
    
    tstsection("Check non-empty") {
        // Iterates over all data again
        tstcheck(tstcurdata.str[0] != '\0');
    }
}
```

---

### Test State Queries

#### `tstpassed()`

Checks if the last test passed.

**Syntax:**
```c
int tstpassed(void);
```

**Returns:**
- Non-zero if last test passed
- Zero otherwise

**Example:**
```c
tstcheck(complex_operation());
if (tstpassed()) {
    verify_side_effects();
}
```

#### `tstfailed()`

Checks if the last test failed.

**Syntax:**
```c
int tstfailed(void);
```

**Returns:**
- Non-zero if last test failed
- Zero otherwise

**Example:**
```c
tst(ptr != NULL);
if (tstfailed()) {
    cleanup();
    return;
}
```

#### `tstskipped()`

Checks if the last test was skipped.

**Syntax:**
```c
int tstskipped(void);
```

**Returns:**
- Non-zero if last test was skipped
- Zero otherwise

**Example:**
```c
tstcheck(optional_feature());
if (tstskipped()) {
    tstnote("Feature not available");
}
```

---

### Global Variables

The following global variables track test execution state:

**Test Counters (Suite-Level):**

| Variable | Type | Description |
|----------|------|-------------|
| `tst_pass` | int | Total passed tests in suite |
| `tst_fail` | int | Total failed tests in suite |
| `tst_skip` | int | Total skipped tests in suite |

**Test Counters (Case-Level):**

| Variable | Type | Description |
|----------|------|-------------|
| `tst_case_pass` | short | Passed tests in current case |
| `tst_case_fail` | short | Failed tests in current case |
| `tst_case_skip` | short | Skipped tests in current case |

**Example:**
```c
tstsuite("Counter Example") {
    tstcheck(1 == 1);
    tstcheck(2 == 2);
    
    tstnote("So far: %d passes, %d fails", tst_pass, tst_fail);
    
    tstcase("Test Case 1") {
        tstcheck(3 == 3);
        tstnote("This case: %d passes", tst_case_pass);
    }
}
```

**Notes:**
- These are read-only from user perspective (do not modify)
- Reset at appropriate scope boundaries
- Useful for conditional logic based on test outcomes

---

## t2h Utility Reference

The `t2h` (TST to HTML) utility converts test log files into interactive HTML reports with visual statistics, syntax highlighting, and source code extraction.

### Command-Line Interface

**Syntax:**
```bash
t2h [OPTIONS] [file1.log file2.log ...]
t2h < input.log > output.html
```

**Options:**

| Option | Description |
|--------|-------------|
| `-h, --help` | Show help message and exit |
| `-v, --version` | Show version information and exit |
| `--dark` | Generate report with dark theme (default) |
| `--dark-theme` | Same as `--dark` |
| `--light` | Generate report with light theme |
| `--light-theme` | Same as `--light` |

**Input Modes:**

1. **Standard Input:** Read from pipe or redirection
   ```bash
   ./test_program | t2h > report.html
   cat test.log | t2h > report.html
   ```

2. **File Input:** Read from one or more log files
   ```bash
   t2h test.log > report.html
   t2h test1.log test2.log test3.log > consolidated.html
   ```

**Output:**
- Single HTML file written to stdout
- Redirect to file to save report

**Exit Codes:**

| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | Error (file not found, parse error, allocation failure) |

**Examples:**
```bash
# Basic usage
./test_basics | t2h > basics_report.html

# Light theme
t2h --light test.log > light_report.html

# Consolidated report from multiple files
t2h logs/*.log > consolidated_report.html

# Process files with dark theme (explicit)
t2h --dark test1.log test2.log > dark_report.html
```

### Features and Capabilities

**Visual Dashboard:**
- Color-coded test case sections (green=passed, red=failed, yellow=mixed/skipped)
- Summary statistics with counts and percentages
- Interactive expandable sections for logs and source code
- SVG pie charts showing pass/fail/skip distribution

**Source Code Extraction (FUNC-006):**
- Automatic extraction of test case source code from test files
- Syntax highlighting for C/C++ code
- Line number display matching log output
- Keyword, type, string, comment, and preprocessor directive highlighting

**Multi-Suite Support:**
- Consolidated reports from multiple test files
- Separate sections for each suite
- Aggregate statistics across all suites
- Suite-level collapsible sections with navigation

**Interactive Features:**
- Clickable test cases to expand/collapse logs
- Tab switching between log output and source code
- Theme toggle button (light/dark mode)
- "Back to Top" navigation for multi-suite reports

**Log Viewer:**
- Syntax highlighting for log markers (PASS/FAIL/SKIP/NOTE/CLCK)
- Preserved indentation and tree structure
- Line numbers from original test execution
- Color-coded status indicators

**Error Handling:**
- Graceful handling of missing source files
- Warnings for truncated logs (exceeds MAX_LOG_LINES)
- Clear error messages for file I/O failures
- Robust parsing of malformed log lines

**Percentage Calculations:**
- Pass% = passed / (passed + failed) × 100
- Fail% = failed / (passed + failed) × 100
- Skipped tests excluded from percentage calculations
- Shows actual execution success rate

### Output Format

**HTML Structure:**
```
- Header (title, theme toggle, aggregate stats for multi-suite)
- For each test suite:
  - Suite header (title, file, timestamps, badges)
  - Suite content (collapsible for multi-suite):
    - For each test case:
      - Case header (name, badges, log/code buttons)
      - Tab container (if source available)
      - Log viewer (expandable)
      - Source code viewer (expandable, if available)
```

**CSS Themes:**
- **Dark Theme (default):** Dark background, light text, syntax-highlighted code
- **Light Theme:** Light background, dark text, high contrast

**Color Scheme:**

| Element | Dark Theme | Light Theme |
|---------|------------|-------------|
| Background | #0d1117 | #f5f5f5 |
| Card | #161b22 | #ffffff |
| Text | #e6edf3 | #333333 |
| Border | #30363d | #dddddd |
| Pass | #28a745 | #28a745 |
| Fail | #dc3545 | #dc3545 |
| Skip | #ffc107 | #ffc107 |

**JavaScript Features:**
- No external dependencies (all inline)
- Toggle functions for expand/collapse
- Tab switching for log/source views
- Theme switching
- Smooth scrolling for "Back to Top"

**Browser Compatibility:**
- Modern browsers (Chrome, Firefox, Safari, Edge)
- Responsive design (works on mobile)
- No JavaScript required for viewing (degraded functionality)

---

## tst.sh API Reference

The `tst.sh` library provides bash functions for writing shell-based tests that produce TST-formatted output.

### Library Components

#### Installation

Source the library in your bash test scripts:

```bash
#!/bin/bash
source "/path/to/tst.sh"
```

#### Output

All output is written to stderr (`>&2`) to match tst.h behavior.

### Suite Management Functions

#### `tstsuite_begin`

Starts a test suite and prints the suite header.

**Syntax:**
```bash
tstsuite_begin <suite_title> <filename> [disabled]
```

**Parameters:**
- `suite_title` - String describing the test suite
- `filename` - Source filename (typically `$0`)
- `disabled` - Optional flag; if set, prints "(disabled)" suffix

**Behavior:**
- Resets total pass/fail/skip counters to 0
- Records suite title, filename, and start timestamp
- Prints suite header line: `----- SUIT / filename "title" timestamp`

**Example:**
```bash
tstsuite_begin "Integration Tests" "$0"
tstsuite_begin "Disabled Tests" "$0" "disabled"
```

#### `tstsuite_end`

Ends the test suite and prints final results.

**Syntax:**
```bash
tstsuite_end
```

**Behavior:**
- Captures end timestamp
- Prints `RSLT` line (normal) or `ABRT` line (if aborted) with counts
- Format: `^^^^^ RSLT \ FAIL | PASS | SKIP timestamp`

**Example:**
```bash
tstsuite_end
```

**Output:**
```
^^^^^ RSLT \ 1 FAIL | 5 PASS | 2 SKIP 2025-11-28 10:30:50
```

### Test Case Functions

#### `tstcase_begin`

Starts a new test case.

**Syntax:**
```bash
tstcase_begin <case_title>
```

**Parameters:**
- `case_title` - String describing the test case

**Behavior:**
- Resets per-case pass/fail/skip counters to 0
- Auto-captures line number using `${BASH_LINENO[0]}`
- Stores line number for later use by `tstcase_end`
- Prints case header: `  LINE CASE,-- title`

**Example:**
```bash
tstcase_begin "File operations"
```

**Output:**
```
    8 CASE,-- File operations
```

#### `tstcase_end`

Ends the current test case and prints partial results.

**Syntax:**
```bash
tstcase_end
tstcase_end <line_number>
```

**Parameters:**
- `line_number` - Optional explicit line number; defaults to line from `tstcase_begin`

**Behavior:**
- Prints case summary with pass/fail/skip counts
- Format: `  LINE     \`--- FAIL | PASS | SKIP`

**Example:**
```bash
tstcase_end
```

**Output:**
```
    8     `--- 1 FAIL | 2 PASS | 0 SKIP
```

#### `tstcase_skip`

Marks a test case as skipped (for tag filtering).

**Syntax:**
```bash
tstcase_skip <case_title>
```

**Parameters:**
- `case_title` - String describing the skipped case

**Behavior:**
- Auto-captures line number
- Prints skip marker: `  LINE SKPT|,-(title)`
- Increments total skip counter

**Example:**
```bash
tstcase_skip "Database tests (no DB available)"
```

**Output:**
```
   15 SKPT|,-(Database tests (no DB available))
```

### Assertion Functions

#### `tstpass`

Records a passing check.

**Syntax:**
```bash
tstpass <expression>
tstpass <line_number> <expression>
```

**Parameters:**
- `expression` - String describing what passed
- `line_number` - Optional explicit line number

**Behavior:**
- Auto-detects line number using `${BASH_LINENO[0]}` if not provided
- Prints: `  LINE PASS|  expression`
- Increments case and total pass counters

**Example:**
```bash
tstpass "test -f config.ini"
tstpass $LINENO "test -f config.ini"  # Explicit
```

**Output:**
```
   42 PASS|  test -f config.ini
```

#### `tstfail`

Records a failing check with optional error message.

**Syntax:**
```bash
tstfail <expression> [message]
tstfail <line_number> <expression> [message]
```

**Parameters:**
- `expression` - String describing what failed
- `message` - Optional error message; use "-" or empty string for no message
- `line_number` - Optional explicit line number

**Behavior:**
- Auto-detects line number if not provided
- Prints: `  LINE FAIL|  expression "message"`
- Message is omitted if empty or "-"
- Increments case and total fail counters

**Example:**
```bash
tstfail "test -f missing.txt"
tstfail "parse result" "Expected success, got error"
tstfail $LINENO "validate" "Invalid format"  # Explicit
```

**Output:**
```
   43 FAIL|  test -f missing.txt
   44 FAIL|  parse result "Expected success, got error"
```

#### `tstskip`

Records a skipped check.

**Syntax:**
```bash
tstskip <expression>
tstskip <line_number> <expression>
```

**Parameters:**
- `expression` - String describing what was skipped
- `line_number` - Optional explicit line number

**Behavior:**
- Auto-detects line number if not provided
- Prints: `  LINE SKIP|  expression`
- Increments case and total skip counters

**Example:**
```bash
tstskip "network test"
```

**Output:**
```
   45 SKIP|  network test
```

#### `tstcheck`

Evaluates a command exit status and records result.

**Syntax:**
```bash
tstcheck <expression> <exit_status> [fail_message]
```

**Parameters:**
- `expression` - String describing the test
- `exit_status` - Exit code (0 = pass, non-zero = fail)
- `fail_message` - Optional message for failures

**Behavior:**
- If `exit_status == 0`, calls `tstpass`
- Otherwise, calls `tstfail` with message

**Example:**
```bash
ls /tmp > /dev/null 2>&1
tstcheck "ls /tmp succeeds" $? "Directory not found"

mkdir /tmp/testdir
tstcheck "Create directory" $?
```

#### `tstassert`

Critical check that aborts the suite on failure.

**Syntax:**
```bash
tstassert <expression> <exit_status> [fail_message]
```

**Parameters:**
- Same as `tstcheck`

**Behavior:**
- If `exit_status == 0`, calls `tstpass` and continues
- If `exit_status != 0`:
  - Calls `tstfail` with message
  - Calls `tstcase_end` to close case
  - Sets `TST_ABORT=1`
  - Calls `tstsuite_end` to close suite
  - Exits with status 1

**Example:**
```bash
mkdir -p /tmp/required_dir
tstassert "Create required dir" $? "Cannot continue without directory"
# Script exits here if mkdir failed
```

**Output on Failure:**
```
   20 FAIL|  Create required dir "Cannot continue without directory"
   15     `--- 1 FAIL | 0 PASS | 0 SKIP
^^^^^ ABRT \ 1 FAIL | 0 PASS | 0 SKIP 2025-11-28 10:30:50
```

### Section Functions

#### `tstsection_begin`

Starts a test section within a case.

**Syntax:**
```bash
tstsection_begin <section_title>
```

**Parameters:**
- `section_title` - String describing the section

**Behavior:**
- Auto-captures line number
- Stores line for `tstsection_end`
- Prints: `  LINE SCTN|,-- title`

**Example:**
```bash
tstsection_begin "Parse config file"
```

**Output:**
```
   25 SCTN|,-- Parse config file
```

#### `tstsection_end`

Ends the current section.

**Syntax:**
```bash
tstsection_end
tstsection_end <line_number>
```

**Parameters:**
- `line_number` - Optional explicit line number; defaults to line from `tstsection_begin`

**Behavior:**
- Prints section end marker: `  LINE     |\`---`

**Example:**
```bash
tstsection_end
```

**Output:**
```
   25     |`---
```

### Skip Block Functions

#### `tstskip_block_begin`

Starts a conditional skip block.

**Syntax:**
```bash
tstskip_block_begin <condition>
```

**Parameters:**
- `condition` - String describing the skip condition

**Behavior:**
- Auto-captures line number
- Stores line for `tstskip_block_end`
- Prints: `  LINE SKPT|,-(condition)`

**Example:**
```bash
tstskip_block_begin "Docker not installed"
```

**Output:**
```
   30 SKPT|,-(Docker not installed)
```

#### `tstskip_block_end`

Ends the skip block.

**Syntax:**
```bash
tstskip_block_end
tstskip_block_end <line_number>
```

**Parameters:**
- `line_number` - Optional explicit line number

**Behavior:**
- Prints skip block end marker: `  LINE     |\`---`

**Example:**
```bash
tstskip_block_end
```

**Output:**
```
   30     |`---
```

### Output Functions

#### `tstnote`

Prints an informational note.

**Syntax:**
```bash
tstnote <message>
```

**Parameters:**
- `message` - String message to print

**Behavior:**
- Auto-captures line number
- Prints: `  LINE NOTE: message`

**Example:**
```bash
tstnote "Testing with config file: $config"
```

**Output:**
```
   35 NOTE: Testing with config file: production.ini
```

#### `tstclock`

Records timing information.

**Syntax:**
```bash
tstclock <elapsed_value> <time_unit> <description>
```

**Parameters:**
- `elapsed_value` - Numeric time value
- `time_unit` - "n" (nanoseconds), "u" (microseconds), or "m" (milliseconds)
- `description` - What was timed

**Behavior:**
- Auto-captures line number
- Prints: `  LINE CLCK:  value units description`

**Example:**
```bash
start=$(date +%s%N)
sleep 1
end=$(date +%s%N)
elapsed=$(( (end - start) / 1000000 ))
tstclock "$elapsed" "m" "Sleep operation"
```

**Output:**
```
   40 CLCK:  1002 ms Sleep operation
```

#### `tstoutput_begin`

Starts an output capture block.

**Syntax:**
```bash
tstoutput_begin <description>
```

**Parameters:**
- `description` - Header for the output block

**Behavior:**
- Auto-captures line number
- Stores line for `tstoutput_end`
- Prints: `  LINE <<<<< description`

**Example:**
```bash
tstoutput_begin "Command output"
echo "Line 1"
echo "Line 2"
tstoutput_end
```

**Output:**
```
   45 <<<<< Command output
Line 1
Line 2
   45 >>>>>
```

#### `tstoutput_end`

Ends the output capture block.

**Syntax:**
```bash
tstoutput_end
tstoutput_end <line_number>
```

**Parameters:**
- `line_number` - Optional explicit line number

**Behavior:**
- Prints: `  LINE >>>>>`

#### `tstoutput_line`

Prints a line within an output block (without line number).

**Syntax:**
```bash
tstoutput_line <text>
```

**Parameters:**
- `text` - Line to print

**Behavior:**
- Prints text directly to stderr

**Example:**
```bash
tstoutput_begin "Data"
tstoutput_line "Item 1"
tstoutput_line "Item 2"
tstoutput_end
```

### Helper Functions

#### `tst_run`

Executes a command and checks its exit status.

**Syntax:**
```bash
tst_run <expression> <command> [args...]
```

**Parameters:**
- `expression` - Description of the command
- `command` - Command to execute
- `args` - Command arguments

**Returns:**
- Command's exit status

**Behavior:**
- Runs the command with provided arguments
- Calls `tstcheck` with expression and exit status
- Returns command's exit status

**Example:**
```bash
tst_run "List /tmp" ls /tmp
tst_run "Remove old files" rm -f /tmp/old*.txt
```

#### `tst_equal`

Compares two values for equality.

**Syntax:**
```bash
tst_equal <actual> <expected> <description>
```

**Parameters:**
- `actual` - Actual value
- `expected` - Expected value
- `description` - Test description

**Behavior:**
- If values are equal, calls `tstpass`
- Otherwise, calls `tstfail` with message showing both values

**Example:**
```bash
result=$(expr 2 + 2)
tst_equal "$result" "4" "2 + 2 == 4"
```

#### `tst_not_equal`

Checks that two values are not equal.

**Syntax:**
```bash
tst_not_equal <actual> <unexpected> <description>
```

**Parameters:**
- `actual` - Actual value
- `unexpected` - Value that should not match
- `description` - Test description

**Behavior:**
- If values differ, calls `tstpass`
- Otherwise, calls `tstfail`

**Example:**
```bash
tst_not_equal "$user" "root" "User is not root"
```

#### `tst_greater`

Numeric greater-than comparison.

**Syntax:**
```bash
tst_greater <actual> <threshold> <description>
```

**Parameters:**
- `actual` - Numeric value to test
- `threshold` - Value to compare against
- `description` - Test description

**Behavior:**
- If `actual > threshold`, calls `tstpass`
- Otherwise, calls `tstfail` with message showing both values

**Example:**
```bash
count=$(ls /tmp | wc -l)
tst_greater "$count" "0" "Temp directory not empty"
```

#### `tst_less`

Numeric less-than comparison.

**Syntax:**
```bash
tst_less <actual> <threshold> <description>
```

**Parameters:**
- `actual` - Numeric value to test
- `threshold` - Value to compare against
- `description` - Test description

**Behavior:**
- If `actual < threshold`, calls `tstpass`
- Otherwise, calls `tstfail` with message showing both values

**Example:**
```bash
size=$(du -b file.txt | cut -f1)
tst_less "$size" "1000000" "File under 1MB"
```

### State Variables

The library maintains internal state accessible via global variables:

**Suite-Level Counters:**

| Variable | Type | Description |
|----------|------|-------------|
| `TST_TOTAL_PASS` | integer | Total passed checks in suite |
| `TST_TOTAL_FAIL` | integer | Total failed checks in suite |
| `TST_TOTAL_SKIP` | integer | Total skipped checks in suite |

**Case-Level Counters:**

| Variable | Type | Description |
|----------|------|-------------|
| `TST_CASE_PASS` | integer | Passed checks in current case |
| `TST_CASE_FAIL` | integer | Failed checks in current case |
| `TST_CASE_SKIP` | integer | Skipped checks in current case |

**Other State:**

| Variable | Type | Description |
|----------|------|-------------|
| `TST_ABORT` | integer | 1 if suite aborted, 0 otherwise |
| `TST_CASE_LINE` | integer | Line number from `tstcase_begin` |
| `TST_SECTION_LINE` | integer | Line number from section/skip/output begin |
| `TST_SUITE_TITLE` | string | Suite title |
| `TST_SUITE_FILE` | string | Suite filename |
| `TST_SUITE_START_TIME` | string | Suite start timestamp |

**Example:**
```bash
tstcase_begin "Status check"
  tstpass "Check 1"
  tstpass "Check 2"
  
  tstnote "This case has $TST_CASE_PASS passes so far"
tstcase_end
```

### Utility Functions

#### `tst_timestamp`

Returns current timestamp in TST format.

**Syntax:**
```bash
tst_timestamp
```

**Returns:**
- String in format: `YYYY-MM-DD HH:MM:SS`

**Example:**
```bash
timestamp=$(tst_timestamp)
echo "Current time: $timestamp"
```

#### `tst_fprintf`

Printf-like output to stderr.

**Syntax:**
```bash
tst_fprintf <format> [args...]
```

**Parameters:**
- `format` - printf format string
- `args` - format arguments

**Behavior:**
- Prints to stderr using printf

**Example:**
```bash
tst_fprintf "Testing with value: %d\n" 42
```

#### `tst_format_lineno`

Formats a line number as 5-character right-aligned field.

**Syntax:**
```bash
tst_format_lineno <line_number>
```

**Parameters:**
- `line_number` - Integer line number

**Returns:**
- String formatted as 5 characters, right-aligned

**Example:**
```bash
formatted=$(tst_format_lineno 42)
# Returns: "   42"
```

### Line Number Handling

The library supports two modes for line number capture:

**Auto-Detect Mode (Default):**
```bash
tstpass "expression"
tstfail "expression" "message"
tstnote "message"
```

Functions use `${BASH_LINENO[0]}` to automatically capture the caller's line number.

**Explicit Mode:**
```bash
tstpass $LINENO "expression"
tstfail $LINENO "expression" "message"
```

Pass `$LINENO` as the first argument for explicit line number control. This is useful in helper functions where auto-detect would show the line inside the helper instead of the caller.

**Detection Logic:**
- If first argument matches regex `^[0-9]+$` and enough arguments provided, treat as explicit line number
- Otherwise, use `${BASH_LINENO[0]}` for auto-detect

---

## Command-Line Options

Test executables generated by `tstsuite` accept the following command-line options:

**Usage:**
```bash
./test_program [OPTIONS] [TAG_FILTERS]
```

**Options:**

| Option | Description |
|--------|-------------|
| `--help` | Show help message with usage and available tags |
| `--report-error` | Return non-zero exit code if any tests fail |
| `--list` | List all test cases with their tags and exit |

**Tag Filters:**

| Filter | Description |
|--------|-------------|
| `+Tag` | Enable tests with `+Tag`, disable tests with `-Tag` |
| `-Tag` | Disable tests with `+Tag`, enable tests with `-Tag` |
| `+*` | Enable all tests with any `+Tag` (not `-Tag` tests) |

**Filter Processing:**
- Processed left-to-right
- Later filters override earlier ones
- Untagged tests always run regardless of filters

**Examples:**
```bash
# Show help
./test_program --help

# List test cases and tags
./test_program --list

# Run with error reporting (for CI)
./test_program --report-error

# Enable database tests
./test_program +RequiresDB

# Enable all tests except slow ones
./test_program +* -SlowTests

# Enable database and network tests
./test_program +RequiresDB +RequiresNet

# Multiple filters (right overrides left)
./test_program +* -SlowTests +Integration
```

**CI/CD Integration:**
```bash
#!/bin/bash
./test_program --report-error
if [ $? -ne 0 ]; then
    echo "Tests failed!"
    exit 1
fi
echo "All tests passed!"
```

---

## Environment Variables

**TSTOPTIONS:**

Default command-line options for all test executables.

**Syntax:**
```bash
export TSTOPTIONS="--report-error +RequiresDB"
./test_program  # Uses options from TSTOPTIONS
```

**Use Cases:**
- CI/CD environments with common settings
- Development workflows with standard tag filters
- Automated test runs with consistent configuration

**Example:**
```bash
# Set defaults for CI environment
export TSTOPTIONS="--report-error +* -ManualTests"

# Run multiple test suites
./test_suite1  # Uses TSTOPTIONS
./test_suite2  # Uses TSTOPTIONS
```

**Notes:**
- Command-line options override TSTOPTIONS
- Empty or unset TSTOPTIONS is valid (no defaults)

---

## Output Format Specification

TST produces structured, line-oriented output designed for both human readability and machine parsing.

### Log Format

**Line Format:**
```
<linenum> <marker> <content>
```

**Markers:**

| Marker | Description | Example |
|--------|-------------|---------|
| `SUIT /` | Suite start | `----- SUIT / test.c "Suite Title" 2025-11-27 10:30:45` |
| `RSLT \` | Suite end (success) | `^^^^^ RSLT \ 1 FAIL \| 5 PASS \| 2 SKIP 2025-11-27 10:30:50` |
| `ABRT \` | Suite end (aborted) | `^^^^^ ABRT \ 1 FAIL \| 2 PASS \| 0 SKIP 2025-11-27 10:30:45` |
| `CASE,--` | Test case start | `    8 CASE,-- Test Name` |
| `` `--- `` | Test case end | `    8     \`--- 1 FAIL \| 2 PASS \| 0 SKIP` |
| `SCTN\|,--` | Section start | `   10 SCTN\|,-- Section Description` |
| `` \|`--- `` | Section end | `       \|`---` |
| `PASS\|` | Passed check | `   15 PASS\|  x > 0` |
| `FAIL\|` | Failed check | `   16 FAIL\|  y == 5 "Expected 5, got 7"` |
| `SKIP\|` | Skipped check | `   17 SKIP\|  z != NULL` |
| `SKPT\|` | Skipped section | `   18 SKPT\|,-(condition)` |
| `NOTE:` | Informational note | `   20 NOTE: Testing with user_id=42` |
| `CLCK:` | Clock measurement | `   25 CLCK:  1250 ms Operation description` |

**Line Number Format:**
- Right-aligned, 5 characters wide
- Shows source line where check appears
- Zero for suite-level output

**Tree Structure:**
- ASCII art using `|`, `-`, `` ` ``, and `╰` characters
- Indentation preserves hierarchical structure
- Parsers should preserve leading whitespace

### Example Output

```
----- SUIT / test_example.c "Example Suite" 2025-11-27 10:30:45
    8 CASE,-- Arithmetic Tests
    9 PASS|  1 + 1 == 2
   10 FAIL|  2 * 2 == 5 "Expected 5, got 4"
    8     `--- 1 FAIL | 1 PASS | 0 SKIP
   15 CASE,-- String Tests
   16 NOTE: Testing with input: "hello"
   17 PASS|  strlen("hello") == 5
   15     `--- 0 FAIL | 1 PASS | 0 SKIP
   22 CLCK:  125 µs Performance test
^^^^^ RSLT \ 1 FAIL | 2 PASS | 0 SKIP 2025-11-27 10:30:50
```

**Parsing Guidelines:**
- Use marker prefixes for line classification
- Preserve all whitespace (structural meaning)
- Extract counts from `RSLT`/`ABRT` and case-end lines
- Line numbers are informational (not guaranteed sequential)

---

## Best Practices

### Test Organization

**DO:**
- Use one suite per source file
- Group related tests in cases
- Use descriptive names for suites, cases, and sections
- Keep tests focused and independent

**DON'T:**
- Nest test cases (use sections instead)
- Rely on test execution order
- Use global state without proper reset

**Example:**
```c
tstsuite("String Parser") {
    tstcase("Valid Input") {
        tstcheck(parse("abc") != NULL);
    }
    
    tstcase("Invalid Input") {
        tstcheck(parse("") == NULL);
    }
    
    tstcase("Edge Cases") {
        tstcheck(parse("a") != NULL);
        tstcheck(parse("x" * 1000) != NULL);
    }
}
```

### Assertion Messages

**DO:**
- Add messages for non-obvious failures
- Include actual and expected values
- Keep messages concise and actionable

**DON'T:**
- Repeat what's obvious from the expression
- Use generic messages like "Failed"

**Good:**
```c
tstcheck(result == expected, "Expected %d, got %d", expected, result);
tstcheck(ptr != NULL, "Allocation failed for %d bytes", size);
```

**Bad:**
```c
tstcheck(x == 5, "x should equal 5");  // Obvious from expression
tstcheck(result, "Failed");            // Not actionable
```

### Test Independence

**DO:**
- Make each test independent
- Use sections for isolation
- Reset state between tests

**Example:**
```c
tstcase("Independent Tests") {
    int *data = allocate_data();
    
    tstsection("Test A") {
        // data is fresh
        modify_data(data, A);
        tstcheck(verify_data(data, A));
    }
    
    tstsection("Test B") {
        // data is reset
        modify_data(data, B);
        tstcheck(verify_data(data, B));
    }
    
    free_data(data);
}
```

### Data-Driven Testing

**DO:**
- Use meaningful test data
- Document edge cases
- Keep data arrays readable

**Example:**
```c
struct {int input; int output; const char *desc;} tstdata[] = {
    {0,    1,   "Zero"},
    {1,    1,   "One"},
    {5,    120, "Normal case"},
    {-1,   0,   "Negative (error)"},
    {20,   -1,  "Overflow"}
};

tstsection("Test with %s", tstcurdata.desc) {
    int result = factorial(tstcurdata.input);
    tstcheck(result == tstcurdata.output,
             "factorial(%d) = %d, expected %d",
             tstcurdata.input, result, tstcurdata.output);
}
```

### Tag Organization

**DO:**
- Use consistent tag naming
- Document tag meanings
- Create logical tag hierarchies

**Common Tags:**
- `Unit` - Fast, isolated tests
- `Integration` - Tests with dependencies
- `Slow` - Long-running tests
- `Manual` - Require manual setup
- `RequiresDB`, `RequiresNet` - Resource requirements

**Example:**
```c
tstsuite("Complete Test Suite",
         Unit, Integration, Slow, RequiresDB) {
    
    tstcase("Fast test", +Unit) {
        tstcheck(quick_function() == 0);
    }
    
    tstcase("Database test", +Integration, +RequiresDB) {
        tstcheck(db_query() == 0);
    }
}
```

### Error Handling

**DO:**
- Use `tstassert` for fatal errors
- Provide context in error messages
- Clean up resources on failure

**Example:**
```c
FILE *f = fopen("test.txt", "r");
tstassert(f != NULL, "Failed to open test.txt: %s", strerror(errno));

char buf[100];
tstcheck(fgets(buf, sizeof(buf), f) != NULL);

fclose(f);  // Always clean up
```

### Performance Testing

**DO:**
- Run multiple iterations
- Compare relative performance
- Document expectations

**Example:**
```c
tstclock("Algorithm A") {
    for (int i = 0; i < 1000; i++)
        algorithm_a(data);
}
clock_t time_a = tstelapsed();

tstclock("Algorithm B") {
    for (int i = 0; i < 1000; i++)
        algorithm_b(data);
}
clock_t time_b = tstelapsed();

tstcheck(time_b < time_a * 1.5,
         "Algorithm B should be within 50%% of A");
```

---

## Platform Support

### Tested Platforms

| Platform | Compiler | Status |
|----------|----------|--------|
| Linux | gcc | ✅ Fully supported |
| Linux | g++ | ✅ Fully supported |
| Linux | clang | ✅ Fully supported |
| Windows WSL2 | gcc | ✅ Fully supported |
| Windows WSL2 | g++ | ✅ Fully supported |
| Windows WSL2 | clang | ✅ Fully supported |
| Windows | cl (MSVC C) | ✅ Fully supported |
| Windows | cl (MSVC C++) | ✅ Fully supported |
| Windows | mingw-gcc | ✅ Fully supported |
| macOS | clang | ✅ Fully supported |

### Compiler Requirements

**C Compilers:**
- C99 or later recommended
- C89 compatible with minor limitations

**C++ Compilers:**
- C++98 or later
- `extern "C"` linkage handled automatically

### Build Instructions

**tst.h (Header-Only):**
```bash
# No build required, just include
gcc -o test test.c
```

**t2h Utility:**
```bash
cd src
make t2h

# Or manually
gcc -o t2h t2h.c -lm
```

### Platform-Specific Notes

**Windows (MSVC):**
- Use `/W3` or `/W4` for warnings
- Some MSVC warnings disabled in header
- Test with both C and C++ compilation modes

**Windows (MinGW):**
- Use POSIX-compatible paths in logs
- Works with both 32-bit and 64-bit

**macOS:**
- Use clang (Apple's default)
- Works with Xcode command-line tools

---

**End of Reference Manual**

For detailed tutorials and examples, see the [Programmer's Manual](prog_manual.md).  
For project information, see [README.md](../README.md).  
For internal architecture and maintenance notes, see the maintainer docs in `docs/maintainer/`.
