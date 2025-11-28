# Organizing tests

Besides being the unit test suite for `tst`, the `makefile` and the
`tstrun` script in thi directory also can serve as an example on how
to organize and launch tests.

The assumption is all test program start with `t_`, that they are all
in the same directory and that they have to be tested both with C and C++
compilers. 

You can organize things differently, for example
having separate directories for the tests related different modules or 
using a different naming convention.

## Tag Filter Test Suite

The test suite includes comprehensive tests for the tag filtering functionality:

- **t_tags.c** - Comprehensive test suite with 21 test cases covering:
  - Single positive/negative tags (+TAG, -TAG)
  - Multiple tags per test case
  - Wildcard behavior (+*)
  - Combined filters (+* -TAG)
  - Override behavior (left-to-right processing)
  - Edge cases: numbers, underscores, long names, case sensitivity

- **t_tag_edge_cases.c** - Additional edge case tests (11 test cases)
  - Empty strings, special characters
  - Boundary conditions

- **test_all_tag_combinations.sh** - Automated validation script
  - Validates all 23 tag filter combinations
  - Ensures correct PASS/SKIP counts for each filter
  - Provides color-coded output for easy verification

Run the comprehensive tag test suite:
```bash
./test_all_tag_combinations.sh
```

