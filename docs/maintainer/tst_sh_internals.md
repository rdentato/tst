# tst.sh Internals and Implementation

**Version:** 0.8.1-beta  
**Last Updated:** November 2025

---

## Overview

The `tst.sh` library is a bash implementation of the TST testing framework API. It provides shell functions that generate TST-formatted log output compatible with the `t2h` HTML report generator.

**Key Design Goals:**
1. Mirror the tst.h C API as closely as possible
2. Generate log output conforming to `docs/log_format.bnf`
3. Support auto-detection of line numbers using bash features
4. Maintain internal state for counters and metadata
5. Be source-able in any bash test script

---

## Architecture

### File Structure

```
src/tst.sh
├── Header comments (SPDX, usage documentation)
├── Internal state variables
├── Utility functions (formatting, timestamps)
├── Suite-level functions
├── Case-level functions
├── Assertion functions
├── Section functions
├── Skip block functions
├── Output functions
├── Helper functions
└── Example usage (when run directly)
```

### State Management

The library maintains global bash variables to track test execution state.

**Design Decision:** Use global variables instead of passing state between functions to mirror the tst.h approach and keep the API simple.

#### Total Counters (Suite-Level)

```bash
TST_TOTAL_PASS=0  # Total passed checks across all cases
TST_TOTAL_FAIL=0  # Total failed checks across all cases
TST_TOTAL_SKIP=0  # Total skipped checks across all cases
```

These are reset by `tstsuite_begin` and printed by `tstsuite_end`.

#### Per-Case Counters

```bash
TST_CASE_PASS=0  # Passed checks in current case
TST_CASE_FAIL=0  # Failed checks in current case
TST_CASE_SKIP=0  # Skipped checks in current case
```

These are reset by `tstcase_begin` and printed by `tstcase_end`.

#### Line Number Tracking

```bash
TST_CASE_LINE=0     # Line number from tstcase_begin
TST_SECTION_LINE=0  # Line number from section/skip/output begin
```

**Purpose:** Store line numbers for end functions that need to print the same line number as the corresponding begin function.

#### Suite Metadata

```bash
TST_SUITE_TITLE=""       # Suite title string
TST_SUITE_FILE=""        # Source filename
TST_SUITE_START_TIME=""  # Timestamp from tstsuite_begin
```

#### Abort Flag

```bash
TST_ABORT=0  # Set to 1 by tstassert on failure
```

**Purpose:** Determines whether `tstsuite_end` prints `RSLT` or `ABRT`.

---

## Line Number Auto-Detection

### Problem Statement

In C, `__LINE__` is a compile-time macro that expands to the current source line. In bash, there's no direct equivalent. We need to capture the line number where a function is called, not where it's defined.

### Solution: BASH_LINENO Array

Bash provides `${BASH_LINENO[0]}`, which contains the line number of the function's caller.

**Example:**
```bash
function foo() {
    echo "Called from line: ${BASH_LINENO[0]}"
}

foo  # Line 5
# Output: Called from line: 5
```

### Implementation

Functions use `${BASH_LINENO[0]}` to capture the caller's line number:

```bash
tstpass() {
    local line_num=${BASH_LINENO[0]}
    local expression="$1"
    
    tst_fprintf "%s PASS|  %s\n" "$(tst_format_lineno "$line_num")" "$expression"
    # ...
}
```

### Hybrid Mode: Optional Explicit Line Numbers

**Challenge:** When calling `tstpass` from a helper function, `${BASH_LINENO[0]}` shows the line inside the helper, not where the helper was called.

**Solution:** Support both auto-detect and explicit line number modes.

```bash
tstpass() {
    local line_num
    local expression
    
    # Check if first arg is a number (explicit line number)
    if [[ "$1" =~ ^[0-9]+$ ]] && [ $# -eq 2 ]; then
        line_num="$1"
        expression="$2"
    else
        # Auto-detect using BASH_LINENO
        line_num=${BASH_LINENO[0]}
        expression="$1"
    fi
    
    # ... rest of function
}
```

**Usage:**
```bash
# Auto-detect (direct call)
tstpass "test -f file.txt"

# Explicit (from helper function)
my_helper() {
    tstpass $LINENO "test from helper"
}
```

**Detection Logic:**
1. If first argument matches regex `^[0-9]+$` (is a number)
2. AND the function has exactly 2 arguments (for `tstpass`, `tstskip`)
3. Then treat first arg as explicit line number
4. Otherwise, use auto-detect mode

**Applied to:**
- `tstpass` (1-2 args)
- `tstfail` (2-3 args)
- `tstskip` (1-2 args)

---

## Function Implementation Details

### Suite Functions

#### `tstsuite_begin`

**Purpose:** Initialize suite and print header.

**Algorithm:**
1. Store suite title, filename
2. Capture start timestamp with `tst_timestamp`
3. Reset total counters to 0
4. Reset abort flag to 0
5. Print suite header line

**Format:**
```
----- SUIT / filename "title" timestamp
----- SUIT / filename "title" timestamp (disabled)
```

**Implementation Note:** The disabled suffix is optional and controlled by the third parameter.

#### `tstsuite_end`

**Purpose:** Print final results.

**Algorithm:**
1. Capture end timestamp
2. Check `TST_ABORT` flag
3. If aborted, print `ABRT` line
4. Otherwise, print `RSLT` line
5. Include fail/pass/skip counts and timestamp

**Format:**
```
^^^^^ RSLT \ FAIL | PASS | SKIP timestamp
^^^^^ ABRT \ FAIL | PASS | SKIP timestamp
```

### Case Functions

#### `tstcase_begin`

**Purpose:** Start a test case and reset per-case counters.

**Algorithm:**
1. Auto-capture line number from `${BASH_LINENO[0]}`
2. Store line number in `TST_CASE_LINE`
3. Reset `TST_CASE_PASS`, `TST_CASE_FAIL`, `TST_CASE_SKIP` to 0
4. Print case header line

**Format:**
```
  LINE CASE,-- title
```

**Design Decision:** Store the line number for use by `tstcase_end`, which needs to print the same line number.

#### `tstcase_end`

**Purpose:** Print case summary with partial results.

**Algorithm:**
1. Use stored `TST_CASE_LINE` or optional explicit line number
2. Print case end marker with pass/fail/skip counts

**Format:**
```
  LINE     `--- FAIL | PASS | SKIP
```

**Note:** The backtick is escaped as `\`` in bash strings.

### Assertion Functions

#### `tstpass`

**Purpose:** Record a passing check.

**Algorithm:**
1. Detect line number (auto or explicit)
2. Print PASS line with formatted line number and expression
3. Increment `TST_CASE_PASS` and `TST_TOTAL_PASS`

**Format:**
```
  LINE PASS|  expression
```

**Counter Update:**
```bash
TST_CASE_PASS=$((TST_CASE_PASS + 1))
TST_TOTAL_PASS=$((TST_TOTAL_PASS + 1))
```

#### `tstfail`

**Purpose:** Record a failing check with optional message.

**Algorithm:**
1. Detect line number and expression
2. Extract optional message (third arg or fourth arg depending on mode)
3. Print FAIL line with expression
4. If message provided and not "-", append quoted message
5. Increment fail counters

**Format:**
```
  LINE FAIL|  expression
  LINE FAIL|  expression "message"
```

**Message Handling:**
```bash
if [ -n "$message" ] && [ "$message" != "-" ]; then
    tst_fprintf " \"%s\"" "$message"
fi
```

The "-" special value allows callers to explicitly suppress the message.

#### `tstcheck`

**Purpose:** Evaluate exit status and call appropriate function.

**Algorithm:**
1. Accept expression, exit status, and optional message
2. If exit status is 0, call `tstpass`
3. Otherwise, call `tstfail` with message

**Design Note:** This is a convenience wrapper. The expression is a string description, not evaluated by `tstcheck` itself.

**Usage Pattern:**
```bash
command_to_test
tstcheck "command succeeds" $?
```

#### `tstassert`

**Purpose:** Critical check that aborts on failure.

**Algorithm:**
1. If exit status is 0, call `tstpass` and return
2. Otherwise:
   - Call `tstfail` with message
   - Call `tstcase_end` to close current case
   - Set `TST_ABORT=1`
   - Call `tstsuite_end` to print final results
   - Exit script with status 1

**Design Decision:** Immediately exit to prevent further test execution, matching tst.h `tstassert` behavior.

### Output Functions

#### `tstnote`

**Purpose:** Print informational notes.

**Algorithm:**
1. Auto-capture line number
2. Print NOTE line with message

**Format:**
```
  LINE NOTE: message
```

#### `tstclock`

**Purpose:** Record timing information.

**Algorithm:**
1. Accept elapsed value, unit character, and description
2. Auto-capture line number
3. Print CLCK line

**Format:**
```
  LINE CLCK:  value units description
```

**Unit Characters:**
- "n" → ns (nanoseconds)
- "u" → µs (microseconds)
- "m" → ms (milliseconds)

**Caller Responsibility:** The caller must perform time measurement and unit conversion. The function only formats output.

#### `tstoutput_begin` / `tstoutput_end`

**Purpose:** Delimit multi-line output blocks.

**Algorithm:**
1. `tstoutput_begin`: Capture line number, print `<<<<<` marker
2. User code prints output
3. `tstoutput_end`: Print `>>>>>` marker with same line number

**Format:**
```
  LINE <<<<< description
output line 1
output line 2
  LINE >>>>>
```

**Design Note:** Output between begin/end has no line numbers, matching tst.h behavior.

---

## Helper Functions

### Design Philosophy

Helper functions provide common testing patterns:
- Reduce boilerplate in test scripts
- Encapsulate comparison logic
- Provide clear failure messages

### `tst_run`

**Purpose:** Run command and check exit status.

**Algorithm:**
1. Accept expression and command with arguments
2. Execute command using `"$@"`
3. Capture exit status in `$?`
4. Call `tstcheck` with expression and status
5. Return command's exit status

**Usage:**
```bash
tst_run "List directory" ls /tmp
```

**Why Return Status?** Allows caller to conditionally execute code based on success/failure.

### `tst_equal`

**Purpose:** String equality comparison.

**Algorithm:**
1. Compare `$1` and `$2` using `[ "$1" = "$2" ]`
2. If equal, call `tstpass` with description
3. Otherwise, call `tstfail` with message showing both values

**Message Format:**
```
Expected 'expected_value', got 'actual_value'
```

### Numeric Comparisons

**`tst_greater`:** Uses `[ "$actual" -gt "$threshold" ]`  
**`tst_less`:** Uses `[ "$actual" -lt "$threshold" ]`

**Design Note:** These use bash's integer comparison operators (`-gt`, `-lt`). They will fail if arguments are not valid integers.

---

## Formatting and Output

### `tst_fprintf`

**Purpose:** Printf-like output to stderr.

**Implementation:**
```bash
tst_fprintf() {
    # shellcheck disable=SC2059
    printf -- "$@" >&2
}
```

**Why `>&2`?** All TST output goes to stderr to match tst.h behavior. This allows test programs to produce normal output on stdout without interfering with logs.

**Why `disable=SC2059`?** Shellcheck warns about using variables in printf format strings. We suppress this because the format is intentionally passed as a variable.

### `tst_format_lineno`

**Purpose:** Format line numbers as 5-character right-aligned fields.

**Implementation:**
```bash
tst_format_lineno() {
    printf "%5d" "$1"
}
```

**Why 5 characters?** Matches tst.h output format for consistency.

**Examples:**
- Line 1 → `"    1"`
- Line 42 → `"   42"`
- Line 123 → `"  123"`
- Line 9999 → ` 9999"`

### `tst_timestamp`

**Purpose:** Generate TST-format timestamps.

**Implementation:**
```bash
tst_timestamp() {
    date '+%Y-%m-%d %H:%M:%S'
}
```

**Format:** `YYYY-MM-DD HH:MM:SS` (matches tst.h)

---

## Error Handling

### Approach

The library uses a permissive approach:
- No validation of user input
- Assumes caller provides correct arguments
- No error checking on state variables

**Rationale:** Testing libraries should be lightweight and fast. Validation overhead would slow down test execution.

### Consequences

**User Errors:**
- Calling `tstcase_end` without `tstcase_begin` will print incorrect line numbers
- Calling `tstpass` with wrong number of arguments may cause failures
- Incrementing counters in wrong state leads to incorrect totals

**Mitigation:** Clear documentation and examples. Users should follow the prescribed patterns.

---

## Compatibility

### Bash Version Requirements

**Minimum:** Bash 3.0+

**Features Used:**
- Arrays: `${BASH_LINENO[0]}`
- Regex matching: `[[ "$1" =~ ^[0-9]+$ ]]`
- Arithmetic: `$((VAR + 1))`
- Here-documents: `cat <<'EOF'`

**Tested On:**
- Bash 4.x (Linux)
- Bash 5.x (macOS, modern Linux)
- Bash 3.x (legacy systems)

### Platform Support

**Linux:** ✅ Fully supported  
**macOS:** ✅ Fully supported  
**Windows (WSL):** ✅ Fully supported  
**Windows (Git Bash):** ✅ Fully supported  
**Windows (Cygwin):** ✅ Should work (not extensively tested)

**Key Compatibility:**
- Uses `date '+format'` (POSIX)
- Uses `printf` (POSIX)
- Avoids bash 4+ features for wider compatibility

---

## Testing

### Self-Test

The library includes example usage at the bottom:

```bash
if [ "${BASH_SOURCE[0]}" = "$0" ]; then
    # Example code here
fi
```

This executes only when the script is run directly (not sourced), providing inline documentation.

### Integration Tests

The library is tested via `test/t_tag_filters.sh`, which:
- Sources the library
- Exercises all major functions
- Generates TST-formatted output
- Serves as a real-world usage example

**Test Coverage:**
- Suite begin/end
- Case begin/end
- All assertion functions (pass/fail/skip/check/assert)
- Sections
- Notes
- Both auto-detect and explicit line numbers

---

## Design Decisions

### Why Global Variables?

**Alternatives Considered:**
1. Pass state as function arguments
2. Use associative arrays
3. Use subshells with state export

**Decision:** Global variables

**Rationale:**
- Simplest API (matches tst.h)
- No parameter passing overhead
- No subshell complexity
- Easy debugging (can inspect state)

### Why Auto-Detect Line Numbers?

**Problem:** Bash doesn't have `__LINE__` macro.

**Alternatives:**
1. Require explicit `$LINENO` for all calls
2. No line numbers in output
3. Auto-detect with `${BASH_LINENO[0]}`

**Decision:** Auto-detect with optional explicit override

**Rationale:**
- Cleaner API for common case
- Matches tst.h user experience (automatic line numbers)
- Explicit mode available when needed (helper functions)

### Why Not Use `set -e`?

**Not Used:** `set -e` (exit on error)

**Rationale:**
- Tests need to continue after failures
- `tstfail` intentionally records failures without exiting
- Only `tstassert` should abort execution
- User scripts may handle errors differently

### Function Naming Convention

**Pattern:**
- Public API: `tstfunction` (no underscore, matches tst.h)
- Internal helpers: `tst_function` (underscore prefix)
- State variables: `TST_UPPER_CASE`

**Examples:**
- `tstpass` - public assertion function
- `tst_run` - public helper function
- `tst_fprintf` - internal utility
- `TST_TOTAL_PASS` - internal state variable

**Rationale:**
- Matches tst.h naming for familiarity
- Clear distinction between API and internals
- Prevents naming conflicts with user code

---

## Future Enhancements

### Potential Improvements

1. **Data-Driven Testing:**
   - Add support for iterating over arrays
   - Implement `tstdata`-like functionality
   - Requires bash 4+ for associative arrays

2. **Tag Filtering:**
   - Implement `--list` mode
   - Parse `+Tag` and `-Tag` arguments
   - Skip tagged cases based on command line

3. **Performance Timing:**
   - Add `tstclock { }` block syntax
   - Automatic time measurement using bash `SECONDS`
   - Requires bash 4+ for sub-second precision

4. **Better Error Messages:**
   - Validate function call order
   - Detect mismatched begin/end pairs
   - Add debug mode with stack traces

### Backwards Compatibility

Any future changes must maintain backwards compatibility with existing test scripts. The current API is considered stable.

---

## Maintenance Notes

### Modifying the Library

**Before Making Changes:**
1. Review `docs/log_format.bnf` to ensure output compliance
2. Run existing tests: `cd test && ./t_tag_filters.sh`
3. Check bash version compatibility
4. Update this documentation

**Testing Changes:**
1. Add tests to `test/t_tag_filters.sh`
2. Run test and verify output format
3. Generate HTML report: `./t_tag_filters.sh | ../src/t2h > test.html`
4. Verify report renders correctly

### Adding New Functions

**Checklist:**
1. Choose appropriate name (public: `tstfunc`, internal: `tst_func`)
2. Add documentation to function header comment
3. Implement line number handling (auto-detect preferred)
4. Update state variables if needed
5. Add to appropriate section in file
6. Document in `docs/prog_manual.md` and `docs/ref_manual.md`
7. Add usage example to `test/t_tag_filters.sh`

### Debugging

**Common Issues:**

**Wrong line numbers:**
- Check if `${BASH_LINENO[0]}` is used correctly
- Verify explicit mode detection logic
- Test with both direct calls and helper functions

**State corruption:**
- Verify counter increments/resets
- Check that begin/end functions match
- Ensure no variable name conflicts

**Output format errors:**
- Compare against `docs/log_format.bnf`
- Check line number formatting (5 chars)
- Verify marker strings match grammar

**Tools:**
- `set -x` for execution tracing
- `declare -p VAR` to inspect variable state
- `bash -n script.sh` for syntax checking

---

## References

- [Bash Manual](https://www.gnu.org/software/bash/manual/)
- [BASH_LINENO documentation](https://www.gnu.org/software/bash/manual/html_node/Bash-Variables.html)
- [TST Log Format Grammar](../log_format.bnf)
- [tst.h Internals](tst_h_internals.md)
- [t2h Architecture](t2h_architecture.md)

---

**End of Document**
