# TST Library Programmer's Manual

## Overview

TST is a lightweight, header-only C testing framework designed for simplicity, flexibility, and zero dependencies. It enables developers to create organized test suites with minimal setup, clear output formatting, and powerful features for data-driven testing, performance measurement, and selective test execution.

**Version:** 0.7.4-rc  
**License:** MIT  
**Language:** C (C++ compatible)

### Key Features

- **Single Header File** - Just include `tst.h`, no configuration needed
- **Zero Dependencies** - Uses only standard C library
- **Data-Driven Testing** - Iterate tests over arrays of test data
- **Fuzzing Support** - Easy random test data generation
- **Performance Timing** - Built-in CPU time measurement
- **Selective Execution** - Tag-based filtering and conditional skipping
- **Flexible Output** - Structured logs with optional color coding
- **Cross-Platform** - Works on Linux, macOS, Windows (with any C compiler)
- **Nested Organization** - Test cases, sections, and hierarchical structure

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
10. [Performance Testing and Timing](#performance-testing-and-timing)
11. [Command Line Options](#command-line-options)
12. [Environment Variables](#environment-variables)
13. [Advanced Techniques](#advanced-techniques)
14. [Best Practices](#best-practices)
15. [Complete Examples](#complete-examples)
16. [API Reference](#api-reference)

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

**Test cases can be nested** for hierarchical organization:

```c
tstcase("Outer Test Case") {
    int setup_value = initialize();
    
    tstcase("Inner Test Case A") {
        tstcheck(setup_value > 0);
    }
    
    tstcase("Inner Test Case B") {
        tstcheck(setup_value < 100);
    }
}
```

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

Both `tstcase` and `tstsection` support formatted descriptions:

```c
int test_value = 42;
tstcase("Testing value %d", test_value) {
    tstcheck(test_value == 42);
}

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

// Add descriptive strings for complex checks
tstcheck("Database connection" && (db != NULL));
int x = factorial(5);
tstcheck("Cecking factorial" && x == 120);  
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

Tags provide compile-time test categorization and runtime filtering.

### Defining Tags

Declare tags in the `tstsuite` definition (up to 8 tags):

```c
tstsuite("Comprehensive Tests", 
         SlowTests, QuickTests, RequiresDB, RequiresNet,
         Integration, Unit, Smoke, Regression) {
    // Tag-based test organization
}
```

### Using Tags in Tests

Check tag status with `tsttag(TagName)`:

```c
tstsuite("API Tests", SlowTests, QuickTests) {
    
    // Quick tests run when QuickTests tag is enabled
    tstskipif(!tsttag(QuickTests)) {
        tstcase("Fast API Check") {
            tstcheck(api_ping() == 0);
        }
    }
    
    // Slow tests only when SlowTests enabled
    tstskipif(!tsttag(SlowTests)) {
        tstcase("Full API Scan") {
            for (int i = 0; i < 1000; i++) {
                tstcheck(api_call(i) >= 0);
            }
        }
    }
}
```

### Runtime Tag Control

Enable/disable tags programmatically:

```c
tstsuite("Dynamic Tags", Debug, Performance) {
    
    // Initially run with current tag settings
    tstskipif(tsttag(Debug)) {
        tstcheck(debug_check_1());
    }
    
    // Disable Debug tag
    tsttag(Debug, 0);
    
    // These won't be skipped even though Debug was initially on
    tstskipif(tsttag(Debug)) {
        tstcheck(debug_check_2());  // Will run
    }
    
    // Re-enable Debug tag
    tsttag(Debug, 1);
}
```

### Command-Line Tag Control

```bash
# Enable SlowTests, disable QuickTests
./test_program +SlowTests -QuickTests

# Enable all tags
./test_program +*

# Disable all tags, then enable specific ones
./test_program -* +Unit +Smoke

# Multiple options
./test_program +SlowTests +Integration --color
```

### Tag-Based Test Organization Patterns

#### By Speed
```c
tstsuite("Tests", Fast, Slow) {
    tstskipif(tsttag(Fast)) {
        tstcase("Quick Checks") { /* ... */ }
    }
    
    tstskipif(tsttag(Slow)) {
        tstcase("Comprehensive Checks") { /* ... */ }
    }
}
```

#### By Resource Requirements
```c
tstsuite("Tests", NoDB, NoNet, NoFS) {
    tstskipif(tsttag(NoDB)) {
        tstcase("Database Tests") { /* ... */ }
    }
    
    tstskipif(tsttag(NoNet)) {
        tstcase("Network Tests") { /* ... */ }
    }
}
```

#### By Test Type
```c
tstsuite("Tests", Unit, Integration, E2E) {
    tstskipif(tsttag(Unit)) {
        tstcase("Unit Tests") { /* ... */ }
    }
    
    tstskipif(tsttag(Integration)) {
        tstcase("Integration Tests") { /* ... */ }
    }
}
```

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

### Color Output

Colors are disabled by default. Enable with `--color`:

```bash
./test_program --color
```

Color scheme:
- **FAIL**: Red
- **PASS**: Green
- **SKIP**: Yellow
- **Headers/Markers**: Cyan

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
./test_program [options] [tags]
```

### Available Options

#### `--help`

Display usage information:

```bash
$ ./test_program --help
Test suite: "My Test Suite"
./test_program [--help] [--color] [--report-error] [--list] [+/-]tag ...
tags: SlowTests QuickTests
```

#### `--color`

Toggle ANSI color output:

```bash
./test_program --color
```

Colors are persistent - using `--color` again toggles them off.

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

#### `--list`

List suite name and available tags:

```bash
$ ./test_program --list
test_program "Database Integration Tests"
tags: SlowTests RequiresDB Integration Smoke
```

### Tag Control

#### Enable Tags

```bash
./test_program +SlowTests +Integration
```

#### Disable Tags

```bash
./test_program -QuickTests
```

#### All Tags

```bash
# Enable all tags
./test_program +*

# Disable all tags
./test_program -*
```

#### Combined Options

```bash
./test_program --color --report-error +SlowTests -QuickTests
```

---

## Environment Variables

### `TSTOPTIONS`

Set default options via environment variable:

```bash
export TSTOPTIONS="--color +SlowTests"
./test_program  # Runs with color and SlowTests enabled
```

Command-line arguments override environment settings:

```bash
export TSTOPTIONS="--color"
./test_program --color  # Toggles color OFF (since env enabled it)
```

### Combining Options

```bash
# In .bashrc or environment
export TSTOPTIONS="--color --report-error"

# Per-test customization
./test_program +Integration    # Adds Integration tag
./test_program -* +Unit +Smoke # Override: only Unit and Smoke
```

### CI/CD Integration

```bash
# GitLab CI example
test:
  script:
    - export TSTOPTIONS="--report-error --color"
    - make test
```

---

## Advanced Techniques

### Nested Test Cases

Organize complex test hierarchies:

```c
tstcase("Top Level") {
    int global_setup = initialize();
    
    tstcase("Category A") {
        int category_setup = setup_a();
        
        tstcheck(test_a1(global_setup, category_setup));
        tstcheck(test_a2(global_setup, category_setup));
        
        cleanup_a(category_setup);
    }
    
    tstcase("Category B") {
        int category_setup = setup_b();
        
        tstcheck(test_b1(global_setup, category_setup));
        
        cleanup_b(category_setup);
    }
    
    cleanup(global_setup);
}
```

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
| `tstsuite(title, ...)` | Define enabled test suite with optional tags |
| `tst_suite(title, ...)` | Define disabled test suite (compile-time skip) |

#### Test Organization
| Macro | Description |
|-------|-------------|
| `tstcase(description, ...)` | Define test case (supports printf formatting) |
| `tst_case(description, ...)` | Disabled test case |
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
| Function | Description |
|----------|-------------|
| `tsttag(tag)` | Returns non-zero if tag enabled |
| `tsttag(tag, value)` | Enable (1) or disable (0) tag |

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
| `--color` | Toggle color output |
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
For a concise API reference, see `reference.md`.
