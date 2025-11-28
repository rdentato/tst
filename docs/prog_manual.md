# TST Library Programmer's Manual

## Overview

TST is a lightweight, header-only C testing framework designed for simplicity, flexibility, and zero dependencies. It enables developers to create organized test suites with minimal setup, clear output formatting, and powerful features for data-driven testing, performance measurement, and selective test execution.

**Version:** 0.8.1-beta  
**License:** MIT  
**Language:** C (C++ compatible)

### Key Features

- **Single Header File** - Just include `tst.h`, no configuration needed
- **Zero Dependencies** - Uses only standard C library
- **Data-Driven Testing** - Iterate tests over arrays of test data
- **Fuzzing Support** - Easy random test data generation
- **Performance Timing** - Built-in CPU time measurement
- **Selective Execution** - Tag-based filtering and conditional skipping
- **Flexible Output** - Structured logs for easy parsing
- **Cross-Platform** - Works on Linux, macOS, Windows (with any C compiler)
- **Hierarchical Organization** - Test cases and sections for structured testing

## Table of Contents

1. [Getting Started](#getting-started)
2. [Creating a Test Suite](#creating-a-test-suite)
3. [Test Cases and Sections](#test-cases-and-sections)
4. [Assertions and Checks](#assertions-and-checks)
5. [Data-Driven Testing](#data-driven-testing)
6. [Fuzzing and Random Testing](#fuzzing-and-random-testing)
7. [Control Flow and Conditional Execution](#control-flow-and-conditional-execution)
8. [Tags and Selective Test Execution](#tags-and-selective-test-execution)
9. [Output and Reporting](#output-and-reporting)
10. [HTML Test Reports with t2h](#html-test-reports-with-t2h)
11. [Performance Testing and Timing](#performance-testing-and-timing)
12. [Command Line Options](#command-line-options)
13. [Environment Variables](#environment-variables)
14. [Advanced Techniques](#advanced-techniques)
15. [Best Practices](#best-practices)
16. [Complete Examples](#complete-examples)
17. [API Reference](#api-reference)

---

## Getting Started

### Installation

TST is a header-only library. Simply copy `tst.h` to your project and include it:

```c
#include "tst.h"
```

Ensure the header is in your compiler's include path (e.g., using `-I` flag).

### Minimal Example

```c
#include "tst.h"

tstsuite("My First Test Suite") {
    tstcheck(1 + 1 == 2, "Basic arithmetic should work");
    tstcheck(5 * 4 == 20, "Multiplication should work");
}
```

Compile and run:

```bash
gcc -o test_program test.c
./test_program
```

Output:

```
----- SUIT / test.c "My First Test Suite" 2024-01-15 10:30:45
    4 PASS|  1 + 1 == 2
    5 PASS|  5 * 4 == 20
^^^^^ RSLT \ 0 FAIL | 2 PASS | 0 SKIP 2024-01-15 10:30:45
```

---

## Creating a Test Suite

### Basic Suite Definition

The `tstsuite` macro creates a test suite and generates the `main()` function:

```c
tstsuite("Suite Title") {
    // Your tests go here
}
```

**Important**: 
- Do not define your own `main()` function
- Only one `tstsuite` per source file
- The suite title is displayed in the output

### Suite with Tags

Define up to 8 tags for selective test execution:

```c
tstsuite("Database Tests", SlowTests, RequiresDB, Integration) {
    // Tests that can be selectively enabled/disabled
}
```

### Disabled Suite

Use `tst_suite` (with underscore) to disable an entire suite at compile time:

```c
tst_suite("Work In Progress") {
    // This entire suite is disabled - no code is executed
    tstcheck(experimental_feature() == 0);
}
```

This is useful for:
- Work-in-progress features
- Temporarily disabled tests
- Debugging specific test sets

---

## Test Cases and Sections

### Test Cases (`tstcase`)

Test cases group related assertions and provide partial result reporting:

```c
tstsuite("Math Library") {
    tstcase("Addition Tests") {
        tstcheck(1 + 1 == 2);
        tstcheck(5 + 3 == 8);
        tstcheck(-1 + 1 == 0);
    }
    
    tstcase("Multiplication Tests") {
        tstcheck(2 * 3 == 6);
        tstcheck(0 * 100 == 0);
    }
}
```

Output shows pass/fail counts for each case:

```
CASE,-- Addition Tests
   5 PASS|  1 + 1 == 2
   6 PASS|  5 + 3 == 8
   7 PASS|  -1 + 1 == 0
    `--- 0 FAIL | 3 PASS | 0 SKIP
```

**Note**: Test cases cannot be nested. If you need hierarchical organization within a case, use `tstsection` instead.

### Sections (`tstsection`)

Sections provide **setup/teardown isolation** within a test case. Each section re-executes the case from the beginning:

```c
tstcase("File Operations") {
    FILE *f = fopen("test.dat", "r");
    tstassert(f != NULL, "File should open");
    
    tstsection("Read first 5 bytes") {
        char buf[5];
        tstcheck(fread(buf, 1, 5, f) == 5);
    }
    
    tstsection("Check file size") {
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        tstcheck(size > 0);
    }
    
    // Cleanup runs after each section
    fclose(f);
}
```

**How sections work:**
1. Setup code runs (file open)
2. First section executes
3. Cleanup code runs (file close)
4. Setup code runs again
5. Second section executes
6. Cleanup code runs again

This ensures each section starts with a fresh state.

### Printf-Style Formatting

### Test Case Descriptions

Test case descriptions are simple strings:

```c
tstcase("Basic arithmetic test") {
    tstcheck(1 + 1 == 2);
}

// With tags
tstcase("Database connection test", +RequiresDB) {
    tstcheck(db_connect() == 0);
}
```

Note: `tstsection` descriptions support printf-style formatting:

```c
int test_value = 42;
tstsection("Section for test %d (expected: %d)", test_value, 42) {
    tstcheck(test_value == 42);
}
```

---

## Assertions and Checks

### Core Assertion Macros

#### `tst(expression)` - Silent Test

Records test result without printing output:

```c
tst(ptr != NULL);
if (tstfailed()) {
    // Handle failure manually
    cleanup();
}
```

#### `tstcheck(condition, message, ...)` - Standard Check

Main assertion macro with optional formatted message:

```c
tstcheck(x > 0);                                    // Simple check
tstcheck(x > 0, "x should be positive");           // With message
tstcheck(x == 42, "Expected 42, got %d", x);       // With formatting
```

Output on success:
```
  10 PASS|  x > 0
```
Output on failure:
```
  10 FAIL|  x > 0 "Expected 42, got 37"
```


#### `tstexpect(condition, message, ...)` - Silent on Pass

Like `tstcheck` but only prints output on **failure**:

```c
// Useful for high-volume assertions
for (int i = 0; i < 1000; i++) {
    tstexpect(array[i] >= 0, "Negative value at index %d", i);
}
```

This keeps logs clean when you have many passing tests.

#### `tstassert(condition, message, ...)` - Critical Check

Aborts the entire test suite on failure:

```c
void *ptr = malloc(1024);
tstassert(ptr != NULL, "Out of memory - cannot continue");
// If malloc fails, test suite exits immediately
```

Use for unrecoverable errors where continuing is pointless.

### Disabled Checks

Add underscore to disable checks at compile time:

```c
tstcheck(expensive_check());    // Enabled
tst_check(expensive_check());   // Disabled (no code generated)
```

### Result Query Functions

Check the result of the last test:

```c
tstcheck(complex_operation());

if (tstpassed()) {
    // Continue with dependent operation
    verify_result();
}

if (tstfailed()) {
    // Cleanup on failure
    rollback_changes();
}

if (tstskipped()) {
    // Handle skipped test
    log_skip_reason();
}
```

### Best Practices for Assertions

**DO:**
```c
// Keep expressions clear in the check
tstcheck(factorial(5) == 120, "5! should be 120");

// Use assignment within check to avoid recalculation
tstcheck((result = factorial(5)) == 120, "Got %d", result);

// Add descriptive strings to make the log more explanatory
// The string literal helps you remember what the test is about
tstcheck("Database connection" && (db != NULL));
int x = factorial(5);
tstcheck("Checking factorial" && x == 120);

// How it works: In C, a non-empty string literal is always true,
// so "text" && condition is equivalent to condition, but the
// output will display the full expression including the string,
// making the log more readable and self-documenting.
```

**DON'T:**
```c
// Don't hide the actual test
int x = factorial(5);
tstcheck(x == 120);  // Output shows "x == 120", not "factorial(5) == 120"
```

---

## Data-Driven Testing

Data-driven testing allows you to run the same tests with different input values, reducing code duplication and improving test coverage.

### Basic Data-Driven Tests

Define an array named `tstdata` in your test case:

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

The section runs **once for each element** in `tstdata`. `tstcurdata` accesses the current element.

### Struct Data

Use structs for complex test data:

```c
tstcase("String Validation") {
    struct {
        const char *input;
        int expected_length;
        int valid;
    } tstdata[] = {
        {"hello",    5, 1},
        {"world",    5, 1},
        {"",         0, 1},
        {NULL,       0, 0},
        {"test123",  7, 1}
    };
    
    tstsection("Validate string") {
        tstnote("Testing: \"%s\"", tstcurdata.input);
        
        if (tstcurdata.valid) {
            tstcheck(strlen(tstcurdata.input) == tstcurdata.expected_length);
        } else {
            tstcheck(tstcurdata.input == NULL);
        }
    }
}
```

### Multiple Sections with Same Data

Each section iterates over the entire dataset:

```c
tstcase("Comprehensive Validation") {
    struct {const char *str; int len;} tstdata[] = {
        {"abc", 3}, {"hello", 5}, {"x", 1}
    };
    
    tstsection("Check length") {
        tstcheck(strlen(tstcurdata.str) == tstcurdata.len);
    }
    
    tstsection("Check non-empty") {
        tstcheck(tstcurdata.str[0] != '\0');
    }
}
```

Output:
```
SCTN|,-- Check length
  - Tests for "abc"
  - Tests for "hello"  
  - Tests for "x"
SCTN|,-- Check non-empty
  - Tests for "abc"
  - Tests for "hello"
  - Tests for "x"
```

### Data Size Information

Access array size with `tst_data_size`:

```c
int tstdata[] = {1, 2, 3, 4, 5};

tstsection("Process data") {
    tstnote("Processing %d of %d items", 
            tst_data_count + 1, tst_data_size);
    tstcheck(tstcurdata > 0);
}
```

---

## Fuzzing and Random Testing

Fuzzing tests your code with random or semi-random inputs to discover edge cases.

### Basic Fuzzing

Generate random test data before sections execute:

```c
#include <stdlib.h>
#include <time.h>

tstsuite("Fuzzing Tests") {
    srand(time(NULL));
    
    tstcase("Random Integer Tests") {
        int tstdata[100];
        
        // Generate random data
        for (int i = 0; i < 100; i++) {
            tstdata[i] = rand() % 1000;
        }
        
        tstsection("Check bounds") {
            tstexpect(tstcurdata >= 0 && tstcurdata < 1000,
                     "Value %d out of bounds", tstcurdata);
        }
    }
}
```

### Multi-Round Fuzzing

Run multiple fuzzing cycles:

```c
tstcase("Fuzz Parser") {
    char tstdata[10][256];
    
    for (int round = 0; round < 10; round++) {
        // Generate new random strings each round
        for (int i = 0; i < 10; i++) {
            int len = rand() % 255;
            for (int j = 0; j < len; j++) {
                tstdata[i][j] = 32 + (rand() % 95);  // Printable ASCII
            }
            tstdata[i][len] = '\0';
        }
        
        tstsection("Parse random string") {
            // Parser should not crash regardless of input
            int result = parse_string(tstcurdata);
            tstexpect(result >= 0, "Parser failed on: %s", tstcurdata);
        }
    }
}
```

### Constrained Random Testing

Generate random data within specific constraints:

```c
tstcase("Constrained Random") {
    struct {int x; int y;} tstdata[50];
    
    // Generate points in first quadrant
    for (int i = 0; i < 50; i++) {
        tstdata[i].x = rand() % 100;
        tstdata[i].y = rand() % 100;
    }
    
    tstsection("Validate quadrant") {
        tstexpect(tstcurdata.x >= 0 && tstcurdata.y >= 0,
                 "Point (%d,%d) not in first quadrant",
                 tstcurdata.x, tstcurdata.y);
    }
}
```

### Displaying Fuzz Data

Use `tstouterr` to show generated data:

```c
tstcase("Show Generated Data") {
    int tstdata[5];
    
    tstouterr("Generated test data:") {
        for (int i = 0; i < 5; i++) {
            tstdata[i] = rand() % 100;
            tstprintf("[%d] = %d\n", i, tstdata[i]);
        }
    }
    
    tstsection("Test data") {
        tstcheck(tstcurdata >= 0 && tstcurdata < 100);
    }
}
```

---

## Control Flow and Conditional Execution

### Conditional Skipping (`tstskipif`)

Skip tests based on runtime conditions:

```c
tstcase("Database Tests") {
    int db_available = connect_to_database();
    
    tstskipif(!db_available) {
        // These tests only run if database is available
        tstcheck(query_user(1) != NULL);
        tstcheck(query_user(2) != NULL);
    }
    
    // This always runs
    tstcheck(db_available >= 0, "Connection attempt should not error");
}
```

Skipped tests are counted separately:

```
  15 SKIP|  query_user(1) != NULL
  16 SKIP|  query_user(2) != NULL
```

### Multiple Skip Conditions

```c
tstskipif(!network_available) {
    tstcheck(ping_server());
}

tstskipif(!database_available) {
    tstcheck(query_database());
}

// Always runs
tstcheck(1 == 1);
```

### Nested Skip Conditions

```c
tstskipif(!feature_enabled) {
    tstskipif(!debug_mode) {
        // Only runs if feature_enabled AND debug_mode
        tstcheck(debug_feature_test());
    }
}
```

---

## Tags and Selective Test Execution

Tags enable selective execution of test cases based on categories like speed, resource requirements, or test type. Tags are specified directly in test case declarations using bare identifiers (no quotes needed).

### Tag Syntax

Tags use `+` or `-` prefixes:
- **`+Tag`**: Test requires this capability/feature to be enabled
- **`-Tag`**: Test runs when this capability/feature is disabled

```c
tstcase("Test name", +Tag1, +Tag2)   // Requires Tag1 and Tag2
tstcase("Test name", -SlowTests)     // Runs when SlowTests disabled
tstcase("Test name")                 // No tags (always runs)
```

### Basic Example

```c
tstsuite("API Tests") {
    
    // This test always runs (no tags)
    tstcase("Basic API Check") {
        tstcheck(api_version() == 1);
    }
    
    // Only runs when RequiresDB is enabled on command line
    tstcase("Database Test", +RequiresDB) {
        tstcheck(db_connect() == 0);
    }
    
    // Only runs when SlowTests is disabled on command line
    tstcase("Fast Test", -SlowTests) {
        tstcheck(quick_check() == 1);
    }
    
    // Multiple tags
    tstcase("Full Integration", +RequiresDB, +RequiresNet, +SlowTests) {
        tstcheck(full_integration_test() == 0);
    }
}
```

### Command Line Tag Filtering

Tags are controlled via command line arguments:

```bash
# Run all tests (untagged run, tagged tests skipped by default)
./test_program

# List all tests with their tags
./test_program --list

# Enable all tests with +RequiresDB
./test_program +RequiresDB

# Enable tests with -SlowTests (disable tests with +SlowTests)
./test_program -SlowTests

# Enable ALL +Tag tests (but not -Tag tests)
./test_program +*

# Combine filters: later filters override earlier ones
./test_program +* -SlowTests    # Enable all, then enable -SlowTests tests
./test_program +RequiresDB +SlowTests   # Enable both
```

### Tag Semantics

**Key Rules:**

1. **Untagged tests always run** - They are immune to all tag filters
2. **Tagged tests are disabled by default** - Must be explicitly enabled
3. **Filters are processed left-to-right** - Later filters override earlier ones
4. **`+*` is special** - Enables all `+Tag` tests (but not `-Tag` tests)

**Filter Behavior:**

| Command Line | Effect |
|--------------|--------|
| `+TAG` | Enable tests with `+TAG`, disable tests with `-TAG` |
| `-TAG` | Disable tests with `+TAG`, enable tests with `-TAG` |
| `+*` | Enable all tests with any `+TAG` (not `-TAG`) |
| `+*, -TAG` | First enable all `+Tag` tests, then enable `-TAG` tests |

**Examples:**

```bash
# Enable database tests, disable non-database tests
./test_program +RequiresDB

# Skip slow tests, run fast tests
./test_program -SlowTests

# Run all tagged tests except slow ones
./test_program +* -SlowTests

# Override: first disable, then re-enable specific tag
./test_program -RequiresDB +RequiresDB    # Net result: RequiresDB enabled
```

### Common Tag Patterns

#### By Speed
```c
tstcase("Quick validation", -SlowTests) {
    tstcheck(quick_check());
}

tstcase("Comprehensive scan", +SlowTests) {
    for (int i = 0; i < 10000; i++) {
        tstcheck(thorough_check(i));
    }
}
```

Run only fast tests:
```bash
./test_program -SlowTests
```

#### By Resource Requirements
```c
tstcase("Offline test", -RequiresDB, -RequiresNet) {
    tstcheck(local_computation() == 42);
}

tstcase("Database test", +RequiresDB) {
    tstcheck(db_query() == 0);
}

tstcase("Network test", +RequiresNet) {
    tstcheck(api_call() == 200);
}
```

Run only tests that don't need database:
```bash
./test_program -RequiresDB
```

#### By Test Type
```c
tstcase("Unit test", +Unit) {
    tstcheck(function_under_test(5) == 25);
}

tstcase("Integration test", +Integration, +RequiresDB) {
    tstcheck(full_workflow() == SUCCESS);
}

tstcase("End-to-end test", +E2E, +RequiresDB, +RequiresNet) {
    tstcheck(complete_user_flow() == SUCCESS);
}
```

Run only unit tests:
```bash
./test_program +Unit
```

### Listing Tagged Tests

The `--list` option shows all test cases with their tags:

```bash
./test_program --list
```

Output:
```
"Basic API Check"
"Database Test" +RequiresDB
"Fast Test" -SlowTests
"Full Integration" +RequiresDB, +RequiresNet, +SlowTests
```

### Tag Truth Table

| Test Tags | No Filter | `+*` | `+TAG` | `-TAG` | `+*, -TAG` |
|-----------|-----------|------|--------|--------|------------|
| (no tags) | ✅ RUN | ✅ RUN | ✅ RUN | ✅ RUN | ✅ RUN |
| `+TAG` | ❌ SKIP | ✅ RUN | ✅ RUN | ❌ SKIP | ❌ SKIP |
| `-TAG` | ❌ SKIP | ❌ SKIP | ❌ SKIP | ✅ RUN | ✅ RUN |
| `+OTHER` | ❌ SKIP | ✅ RUN | ❌ SKIP | ❌ SKIP | ✅ RUN |

---

## Output and Reporting

### Standard Output Format

TST produces structured, hierarchical output:

```
----- SUIT / filename.c "Suite Title" 2024-01-15 10:30:45
CASE,-- Test Case Name
  123 PASS|  condition_check
  124 FAIL|  failed_condition "Error: expected 5, got 3"
    `--- 1 FAIL | 1 PASS | 0 SKIP
^^^^^ RSLT \ 1 FAIL | 1 PASS | 0 SKIP 2024-01-15 10:30:45
```

Components:
- **SUIT**: Suite header with filename, title, timestamp
- **CASE**: Test case marker with description
- **Line numbers**: Source line where check appears
- **Status**: PASS, FAIL, or SKIP
- **Expression**: The tested condition
- **Message**: Optional failure message
- **Case summary**: Partial results per case
- **RSLT**: Final suite results with timestamp

### Informational Output

#### `tstnote` - Inline Notes

Add contextual information to test output:

```c
tstnote("Testing with configuration: %s", config_name);
tstnote("Current user: %d", user_id);
```

Output:
```
  45 NOTE: Testing with configuration: production
```

#### `tstprintf` - Direct Output

Print directly to stderr (no formatting):

```c
tstprintf("Debug: x=%d, y=%d\n", x, y);
```

#### `tstouterr` - Delimited Output Blocks

Capture multi-line output with clear markers:

```c
tstouterr("API Response:") {
    tstprintf("Status: %d\n", response.status);
    tstprintf("Body: %s\n", response.body);
    tstprintf("Headers: %d\n", response.header_count);
}
```

Output:
```
  67 <<<<< API Response:
Status: 200
Body: {"result": "success"}
Headers: 5
  67 >>>>>
```

### Capturing Test Counts

Access global counters in your code:

```c
// Global counters
extern int tst_pass;  // Total passes
extern int tst_fail;  // Total failures
extern int tst_skip;  // Total skips

// Per-case counters (use inside tstcase)
extern short tst_case_pass;
extern short tst_case_fail;
extern short tst_case_skip;

tstsuite("Counter Example") {
    tstcheck(1 == 1);
    tstcheck(2 == 2);
    
    tstnote("So far: %d passes, %d fails", tst_pass, tst_fail);
}
```

---

## HTML Test Reports with t2h

The `t2h` tool converts TST log output into interactive HTML dashboards with visual statistics, charts, and detailed test logs.

### Basic Usage

#### Single Test File

Convert a single test log to HTML:

```bash
# From stdin
./test_program | t2h > report.html

# From file
t2h test.log > report.html
```

#### Multiple Test Files

Generate a consolidated report from multiple test logs:

```bash
t2h test1.log test2.log test3.log > consolidated.html
```

The consolidated report includes:
- Overall summary across all test files
- Cumulative statistics with percentages
- Individual sections for each test file
- Quick navigation between suites

### Features

#### Interactive Dashboard

The HTML report includes:

1. **Overall Summary**
   - Pie chart showing pass/fail distribution
   - Summary grid with counts and percentages
   - Total test count and execution time

2. **Test Statistics Card**
   - Total tests executed
   - Pass/fail/skip counts with percentages
   - Number of test cases

3. **Visual Progress Bars**
   - Color-coded segmented bars
   - Hover tooltips with exact counts
   - Pass rate and fail rate display

4. **Test Cases Detail**
   - Clickable test case sections
   - Expandable log viewer for each case
   - Color-coded log lines (PASS/FAIL/SKIP/NOTE)
   - Syntax highlighting

#### Visual Indicators

**Left Border Colors** - Quick status identification:
- **Red border** - Contains failed tests
- **Green border** - All tests passed
- **Blue border** - Mixed results or skipped tests

**Status Colors**:
- Green (#28a745) - Passed tests
- Red (#dc3545) - Failed tests
- Yellow (#ffc107) - Skipped tests

#### Percentage Calculations

**Important**: Percentages exclude skipped tests:
- Pass% = passed / (passed + failed) × 100
- Fail% = failed / (passed + failed) × 100
- Skipped tests show count only (no percentage)

This provides accurate pass/fail rates for actually executed tests.

#### Log Viewer

Click any test case section to expand its log details:
- Line numbers preserved
- Indentation maintained
- Syntax-highlighted output
- Color-coded test results
- Dark theme for readability

#### Pie Charts

Each summary includes a donut chart showing:
- Visual proportion of pass/fail/skip
- Tooltips with counts and percentages
- Full circle for 100% pass/fail cases
- Positioned left of summary grid

### Multi-File Reports

When processing multiple files, t2h generates:

1. **Consolidated Header**
   - Combined statistics from all files
   - List of all test suites
   - Clickable suite blocks for navigation
   - Color-coded status indicators

2. **Individual Suite Sections**
   - Full details for each test file
   - Separate statistics per suite
   - "Back to Top" navigation links
   - Unique IDs for proper JavaScript toggling

### Warnings and Limits

**Log Line Limit**: Each test suite can store up to 10,000 log lines.

When the limit is exceeded:
- Warning printed to stderr during conversion
- Yellow warning box displayed in HTML report
- Message: "Log exceeded 10000 lines. Some test case details may be incomplete."

### Example Workflow

```bash
# Run tests and save logs
./test_basics > logs/basics.log
./test_advanced > logs/advanced.log
./test_integration > logs/integration.log

# Generate consolidated HTML report
t2h logs/*.log > reports/test-report-$(date +%Y%m%d).html

# Open in browser
xdg-open reports/test-report-*.html
```

### Features Summary

- ✅ Single-file HTML output (no dependencies)
- ✅ Responsive design (works on mobile)
- ✅ Interactive expandable sections
- ✅ Pie charts and progress bars
- ✅ Color-coded status indicators
- ✅ Syntax-highlighted logs
- ✅ Multi-file consolidation
- ✅ Warning notifications
- ✅ Dark-themed log viewer
- ✅ Clickable navigation

### Building t2h

The t2h tool is included in the TST distribution:

```bash
cd src
make t2h
```

Requirements:
- C compiler (gcc, clang, etc.)
- Math library (links with `-lm`)

---

## Shell Testing with tst.sh

TST provides a bash library (`tst.sh`) for writing shell-based tests that produce TST-formatted output. This enables testing of shell scripts, system commands, and integration scenarios using the same test framework and HTML reporting as C tests.

### Overview

The `tst.sh` library mirrors the tst.h API but is implemented as bash functions. It generates the same log format, making test results compatible with `t2h` for HTML report generation.

**Key Features:**
- TST-compatible log output
- Automatic line number detection using `${BASH_LINENO[0]}`
- Optional explicit line numbers for helper functions
- Suite, case, and section organization
- Built-in helper functions for common patterns
- State management for pass/fail/skip counts

### Installation

The `tst.sh` library is a single bash script. Source it in your test scripts:

```bash
#!/bin/bash
source "/path/to/tst.sh"

# Your tests here
```

### Basic Usage

#### Minimal Example

```bash
#!/bin/bash
source "$(dirname "$0")/../src/tst.sh"

tstsuite_begin "Basic Shell Tests" "$0"

tstcase_begin "Simple checks"
  tstpass "1 + 1 == 2"
  tstfail "1 + 1 == 3" "Math is broken"
tstcase_end

tstsuite_end
```

#### Running the Test

```bash
chmod +x test_script.sh
./test_script.sh
```

Output:
```
----- SUIT / test_script.sh "Basic Shell Tests" 2025-11-28 10:30:45
    6 CASE,-- Simple checks
    7 PASS|  1 + 1 == 2
    8 FAIL|  1 + 1 == 3 "Math is broken"
    6     `--- 1 FAIL | 1 PASS | 0 SKIP
^^^^^ RSLT \ 1 FAIL | 1 PASS | 0 SKIP 2025-11-28 10:30:45
```

### Suite Management

#### `tstsuite_begin`

Starts a test suite and prints the suite header.

```bash
tstsuite_begin "Suite Title" "$0"
tstsuite_begin "Suite Title" "$0" "disabled"  # Disabled suite
```

**Parameters:**
- `suite_title` - Descriptive title for the suite
- `filename` - Source filename (use `$0`)
- `disabled` - Optional, adds "(disabled)" suffix

**Example:**
```bash
tstsuite_begin "Integration Tests" "$0"
```

#### `tstsuite_end`

Ends the test suite and prints final results.

```bash
tstsuite_end
```

Prints either `RSLT` (normal) or `ABRT` (if a test called `tstassert` and failed).

### Test Cases

#### `tstcase_begin`

Starts a test case.

```bash
tstcase_begin "Test case description"
```

Automatically captures the line number using `${BASH_LINENO[0]}`.

**Example:**
```bash
tstcase_begin "File operations"
  # Tests go here
tstcase_end
```

#### `tstcase_end`

Ends the current test case and prints partial results.

```bash
tstcase_end
tstcase_end "$line_number"  # Optional explicit line number
```

### Assertions

#### `tstpass`

Records a passing check.

```bash
tstpass "expression"           # Auto-detect line number
tstpass $LINENO "expression"   # Explicit line number
```

**Auto-Detect (Recommended):**
```bash
tstpass "test -f /etc/passwd"
```

**Explicit (For Helper Functions):**
```bash
check_file() {
  if [ -f "$1" ]; then
    tstpass $LINENO "File $1 exists"
  fi
}
```

#### `tstfail`

Records a failing check with an optional error message.

```bash
tstfail "expression" "message"    # Auto-detect line number
tstfail $LINENO "expression" "message"  # Explicit
```

**Examples:**
```bash
tstfail "test -f /missing" "File not found"
tstfail $LINENO "parse result" "Expected success, got error"
```

#### `tstskip`

Records a skipped check.

```bash
tstskip "expression"           # Auto-detect
tstskip $LINENO "expression"   # Explicit
```

#### `tstcheck`

Evaluates a command result and records pass/fail.

```bash
tstcheck "description" $exit_code "failure message"
```

**Example:**
```bash
ls /tmp > /dev/null 2>&1
tstcheck "ls /tmp succeeds" $? "Directory not found"
```

#### `tstassert`

Critical check that aborts the suite on failure.

```bash
tstassert "expression" $exit_code "failure message"
```

**Example:**
```bash
mkdir -p /tmp/testdir
tstassert "Create test directory" $? "Failed to create directory"
# Suite exits here if mkdir failed
```

### Sections

Sections group related checks within a test case.

#### `tstsection_begin`

```bash
tstsection_begin "Section description"
```

#### `tstsection_end`

```bash
tstsection_end
tstsection_end "$line_number"  # Optional explicit line
```

**Example:**
```bash
tstcase_begin "Configuration tests"
  tstsection_begin "Parse config file"
    result=$(parse_config config.ini)
    tstpass "Config parsed"
  tstsection_end
  
  tstsection_begin "Validate settings"
    tstpass "Settings valid"
  tstsection_end
tstcase_end
```

### Skip Blocks

Skip blocks conditionally skip groups of tests.

```bash
tstskip_block_begin "condition description"
  # Tests to skip
tstskip_block_end
```

**Example:**
```bash
if ! command -v docker &> /dev/null; then
  tstskip_block_begin "Docker not installed"
    tstskip "Docker daemon running"
    tstskip "Container can start"
  tstskip_block_end
fi
```

### Output and Notes

#### `tstnote`

Prints an informational note.

```bash
tstnote "Testing with config: $config_file"
```

#### `tstoutput_begin` / `tstoutput_end`

Captures multi-line output.

```bash
tstoutput_begin "Command output"
  echo "Line 1"
  echo "Line 2"
tstoutput_end
```

Output:
```
   15 <<<<< Command output
Line 1
Line 2
   15 >>>>>
```

#### `tstclock`

Records timing information.

```bash
tstclock "$elapsed" "m" "Operation description"
```

Parameters:
- `elapsed` - Numeric value
- `unit` - "n" (ns), "u" (µs), or "m" (ms)
- `description` - What was timed

**Example:**
```bash
start=$(date +%s%N)
sleep 1
end=$(date +%s%N)
elapsed=$(( (end - start) / 1000000 ))  # Convert to ms
tstclock "$elapsed" "m" "Sleep 1 second"
```

### Helper Functions

The library provides helper functions for common patterns.

#### `tst_run`

Runs a command and checks its exit status.

```bash
tst_run "description" command [args...]
```

**Example:**
```bash
tst_run "List temp dir" ls /tmp
tst_run "Remove old files" rm -f /tmp/old*.txt
```

#### `tst_equal`

Compares two values for equality.

```bash
tst_equal "$actual" "$expected" "description"
```

**Example:**
```bash
result=$(expr 2 + 2)
tst_equal "$result" "4" "2 + 2 == 4"
```

#### `tst_not_equal`

Checks that values are not equal.

```bash
tst_not_equal "$actual" "$unexpected" "description"
```

#### `tst_greater`

Numeric greater-than comparison.

```bash
tst_greater "$actual" "$threshold" "description"
```

**Example:**
```bash
count=$(ls /tmp | wc -l)
tst_greater "$count" "0" "Temp dir not empty"
```

#### `tst_less`

Numeric less-than comparison.

```bash
tst_less "$actual" "$threshold" "description"
```

### Complete Example

```bash
#!/bin/bash
source "$(dirname "$0")/../src/tst.sh"

tstsuite_begin "File System Tests" "$0"

tstcase_begin "Directory operations"
  tstnote "Testing basic directory operations"
  
  # Create test directory
  mkdir -p /tmp/tst_test
  tstassert "Create test dir" $? "Failed to create directory"
  
  # Verify it exists
  if [ -d /tmp/tst_test ]; then
    tstpass "Directory exists"
  else
    tstfail "Directory exists" "Not found"
  fi
  
  # Test with helper
  file_count=$(ls /tmp/tst_test | wc -l)
  tst_equal "$file_count" "0" "Directory is empty"
  
  # Cleanup
  rm -rf /tmp/tst_test
  tst_run "Cleanup test dir" rm -rf /tmp/tst_test
tstcase_end

tstcase_begin "Command execution"
  tstsection_begin "Echo test"
    output=$(echo "hello")
    tst_equal "$output" "hello" "Echo works"
  tstsection_end
  
  tstsection_begin "Date command"
    date > /dev/null
    tstcheck "Date command succeeds" $?
  tstsection_end
tstcase_end

tstsuite_end
```

### Line Number Handling

The library supports two modes for line numbers:

**Auto-Detect (Recommended):**
```bash
tstpass "test expression"
tstfail "test expression" "error message"
```

The function automatically captures the caller's line number using `${BASH_LINENO[0]}`.

**Explicit (For Helper Functions):**
```bash
my_check() {
  if [ -f "$1" ]; then
    tstpass $LINENO "File $1 exists"
  else
    tstfail $LINENO "File $1 exists" "Not found"
  fi
}
```

When you call a helper function, auto-detect would show the line number inside the helper, not where you called it. Use `$LINENO` to pass the caller's line number explicitly.

### Integration with t2h

Tests written with `tst.sh` produce TST-formatted logs that work with `t2h`:

```bash
# Run test and generate HTML report
./test_script.sh | t2h > report.html

# Multiple shell test logs
./test_system.sh > system.log
./test_integration.sh > integration.log
t2h system.log integration.log > consolidated.html
```

### Best Practices

**DO:**
- Use auto-detect for direct assertions
- Use explicit `$LINENO` in helper functions
- Add descriptive messages to failures
- Use helper functions for common patterns
- Clean up resources in test cases

**Example:**
```bash
tstcase_begin "Resource cleanup"
  temp_file=$(mktemp)
  tstassert "Create temp file" $? "mktemp failed"
  
  echo "test data" > "$temp_file"
  tstpass "Write to temp file"
  
  # Always cleanup
  rm -f "$temp_file"
  tst_run "Remove temp file" rm -f "$temp_file"
tstcase_end
```

**DON'T:**
```bash
# Bad: No cleanup
tstcase_begin "Bad example"
  temp_file=$(mktemp)
  echo "data" > "$temp_file"
  tstpass "Created file"
  # File leaked!
tstcase_end

# Bad: Generic error messages
tstfail "test" "Failed"  # Not helpful

# Bad: No line numbers in helper
my_helper() {
  tstpass "check"  # Line number will be inside helper, not caller
}
```

### State Variables

The library maintains internal state that can be accessed:

| Variable | Description |
|----------|-------------|
| `TST_TOTAL_PASS` | Total passes in suite |
| `TST_TOTAL_FAIL` | Total failures in suite |
| `TST_TOTAL_SKIP` | Total skips in suite |
| `TST_CASE_PASS` | Passes in current case |
| `TST_CASE_FAIL` | Failures in current case |
| `TST_CASE_SKIP` | Skips in current case |
| `TST_ABORT` | Set to 1 if suite aborted |

**Example:**
```bash
tstcase_begin "Conditional logic"
  tstpass "First check"
  tstpass "Second check"
  
  tstnote "Current case has $TST_CASE_PASS passes"
tstcase_end
```

---

## Performance Testing and Timing

### Basic Timing

Measure execution time of code blocks:

```c
tstclock("Sort 10000 elements") {
    sort_array(data, 10000);
}
```

Output shows elapsed time:
```
  42 CLCK:  1250 ms Sort 10000 elements
```

The time unit (ns, µs, ms) is automatically selected based on `CLOCKS_PER_SEC`.

### Comparing Performance

```c
tstsuite("Performance Comparison") {
    clock_t recursive_time, iterative_time;
    
    tstclock("Recursive factorial") {
        for (int i = 0; i < 100000; i++) {
            factorial_recursive(10);
        }
    }
    recursive_time = tstelapsed();
    
    tstclock("Iterative factorial") {
        for (int i = 0; i < 100000; i++) {
            factorial_iterative(10);
        }
    }
    iterative_time = tstelapsed();
    
    tstcase("Performance Check") {
        tstnote("Recursive: %ld ticks", recursive_time);
        tstnote("Iterative: %ld ticks", iterative_time);
        tstcheck(iterative_time <= recursive_time,
                "Iterative should be faster");
    }
}
```

### Timing with Data-Driven Tests

```c
tstcase("Sorting Performance") {
    int sizes[] = {10, 100, 1000, 10000};
    
    int tstdata[] = {10, 100, 1000, 10000};
    
    tstsection("Time sort") {
        int *arr = generate_random_array(tstcurdata);
        
        tstclock("Sort %d elements", tstcurdata) {
            sort_array(arr, tstcurdata);
        }
        
        tstcheck(is_sorted(arr, tstcurdata));
        free(arr);
    }
}
```

### Understanding Clock Measurements

**Important**: `tstclock` measures **CPU time**, not wall-clock time:

```c
tstclock("Sleep test") {
    sleep(2);  // Wall-clock: ~2000ms, CPU time: ~0ms
}

tstclock("CPU test") {
    for (int i = 0; i < 1000000; i++) {
        // CPU time will show actual computation time
        compute(i);
    }
}
```

---

## Command Line Options

### Syntax

```bash
./test_program [options] [tag-filters]
```

### Available Options

#### `--list`

List all test cases with their tags:

```bash
$ ./test_program --list
"Basic API Check"
"Database Test" +RequiresDB
"Fast Test" -SlowTests
"Full Integration" +RequiresDB, +RequiresNet, +SlowTests
```

This shows:
- Untagged tests (no tags shown)
- Tagged tests with their `+Tag` and `-Tag` markers
- Multiple tags separated by commas

#### `--report-error`

Return non-zero exit code on test failure:

```bash
./test_program --report-error
echo $?  # Will be non-zero if any test failed
```

Useful for CI/CD pipelines:

```bash
#!/bin/bash
./test_program --report-error
if [ $? -ne 0 ]; then
    echo "Tests failed!"
    exit 1
fi
```

### Tag Filters

Tag filters control which tagged tests execute. **Untagged tests always run regardless of filters.**

#### Enable Specific Tags

```bash
# Run tests with +RequiresDB
./test_program +RequiresDB

# Run tests with +SlowTests and +Integration
./test_program +SlowTests +Integration
```

#### Disable Specific Tags

```bash
# Run tests with -SlowTests (skip tests with +SlowTests)
./test_program -SlowTests

# Enable tests with -RequiresDB
./test_program -RequiresDB
```

#### Enable All Tagged Tests

```bash
# Enable all tests with any +Tag (but not -Tag tests)
./test_program +*
```

#### Combining Filters (Priority Matters!)

Filters are processed **left-to-right**, with later filters overriding earlier ones:

```bash
# Enable all +Tag tests, then also enable -SlowTests tests
./test_program +* -SlowTests

# Enable +RequiresDB, then override with -RequiresDB
./test_program +RequiresDB -RequiresDB    # Net: -RequiresDB wins
```

### Filter Examples

```bash
# Development: run only fast tests
./test_program -SlowTests

# CI: run all tests including slow ones
./test_program +*

# Integration testing: database and network tests
./test_program +RequiresDB +RequiresNet

# Pre-commit: fast offline tests only
./test_program -SlowTests -RequiresDB -RequiresNet

# Full suite with error reporting
./test_program +* --report-error
```

---

## Environment Variables

TST does not currently use environment variables for configuration. All options must be specified on the command line.

---

## Advanced Techniques

### Complex Sections with Setup/Teardown

```c
tstcase("Resource Management") {
    // Setup runs before EACH section
    void *resource = allocate_resource();
    tstassert(resource != NULL);
    
    tstsection("Test operation 1") {
        tstcheck(operation1(resource) == 0);
    }
    
    tstsection("Test operation 2") {
        tstcheck(operation2(resource) == 0);
    }
    
    // Cleanup runs after EACH section
    free_resource(resource);
}
```

### Parameterized Test Patterns

Create test generators:

```c
#define TEST_MATH_OP(op, a, b, expected) \
    tstcheck((a op b) == expected, \
             #a " " #op " " #b " should be %d", expected)

tstsuite("Math Operations") {
    tstcase("Addition") {
        TEST_MATH_OP(+, 2, 3, 5);
        TEST_MATH_OP(+, -1, 1, 0);
        TEST_MATH_OP(+, 0, 0, 0);
    }
}
```

### State Machine Testing

```c
tstcase("State Machine") {
    enum State {INIT, RUNNING, STOPPED} state = INIT;
    
    tstsection("Transition to RUNNING") {
        tstcheck(state == INIT);
        state = transition(state, START_EVENT);
        tstcheck(state == RUNNING);
    }
    
    tstsection("Transition to STOPPED") {
        // State resets to INIT due to section re-execution
        tstcheck(state == INIT);
        state = transition(state, START_EVENT);
        state = transition(state, STOP_EVENT);
        tstcheck(state == STOPPED);
    }
}
```

### Fixture Pattern

```c
// Common test fixture
struct Fixture {
    int *data;
    int size;
};

struct Fixture setup_fixture(int size) {
    struct Fixture f;
    f.data = malloc(size * sizeof(int));
    f.size = size;
    return f;
}

void teardown_fixture(struct Fixture *f) {
    free(f->data);
    f->data = NULL;
    f->size = 0;
}

tstsuite("Fixture Tests") {
    tstcase("Array Operations") {
        struct Fixture f = setup_fixture(100);
        
        tstsection("Test fill") {
            fill_array(f.data, f.size, 42);
            tstcheck(f.data[0] == 42);
            tstcheck(f.data[99] == 42);
        }
        
        tstsection("Test sort") {
            randomize_array(f.data, f.size);
            sort_array(f.data, f.size);
            tstcheck(is_sorted(f.data, f.size));
        }
        
        teardown_fixture(&f);
    }
}
```

---

## Best Practices

### 1. Test Organization

**DO:**
- One suite per source file
- Group related tests in cases
- Use descriptive names
- Keep tests focused and small

```c
// Good
tstsuite("String Parser") {
    tstcase("Basic Parsing") { /* ... */ }
    tstcase("Edge Cases") { /* ... */ }
    tstcase("Error Handling") { /* ... */ }
}
```

**DON'T:**
```c
// Bad - too generic
tstsuite("Tests") {
    tstcheck(everything());
}
```

### 2. Assertion Messages

**DO:**
- Add messages for non-obvious failures
- Include actual values in messages
- Keep messages concise

```c
tstcheck(result == expected, 
         "Expected %d, got %d", expected, result);
```

**DON'T:**
```c
// Bad - message repeats obvious information
tstcheck(x == 5, "x should equal 5");

// Bad - no context
tstcheck(result == 0, "Failed");
```

### 3. Test Independence

**DO:**
- Each test should be independent
- Use sections for setup/teardown
- Don't rely on test execution order

```c
// Good - each section is independent
tstsection("Test A") {
    setup();
    test_a();
    teardown();
}

tstsection("Test B") {
    setup();
    test_b();
    teardown();
}
```

### 4. Data-Driven Testing

**DO:**
- Use data-driven tests for variations
- Keep test data readable
- Document edge cases

```c
struct {int input; int output; const char *desc;} tstdata[] = {
    {0,    1,   "Zero"},
    {1,    1,   "One"},
    {5,    120, "Normal"},
    {-1,   0,   "Negative (error)"}
};
```

### 5. Tag Usage

**DO:**
- Use tags for test categorization
- Create consistent tag naming schemes
- Document tag meanings

```c
// Good tag organization
tstsuite("Tests", 
         Unit,        // Fast, isolated tests
         Integration, // Tests with dependencies
         Slow,        // Long-running tests
         Manual);     // Require manual setup
```

### 6. Error Messages

**DO:**
- Make failure messages actionable
- Include context and expected values
- Use `tstnote` for additional info

```c
tstnote("Testing with user_id=%d", user_id);
tstcheck(result != NULL, 
         "Failed to fetch user %d", user_id);
```

### 7. Performance Testing

**DO:**
- Run performance tests multiple times
- Compare relative performance
- Document performance expectations

```c
tstclock("Expected < 100ms") {
    operation();
}
tstcheck(tstelapsed() < 100000, 
         "Too slow: %ld ticks", tstelapsed());
```

---

## Complete Examples

### Example 1: Unit Testing a Parser

```c
#include "tst.h"
#include "parser.h"

tstsuite("JSON Parser", Unit, QuickTests) {
    
    tstcase("Valid JSON") {
        struct {
            const char *json;
            int expected_type;
        } tstdata[] = {
            {"{}", JSON_OBJECT},
            {"[]", JSON_ARRAY},
            {"123", JSON_NUMBER},
            {"\"text\"", JSON_STRING},
            {"true", JSON_BOOL},
            {"null", JSON_NULL}
        };
        
        tstsection("Parse valid input") {
            json_value_t *val = json_parse(tstcurdata.json);
            tstcheck(val != NULL, 
                    "Failed to parse: %s", tstcurdata.json);
            tstcheck(val->type == tstcurdata.expected_type,
                    "Wrong type for: %s", tstcurdata.json);
            json_free(val);
        }
    }
    
    tstcase("Invalid JSON") {
        const char *tstdata[] = {
            "{",
            "[1, 2",
            "\"unterminated",
            "undefined",
            "{\"key\" 123}"
        };
        
        tstsection("Reject invalid input") {
            json_value_t *val = json_parse(tstcurdata);
            tstcheck(val == NULL,
                    "Should reject: %s", tstcurdata);
        }
    }
}
```

### Example 2: Integration Testing with Database

```c
#include "tst.h"
#include "database.h"

tstsuite("Database Integration", Integration, RequiresDB) {
    
    db_connection_t *db = NULL;
    
    // Skip all tests if database unavailable
    tstskipif(!db_is_available()) {
        
        tstcase("Connection Management") {
            db = db_connect("test.db");
            tstassert(db != NULL, "Database connection failed");
            
            tstsection("Basic operations") {
                tstcheck(db_ping(db) == 0);
                tstcheck(db_get_version(db) > 0);
            }
            
            tstsection("Transaction") {
                tstcheck(db_begin_transaction(db) == 0);
                tstcheck(db_execute(db, "CREATE TABLE test(id INT)") == 0);
                tstcheck(db_commit(db) == 0);
            }
            
            db_close(db);
        }
        
        tstcase("CRUD Operations") {
            db = db_connect("test.db");
            tstassert(db != NULL);
            
            // Create
            int user_id = db_create_user(db, "test@example.com");
            tstcheck(user_id > 0, "Failed to create user");
            
            // Read
            user_t *user = db_get_user(db, user_id);
            tstcheck(user != NULL);
            tstcheck(strcmp(user->email, "test@example.com") == 0);
            
            // Update
            tstcheck(db_update_user(db, user_id, "new@example.com") == 0);
            user = db_get_user(db, user_id);
            tstcheck(strcmp(user->email, "new@example.com") == 0);
            
            // Delete
            tstcheck(db_delete_user(db, user_id) == 0);
            tstcheck(db_get_user(db, user_id) == NULL);
            
            db_close(db);
        }
    }
}
```

### Example 3: Fuzzing Test

```c
#include "tst.h"
#include "parser.h"
#include <stdlib.h>
#include <time.h>

tstsuite("Parser Fuzzing", Fuzzing, Slow) {
    srand(time(NULL));
    
    tstcase("Random Input Fuzzing") {
        char tstdata[1000][256];
        
        // Generate random strings
        for (int i = 0; i < 1000; i++) {
            int len = rand() % 255;
            for (int j = 0; j < len; j++) {
                // Random printable ASCII
                tstdata[i][j] = 32 + (rand() % 95);
            }
            tstdata[i][len] = '\0';
        }
        
        tstsection("Parser should not crash") {
            // Parser must handle any input gracefully
            parsed_t *result = parse(tstcurdata);
            
            // We don't care if parsing succeeds or fails,
            // just that it doesn't crash or leak
            if (result) {
                tstexpect(result->valid == 0 || result->valid == 1);
                free_parsed(result);
            }
        }
    }
    
    tstcase("Boundary Fuzzing") {
        struct {
            size_t size;
            char *data;
        } tstdata[100];
        
        // Test various buffer sizes
        int sizes[] = {0, 1, 2, 7, 8, 9, 15, 16, 17, 
                       255, 256, 257, 1023, 1024, 1025,
                       4095, 4096, 4097, 65535, 65536};
        
        for (int i = 0; i < 100; i++) {
            size_t size = sizes[rand() % (sizeof(sizes)/sizeof(sizes[0]))];
            tstdata[i].size = size;
            tstdata[i].data = malloc(size + 1);
            
            // Fill with random data
            for (size_t j = 0; j < size; j++) {
                tstdata[i].data[j] = rand() % 256;
            }
            tstdata[i].data[size] = '\0';
        }
        
        tstsection("Handle various buffer sizes") {
            tstnote("Testing size: %zu", tstcurdata.size);
            
            int result = process_buffer(tstcurdata.data, tstcurdata.size);
            tstexpect(result >= 0, 
                     "Buffer processing failed for size %zu", 
                     tstcurdata.size);
        }
        
        // Cleanup
        for (int i = 0; i < 100; i++) {
            free(tstdata[i].data);
        }
    }
}
```

### Example 4: Performance Regression Testing

```c
#include "tst.h"
#include "sorting.h"

tstsuite("Sorting Performance", Performance, Slow) {
    
    tstcase("Algorithm Comparison") {
        struct {
            const char *name;
            void (*sort_fn)(int*, int);
            clock_t elapsed;
        } algorithms[] = {
            {"Quicksort", quicksort, 0},
            {"Mergesort", mergesort, 0},
            {"Heapsort", heapsort, 0}
        };
        
        const int SIZE = 100000;
        int *data = malloc(SIZE * sizeof(int));
        
        for (int i = 0; i < 3; i++) {
            // Generate same random data for each algorithm
            srand(12345);
            for (int j = 0; j < SIZE; j++) {
                data[j] = rand();
            }
            
            tstclock("%s on %d elements", algorithms[i].name, SIZE) {
                algorithms[i].sort_fn(data, SIZE);
            }
            algorithms[i].elapsed = tstelapsed();
            
            // Verify sorted
            tstcheck(is_sorted(data, SIZE), 
                    "%s failed to sort correctly", algorithms[i].name);
        }
        
        // Compare performance
        tstnote("Quicksort: %ld ticks", algorithms[0].elapsed);
        tstnote("Mergesort: %ld ticks", algorithms[1].elapsed);
        tstnote("Heapsort: %ld ticks", algorithms[2].elapsed);
        
        // Performance regression check
        tstcheck(algorithms[0].elapsed < algorithms[1].elapsed * 2,
                "Quicksort unexpectedly slow");
        
        free(data);
    }
}
```

---

## API Reference

### Macros

#### Suite Definition
| Macro | Description |
|-------|-------------|
| `tstsuite(title)` | Define enabled test suite |
| `tst_suite(title)` | Define disabled test suite (compile-time skip) |

#### Test Organization
| Macro | Description |
|-------|-------------|
| `tstcase(description)` | Define test case with description |
| `tstcase(description, +Tag, ...)` | Define test case with tags (bare identifiers) |
| `tst_case(description)` | Disabled test case |
| `tstsection(description, ...)` | Define section with setup/teardown isolation |
| `tst_section(description, ...)` | Disabled section |

#### Assertions
| Macro | Description |
|-------|-------------|
| `tst(expression)` | Silent test (no output, sets result) |
| `tstcheck(condition, message, ...)` | Standard assertion with output |
| `tst_check(...)` | Disabled check |
| `tstexpect(condition, message, ...)` | Silent on pass, prints on fail |
| `tst_expect(...)` | Disabled expect |
| `tstassert(condition, message, ...)` | Critical assertion (aborts on fail) |
| `tst_assert(...)` | Disabled assert |

#### Control Flow
| Macro | Description |
|-------|-------------|
| `tstskipif(condition)` | Skip enclosed tests if condition true |
| `tst_skpif(condition)` | Disabled skip |

#### Output
| Macro | Description |
|-------|-------------|
| `tstnote(message, ...)` | Print informational note |
| `tst_note(...)` | Disabled note |
| `tstprintf(format, ...)` | Direct fprintf to stderr |
| `tstouterr(message, ...)` | Delimited output block |

#### Timing
| Macro | Description |
|-------|-------------|
| `tstclock(description, ...)` | Time code block execution |
| `tst_clock(...)` | Disabled clock |
| `tstelapsed()` | Get last clock measurement (clock_t) |

#### Data-Driven Testing
| Macro/Variable | Description |
|----------------|-------------|
| `tstdata[]` | Test data array (must be named tstdata) |
| `tstcurdata` | Current data element in section |
| `tst_data_size` | Number of elements in tstdata |

#### Tags
Tags are specified directly in `tstcase()` declarations as bare identifiers with `+` or `-` prefix. They are controlled via command line arguments:

| Command Line | Description |
|--------------|-------------|
| `--list` | List all test cases with their tags |
| `+TAG` | Enable tests with `+TAG`, disable tests with `-TAG` |
| `-TAG` | Disable tests with `+TAG`, enable tests with `-TAG` |
| `+*` | Enable all tests with any `+TAG` (not `-TAG`) |

See [Tags and Selective Test Execution](#tags-and-selective-test-execution) for details.

### Functions

| Function | Returns | Description |
|----------|---------|-------------|
| `tstpassed()` | int | Non-zero if last test passed |
| `tstfailed()` | int | Non-zero if last test failed |
| `tstskipped()` | int | Non-zero if last test skipped |

### Global Variables

| Variable | Type | Description |
|----------|------|-------------|
| `tst_pass` | int | Total passed tests |
| `tst_fail` | int | Total failed tests |
| `tst_skip` | int | Total skipped tests |
| `tst_case_pass` | short | Passed tests in current case |
| `tst_case_fail` | short | Failed tests in current case |
| `tst_case_skip` | short | Skipped tests in current case |

### Command Line Options

| Option | Description |
|--------|-------------|
| `--help` | Show help message |
| `--report-error` | Return error on test failure |
| `--list` | List suite name and tags |
| `+tag` | Enable specific tag |
| `-tag` | Disable specific tag |
| `+*` | Enable all tags |
| `-*` | Disable all tags |

### Environment Variables

| Variable | Description |
|----------|-------------|
| `TSTOPTIONS` | Default command line options |

---

## Version History

### 0.7.4-rc (Current)
- Complete data-driven testing support
- Enhanced section behavior
- Printf-style formatting for cases/sections
- Improved tag handling
- Bug fixes and stability improvements

### License

MIT License - See LICENSE file for details

---

**End of Manual**

For more examples and tutorials, see the `tutorial/` directory.  
For a concise API reference, see `ref_manual.md`.  
For maintainer-focused internals and upgrade guides, see `docs/maintainer/`.
