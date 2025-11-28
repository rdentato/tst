# TST Quick Reference Card

## Minimal Test Structure
```c
#include "tst.h"

tstsuite("Test Suite Name") {
  tstcase("Test case name") {
    tstcheck(condition, "Optional error message");
  }
}
```

## Command-Line Options

| Option | Purpose |
|--------|---------|
| `./test` | Run all untagged tests |
| `./test --list` | Show all tests and their tags |
| `./test --report-error` | Exit 1 on failures (for CI/CD) |
| `./test +tag` | Enable tests with +tag |
| `./test -tag` | Enable tests with -tag |
| `./test +*` | Enable all +tag tests |
| `./test --color` | Colored output |

## Tag Examples
```c
tstcase("Fast test") { }              // Always runs
tstcase("Slow test", +slow) { }       // Only with +slow
tstcase("Interactive", -ci) { }       // Only with -ci
tstcase("DB test", +db, +slow) { }    // With +db OR +slow
```

## Assertion Types
```c
tstcheck(expr);           // Report pass/fail
tstassert(expr);          // Abort test on failure
tstexpect(expr);          // Only report failures
tst(expr);                // Test without reporting
```

## Test Organization
```c
tstcase("Test name") {
  // Setup
  
  tstsection("First scenario") {
    tstcheck(...);
  }
  
  tstsection("Second scenario") {
    tstcheck(...);
  }
  
  // Cleanup runs after last section
}
```

## CI/CD Example
```bash
# Build and run tests with strict error checking
make test
./t_mytest +* -manual --report-error
```

## Common Workflows

**Development:**
```bash
./test              # Quick smoke tests
./test +*           # Full test suite
```

**CI/CD:**
```bash
./test -manual --report-error    # Automated tests only
./test +* --report-error         # All tests, fail on error
```

**Debugging:**
```bash
./test --list              # See available tests
./test +database           # Focus on one subsystem
```

## Exit Codes

- Without `--report-error`: Always 0
- With `--report-error`: 0 if pass, 1 if fail

## Tutorial Examples

All in `/tutorial`:
- `t_fact_0.c` - Minimal example
- `t_tags_demo.c` - Tag usage
- `t_report_error_demo.c` - Exit code behavior
- Run `demo_report_error.sh` for interactive demo
