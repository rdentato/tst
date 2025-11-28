#!/bin/bash

echo "========================================="
echo "Demonstrating --report-error behavior"
echo "========================================="
echo

echo "Test 1: Without --report-error (no failures)"
echo "Command: ./t_report_error_demo"
OUTPUT=$(./t_report_error_demo 2>&1)
EXIT_CODE=$?
echo "$OUTPUT" | tail -2
echo "Exit code: $EXIT_CODE"
echo

echo "Test 2: Without --report-error (with failures)"
echo "Command: ./t_report_error_demo +demo"
OUTPUT=$(./t_report_error_demo +demo 2>&1)
EXIT_CODE=$?
echo "$OUTPUT" | tail -2
echo "Exit code: $EXIT_CODE"
echo

echo "Test 3: With --report-error (no failures)"
echo "Command: ./t_report_error_demo --report-error"
OUTPUT=$(./t_report_error_demo --report-error 2>&1)
EXIT_CODE=$?
echo "$OUTPUT" | tail -2
echo "Exit code: $EXIT_CODE"
echo

echo "Test 4: With --report-error (with failures) ⚠️"
echo "Command: ./t_report_error_demo +demo --report-error"
OUTPUT=$(./t_report_error_demo +demo --report-error 2>&1)
EXIT_CODE=$?
echo "$OUTPUT" | tail -2
echo "Exit code: $EXIT_CODE ← Returns 1 to indicate failure!"
echo

echo "========================================="
echo "Summary:"
echo "  - Without flag: Always exit 0 (Tests 1 & 2)"
echo "  - With flag + no failures: Exit 0 (Test 3)"
echo "  - With flag + failures: Exit 1 (Test 4) ✓"
echo ""
echo "Use --report-error in CI/CD to fail builds"
echo "when tests fail!"
echo "========================================="
