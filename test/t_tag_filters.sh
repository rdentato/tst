#!/bin/bash
#  SPDX-FileCopyrightText: © 2025 Remo Dentato <rdentato@gmail.com>
#  SPDX-License-Identifier: MIT
#
#  Tag filter validation script with TST-compatible output format
#  Tests all tag-related test programs with different filter combinations
#  and validates expected outcomes based on the truth table in docs/manual.md

set -e

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

# Source the TST shell library
source ../src/tst.sh

# Test a specific filter combination
# Usage: test_filter <line_num> <test_binary> <description> <expected_pass> <expected_fail> <expected_skip> [filter_args...]
test_filter() {
    local line_num="$1"
    shift
    local test_binary="$1"
    shift
    local description="$1"
    shift
    local expected_pass="$1"
    shift
    local expected_fail="$1"
    shift
    local expected_skip="$1"
    shift
    # Remaining args are filter arguments
    local filter_args=("$@")
    
    # Build filter display string
    local filter_display=""
    if [ ${#filter_args[@]} -eq 0 ]; then
        filter_display="(no filter)"
    else
        filter_display="${filter_args[*]}"
    fi
    
    # Run test and capture result line
    local result
    if [ ${#filter_args[@]} -eq 0 ]; then
        result=$("./$test_binary" 2>&1 | grep "RSLT" || true)
    else
        result=$("./$test_binary" "${filter_args[@]}" 2>&1 | grep "RSLT" || true)
    fi
    
    # Parse results
    local actual_fail=$(echo "$result" | sed -n 's/.*\([0-9]\+\) FAIL.*/\1/p')
    local actual_pass=$(echo "$result" | sed -n 's/.*| \([0-9]\+\) PASS.*/\1/p')
    local actual_skip=$(echo "$result" | sed -n 's/.*| \([0-9]\+\) SKIP.*/\1/p')
    
    # Default to 0 if not found
    actual_fail=${actual_fail:-0}
    actual_pass=${actual_pass:-0}
    actual_skip=${actual_skip:-0}
    
    # Create test message
    local test_msg="${description} [${filter_display}]"
    
    # Check if results match expectations
    if [ "$actual_pass" = "$expected_pass" ] && \
       [ "$actual_fail" = "$expected_fail" ] && \
       [ "$actual_skip" = "$expected_skip" ]; then
        tstpass $line_num "$test_msg"
    else
        local detail="Expected: F=$expected_fail P=$expected_pass S=$expected_skip; Got: F=$actual_fail P=$actual_pass S=$actual_skip"
        tstfail $line_num "$test_msg" "$detail"
    fi
}

# ============================================================================
# Main Test Execution
# ============================================================================

tstsuite_begin "Tag Filter Validation Tests" "$0"

# Build all tag-related tests
tstnote "Building test binaries..."
if make s_tags s_tags_test s_tag_edge_cases > /dev/null 2>&1; then
    tstnote "All test binaries built successfully"
else
    tstnote "Failed to build test binaries"
    tstsuite_end
    exit 1
fi

# ============================================================================
# Test s_tags_test.c - Basic tag system tests
# ============================================================================

tstcase_begin "s_tags_test - Basic Tag System Tests"

# No filter: only untagged test runs
test_filter $LINENO "s_tags_test" "No filter - only untagged runs" 1 0 6

# +RequiresDB: enables tests with +RequiresDB
test_filter $LINENO "s_tags_test" "+RequiresDB filter" 4 0 3 "+RequiresDB"

# -SlowTests: enables tests with -SlowTests
test_filter $LINENO "s_tags_test" "-SlowTests filter" 3 0 4 "-SlowTests"

# +*: enables all +Tag tests
test_filter $LINENO "s_tags_test" "+* wildcard - all positive tags" 5 0 2 "+*"

# +SlowTests: enables tests with +SlowTests
test_filter $LINENO "s_tags_test" "+SlowTests filter" 2 0 5 "+SlowTests"

# Combined: +* then -SlowTests
test_filter $LINENO "s_tags_test" "+* -SlowTests - combined filters" 6 0 1 "+*" "-SlowTests"

tstcase_end

# ============================================================================
# Test s_tag_edge_cases.c - Edge case tests
# ============================================================================

tstcase_begin "s_tag_edge_cases - Edge Case Tests"

# No filter: only untagged runs
test_filter $LINENO "s_tag_edge_cases" "No filter - only untagged runs" 1 0 10

# +SingleTag
test_filter $LINENO "s_tag_edge_cases" "+SingleTag filter" 2 0 9 "+SingleTag"

# -NegTag
test_filter $LINENO "s_tag_edge_cases" "-NegTag filter" 3 0 8 "-NegTag"

# +TagA
test_filter $LINENO "s_tag_edge_cases" "+TagA filter" 2 0 9 "+TagA"

# +My_Tag (underscore test)
test_filter $LINENO "s_tag_edge_cases" "+My_Tag filter" 2 0 9 "+My_Tag"

# +VeryLongTagNameThatIsStillValid
test_filter $LINENO "s_tag_edge_cases" "Long tag name filter" 2 0 9 "+VeryLongTagNameThatIsStillValid"

# +* wildcard
test_filter $LINENO "s_tag_edge_cases" "+* wildcard" 9 0 2 "+*"

tstcase_end

# ============================================================================
# Test s_tags.c - Comprehensive tag tests
# ============================================================================

tstcase_begin "s_tags - Comprehensive Truth Table Tests"

# No filter: only untagged runs
test_filter $LINENO "s_tags" "No filter - only untagged runs" 1 0 20

# Basic single tag filters
test_filter $LINENO "s_tags" "+TAG filter" 4 0 17 "+TAG"
test_filter $LINENO "s_tags" "-TAG filter" 4 0 17 "-TAG"
test_filter $LINENO "s_tags" "+OTHER filter" 4 0 17 "+OTHER"
test_filter $LINENO "s_tags" "-OTHER filter" 4 0 17 "-OTHER"

# Wildcard
test_filter $LINENO "s_tags" "+* wildcard - all positive tags" 15 0 6 "+*"

# Combined filters
test_filter $LINENO "s_tags" "+* then -TAG (additive)" 14 0 7 "+*" "-TAG"
test_filter $LINENO "s_tags" "+* then -OTHER (additive)" 14 0 7 "+*" "-OTHER"

# Override behavior
test_filter $LINENO "s_tags" "+TAG then +OTHER (override)" 5 0 16 "+TAG" "+OTHER"
test_filter $LINENO "s_tags" "+TAG then -TAG (override)" 4 0 17 "+TAG" "-TAG"
test_filter $LINENO "s_tags" "-TAG then +TAG (override)" 4 0 17 "-TAG" "+TAG"

# Specific tag name tests
test_filter $LINENO "s_tags" "+Alpha filter" 2 0 19 "+Alpha"
test_filter $LINENO "s_tags" "+Database filter" 2 0 19 "+Database"
test_filter $LINENO "s_tags" "-Slow filter" 2 0 19 "-Slow"

# Edge case tag names
test_filter $LINENO "s_tags" "+Tag123 (numbers in tags)" 2 0 19 "+Tag123"
test_filter $LINENO "s_tags" "+My_Tag (underscores)" 2 0 19 "+My_Tag"
test_filter $LINENO "s_tags" "Long tag name" 2 0 19 "+VeryLongTagNameThatIsStillValidAndShouldWork"

# Case sensitivity
test_filter $LINENO "s_tags" "+lowercase (case sensitive)" 2 0 19 "+lowercase"
test_filter $LINENO "s_tags" "+UPPERCASE (no match)" 1 0 20 "+UPPERCASE"

# Wildcard behavior tests
test_filter $LINENO "s_tags" "+Wild1 filter" 2 0 19 "+Wild1"
test_filter $LINENO "s_tags" "-Wild1 filter" 2 0 19 "-Wild1"

# Override behavior
test_filter $LINENO "s_tags" "+Override filter" 2 0 19 "+Override"
test_filter $LINENO "s_tags" "-Override filter" 2 0 19 "-Override"

tstcase_end

# ============================================================================
# Final Summary
# ============================================================================

tstnote "Tag filter validation complete"
tstsuite_end

rm -f s_tags s_tags_test s_tag_edge_cases 

# Exit with appropriate code
if [ "$TST_TOTAL_FAIL" -eq 0 ]; then
    exit 0
else
    exit 1
fi
