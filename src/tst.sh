#!/bin/bash
#  SPDX-FileCopyrightText: © 2025 Remo Dentato <rdentato@gmail.com>
#  SPDX-License-Identifier: MIT
#
#  TST Shell Library
#  =================
#
#  A bash library for creating shell-based test scripts that produce
#  output conforming to the TST log format grammar (docs/log_format.bnf).
#
#  This library provides functions to generate all TST log elements:
#  - Suite headers/footers
#  - Test cases with pass/fail/skip assertions
#  - Sections for grouping related checks
#  - Skip blocks
#  - Notes, clock timing, and output capture
#  - Result summaries
#
#  Usage:
#    source path/to/tst.sh
#    tstsuite_begin "My Test Suite" "$0"
#    tstcase_begin "Test case name"
#      tstpass "condition"                    # Auto-detect line number
#      tstfail "condition" "error message"    # Auto-detect line number
#      tstpass $LINENO "explicit line"        # Or use explicit $LINENO
#    tstcase_end
#    tstsuite_end
#
#  Note: Line numbers are automatically captured using BASH_LINENO.
#        You can optionally pass $LINENO explicitly for special cases.
#
#  State Management:
#    The library maintains internal state for tracking:
#    - Total PASS/FAIL/SKIP counts
#    - Per-case PASS/FAIL/SKIP counts
#    - Current case line number
#    - Suite start timestamp
#    - Abort status
#
#  All output is written to stderr (>&2) to match tst.h behavior.
#

# ============================================================================
# Internal State Variables
# ============================================================================

# Total counters (suite-level)
TST_TOTAL_PASS=0
TST_TOTAL_FAIL=0
TST_TOTAL_SKIP=0

# Per-case counters
TST_CASE_PASS=0
TST_CASE_FAIL=0
TST_CASE_SKIP=0

# Current case line number
TST_CASE_LINE=0

# Suite metadata
TST_SUITE_TITLE=""
TST_SUITE_FILE=""
TST_SUITE_START_TIME=""

# Abort flag (set by tst_assert)
TST_ABORT=0

# Section state
TST_SECTION_LINE=0


# ============================================================================
# Utility Functions
# ============================================================================

# Get current timestamp in TST format (YYYY-MM-DD HH:MM:SS)
tst_timestamp() {
    date '+%Y-%m-%d %H:%M:%S'
}

# Print to stderr with printf formatting
# Usage: tst_fprintf "format" [args...]
tst_fprintf() {
    # shellcheck disable=SC2059
    printf -- "$@" >&2
}

# Format line number as 5-character right-aligned field
# Usage: tst_format_lineno <line_number>
tst_format_lineno() {
    printf "%5d" "$1"
}


# ============================================================================
# Suite-Level Functions
# ============================================================================

# Begin a test suite
# Usage: tstsuite_begin <suite_title> <filename> [disabled]
# Arguments:
#   suite_title - descriptive title for the suite
#   filename    - source filename (typically $0)
#   disabled    - optional, if set prints " (disabled)" suffix
tstsuite_begin() {
    local suite_title="$1"
    local filename="$2"
    local disabled="$3"
    
    TST_SUITE_TITLE="$suite_title"
    TST_SUITE_FILE="$filename"
    TST_SUITE_START_TIME=$(tst_timestamp)
    
    # Reset total counters
    TST_TOTAL_PASS=0
    TST_TOTAL_FAIL=0
    TST_TOTAL_SKIP=0
    TST_ABORT=0
    
    local disabled_suffix=""
    if [ -n "$disabled" ]; then
        disabled_suffix=" (disabled)"
    fi
    
    tst_fprintf "----- SUIT / %s \"%s\" %s%s\n" \
        "$filename" "$suite_title" "$TST_SUITE_START_TIME" "$disabled_suffix"
}

# End a test suite with normal termination
# Usage: tstsuite_end
tstsuite_end() {
    local end_time
    end_time=$(tst_timestamp)
    
    if [ "$TST_ABORT" -eq 1 ]; then
        tst_fprintf "\n^^^^^ ABRT \\ %d FAIL | %d PASS | %d SKIP %s\n" \
            "$TST_TOTAL_FAIL" "$TST_TOTAL_PASS" "$TST_TOTAL_SKIP" "$end_time"
    else
        tst_fprintf "^^^^^ RSLT \\ %d FAIL | %d PASS | %d SKIP %s\n" \
            "$TST_TOTAL_FAIL" "$TST_TOTAL_PASS" "$TST_TOTAL_SKIP" "$end_time"
    fi
}

# Print a result summary (used internally)
# Usage: tst_result_summary <fail_count> <pass_count> <skip_count>
tst_result_summary() {
    tst_fprintf "%d FAIL | %d PASS | %d SKIP" "$1" "$2" "$3"
}


# ============================================================================
# Case-Level Functions
# ============================================================================

# Begin a test case
# Usage: tstcase_begin <case_title>
tstcase_begin() {
    local case_title="$1"
    local line_num=${BASH_LINENO[0]}
    
    # Reset per-case counters
    TST_CASE_PASS=0
    TST_CASE_FAIL=0
    TST_CASE_SKIP=0
    TST_CASE_LINE="$line_num"
    
    tst_fprintf "%s CASE,--%s\n" "$(tst_format_lineno "$line_num")" "$case_title"
}

# End a test case
# Usage: tstcase_end <line_number>
tstcase_end() {
    local line_num="${1:-$TST_CASE_LINE}"
    
    tst_fprintf "%s     \`--- " "$(tst_format_lineno "$line_num")"
    tst_result_summary "$TST_CASE_FAIL" "$TST_CASE_PASS" "$TST_CASE_SKIP"
    tst_fprintf "\n"
}

# Print a skipped case (filtered by tags or conditions)
# Usage: tstcase_skip <case_title>
tstcase_skip() {
    local case_title="$1"
    local line_num=${BASH_LINENO[0]}
    
    tst_fprintf "%s SKPT|,-(%s)\n" "$(tst_format_lineno "$line_num")" "$case_title"
    
    # Increment skip counters
    TST_TOTAL_SKIP=$((TST_TOTAL_SKIP + 1))
}


# ============================================================================
# Assertion Functions
# ============================================================================

# Record a passing check
# Usage: tstpass <expression>
#        tstpass <line_number> <expression>
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
    
    tst_fprintf "%s PASS|  %s\n" "$(tst_format_lineno "$line_num")" "$expression"
    
    TST_CASE_PASS=$((TST_CASE_PASS + 1))
    TST_TOTAL_PASS=$((TST_TOTAL_PASS + 1))
}

# Record a failing check
# Usage: tstfail <expression> [message]
#        tstfail <line_number> <expression> [message]
# If message is empty or "-", no message is printed
tstfail() {
    local line_num
    local expression
    local message
    
    # Check if first arg is a number (explicit line number)
    if [[ "$1" =~ ^[0-9]+$ ]] && [ $# -ge 2 ]; then
        line_num="$1"
        expression="$2"
        message="$3"
    else
        # Auto-detect using BASH_LINENO
        line_num=${BASH_LINENO[0]}
        expression="$1"
        message="$2"
    fi
    
    tst_fprintf "%s FAIL|  %s" "$(tst_format_lineno "$line_num")" "$expression"
    
    if [ -n "$message" ] && [ "$message" != "-" ]; then
        tst_fprintf " \"%s\"" "$message"
    fi
    
    tst_fprintf "\n"
    
    TST_CASE_FAIL=$((TST_CASE_FAIL + 1))
    TST_TOTAL_FAIL=$((TST_TOTAL_FAIL + 1))
}

# Record a skipped check
# Usage: tstskip <expression>
#        tstskip <line_number> <expression>
tstskip() {
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
    
    tst_fprintf "%s SKIP|  %s\n" "$(tst_format_lineno "$line_num")" "$expression"
    
    TST_CASE_SKIP=$((TST_CASE_SKIP + 1))
    TST_TOTAL_SKIP=$((TST_TOTAL_SKIP + 1))
}

# Perform a check and record result
# Usage: tstcheck <expression> <result> [fail_message]
# Arguments:
#   expression - string describing the condition
#   result - 0 for pass, non-zero for fail
#   fail_message - optional message for failures
tstcheck() {
    local expression="$1"
    local result="$2"
    local fail_message="${3:-}"
    
    if [ "$result" -eq 0 ]; then
        tstpass "$expression"
    else
        tstfail "$expression" "$fail_message"
    fi
}

# Perform a check that aborts on failure (like tstassert)
# Usage: tstassert <expression> <result> [fail_message]
tstassert() {
    local expression="$1"
    local result="$2"
    local fail_message="${3:-}"
    
    if [ "$result" -eq 0 ]; then
        tstpass "$expression"
    else
        tstfail "$expression" "$fail_message"
        
        # Close the current case
        tstcase_end "$TST_CASE_LINE"
        
        # Mark abort and end suite
        TST_ABORT=1
        tstsuite_end
        
        exit 1
    fi
}


# ============================================================================
# Section Functions
# ============================================================================

# Begin a test section
# Usage: tstsection_begin <section_title>
tstsection_begin() {
    local section_title="$1"
    local line_num=${BASH_LINENO[0]}
    
    TST_SECTION_LINE="$line_num"
    
    tst_fprintf "%s SCTN|,-- %s\n" "$(tst_format_lineno "$line_num")" "$section_title"
}

# End a test section
# Usage: tstsection_end <line_number>
tstsection_end() {
    local line_num="${1:-$TST_SECTION_LINE}"
    
    tst_fprintf "%s     |\`---\n" "$(tst_format_lineno "$line_num")"
}


# ============================================================================
# Skip Block Functions
# ============================================================================

# Begin a skip block (like tstskipif)
# Usage: tstskip_block_begin <condition>
tstskip_block_begin() {
    local condition="$1"
    local line_num=${BASH_LINENO[0]}
    
    TST_SECTION_LINE="$line_num"  # Reuse section line tracker
    
    tst_fprintf "%s SKPT|,-(%s)\n" "$(tst_format_lineno "$line_num")" "$condition"
}

# End a skip block
# Usage: tstskip_block_end <line_number>
tstskip_block_end() {
    local line_num="${1:-$TST_SECTION_LINE}"
    
    tst_fprintf "%s     |\`---\n" "$(tst_format_lineno "$line_num")"
}


# ============================================================================
# Note and Clock Functions
# ============================================================================

# Print a note
# Usage: tstnote <message>
tstnote() {
    local message="$1"
    local line_num=${BASH_LINENO[0]}
    
    tst_fprintf "%s NOTE: %s\n" "$(tst_format_lineno "$line_num")" "$message"
}

# Print a clock/timing line
# Usage: tstclock <elapsed_value> <time_unit> <description>
# Arguments:
#   elapsed_value - numeric value of elapsed time
#   time_unit - "n" (nanoseconds), "u" (microseconds), or "m" (milliseconds)
#   description - description of what was timed
tstclock() {
    local elapsed="$1"
    local unit="$2"
    local description="$3"
    local line_num=${BASH_LINENO[0]}
    
    tst_fprintf "%s CLCK:  %s %ss %s\n" \
        "$(tst_format_lineno "$line_num")" "$elapsed" "$unit" "$description"
}


# ============================================================================
# Output Capture Functions
# ============================================================================

# Begin output capture block
# Usage: tstoutput_begin <description>
tstoutput_begin() {
    local description="$1"
    local line_num=${BASH_LINENO[0]}
    
    TST_SECTION_LINE="$line_num"
    
    tst_fprintf "%s <<<<< %s\n" "$(tst_format_lineno "$line_num")" "$description"
}

# End output capture block
# Usage: tstoutput_end <line_number>
tstoutput_end() {
    local line_num="${1:-$TST_SECTION_LINE}"
    
    tst_fprintf "%s >>>>>\n" "$(tst_format_lineno "$line_num")"
}

# Print captured output line (without line number)
# Usage: tstoutput_line <text>
tstoutput_line() {
    tst_fprintf "%s\n" "$1"
}


# ============================================================================
# List Mode Functions (for --list support)
# ============================================================================

# Print a case in list mode
# Usage: tstlist_case <case_title> [tags...]
# Arguments:
#   case_title - the case name
#   tags - optional space-separated tags like "+Tag1, -Tag2"
tstlist_case() {
    local case_title="$1"
    shift
    local tags="$*"
    
    tst_fprintf "\"%s\"" "$case_title"
    
    if [ -n "$tags" ]; then
        tst_fprintf " %s" "$tags"
    fi
    
    tst_fprintf "\n"
}


# ============================================================================
# Helper Functions for Common Patterns
# ============================================================================

# Execute a command and check its exit status
# Usage: tst_run <expression_description> <command> [args...]
# Returns the command's exit status
tst_run() {
    local expression="$1"
    shift
    
    # Run command and capture exit status
    "$@"
    local status=$?
    
    tstcheck "$expression" "$status"
    
    return "$status"
}

# Compare two values and record result
# Usage: tst_equal <actual> <expected> <description>
tst_equal() {
    local actual="$1"
    local expected="$2"
    local description="$3"
    
    if [ "$actual" = "$expected" ]; then
        tstpass "$description"
    else
        tstfail "$description" "Expected '$expected', got '$actual'"
    fi
}

# Check if a value is not equal
# Usage: tst_not_equal <actual> <unexpected> <description>
tst_not_equal() {
    local actual="$1"
    local unexpected="$2"
    local description="$3"
    
    if [ "$actual" != "$unexpected" ]; then
        tstpass "$description"
    else
        tstfail "$description" "Expected not '$unexpected', got '$actual'"
    fi
}

# Numeric comparison: greater than
# Usage: tst_greater <actual> <threshold> <description>
tst_greater() {
    local actual="$1"
    local threshold="$2"
    local description="$3"
    
    if [ "$actual" -gt "$threshold" ]; then
        tstpass "$description"
    else
        tstfail "$description" "Expected > $threshold, got $actual"
    fi
}

# Numeric comparison: less than
# Usage: tst_less <actual> <threshold> <description>
tst_less() {
    local actual="$1"
    local threshold="$2"
    local description="$3"
    
    if [ "$actual" -lt "$threshold" ]; then
        tstpass "$description"
    else
        tstfail "$description" "Expected < $threshold, got $actual"
    fi
}


# ============================================================================
# Example Usage
# ============================================================================

# This section is only executed if the script is run directly (not sourced)
if [ "${BASH_SOURCE[0]}" = "$0" ]; then
    echo "TST Shell Library - Example Usage"
    echo "=================================="
    echo
    echo "This is a library file meant to be sourced by test scripts."
    echo
    echo "Example:"
    echo
    cat <<'EOF'
#!/bin/bash
source "$(dirname "$0")/tst.sh"

tstsuite_begin "Example Test Suite" "$0"

tstcase_begin "Basic arithmetic"
  result=$((2 + 2))
  tst_equal "$result" "4" "2 + 2 == 4"
  
  result=$((3 * 3))
  tst_equal "$result" "9" "3 * 3 == 9"
tstcase_end

tstcase_begin "String comparison"
  str="hello"
  tst_equal "$str" "hello" "str == 'hello'"
  tst_not_equal "$str" "world" "str != 'world'"
tstcase_end

tstcase_begin "Command execution"
  tstnote "Testing command success"
  
  tst_run "ls /tmp succeeds" ls /tmp > /dev/null
  
  # This should fail
  if ! ls /nonexistent 2>/dev/null; then
    tstpass "ls /nonexistent fails as expected"
  fi
tstcase_end

tstsuite_end
EOF
    echo
    echo "For more examples, see test/t_tag_filters.sh"
fi
