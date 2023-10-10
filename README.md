# tst
A minimalistic unit test framework for C.

## Introduction
`tst` is a lightweight unit testing framework designed for C programs. It provides a suite of functionalities to define, group, and validate test cases, while offering utilities for expressive reporting and diagnostic messaging. With minimal syntax, `tst` fosters easy test integration into C projects.

I never understood why some of the most common unit test frameworks had so many files and complex build.
If you want to use `tst`, just include `tst.h` and you're ready to write your test cases.

## Core Functions Overview
1. **tstrun( ... )**
   - **Purpose**: Groups test cases and outputs the title.
   - **Example**:
     ```c
     tstrun("Running all tests") {
       // Your test cases here...
     }
     ```
   
2. **tstcase( ... )**
   - **Purpose**: Defines a test case, with a formatted title.
   - **Example**:
     ```c
     tstcase("Testing equality of %d and %d", 1, 1) {
       // Your checks/assertions here...
     }
     ```
   
3. **tstcheck(int test, ... )**
   - **Purpose**: Validates whether `test` is true and outputs either "PASS" or "FAIL" alongside an optional message.
   - **Example**:
     ```c
     tstcheck(1 == 1, "Value mismatch! Expected equality.");
     ```
   
4. **tstassert(int test, ... )**
   - **Purpose**: Similar to `tstcheck` but halts program execution upon failure.
   - **Example**:
     ```c
     tstassert(1 == 1, "Critical: Value mismatch!");
     ```
   
5. **tstgroup(int test, ... )**
   - **Purpose**: Conditionally executes a block of checks; skips if `test` is false.
   - **Example**:
     ```c
     tstgroup(1 == 1) {
       tstcheck(2 == 2, "Secondary check failed!");
     }
     ```
   
6. **tstclk( ... )**
   - **Purpose**: Measures and displays elapsed time for specified instructions.
   - **Example**:
     ```c
     tstclk({
       // Code block to measure...
     });
     ```
   
7. **tstdata( ... )**
   - **Purpose**: Attaches data blocks for enhanced test contextualization.
   - **Example**:
     ```c
     tstdata("Relevant test data", dataPointer);
     ```
   
8. **tstnote( ... )**
   - **Purpose**: Allows for the insertion of contextual notes within test output.
   - **Example**:
     ```c
     tstnote("Note: This test is pivotal for module X integrity.");
     ```
   
## Comprehensive Example
```c
#include "tst.h"  // Ensure the tst framework is included

void myData(FILE *f)
{
  fprintf(f,"\nMYDATA MYDATA MYDATA MYDATA MYDATA\n MYDATA MYDATA MYDATA MYDATA \n");
}

int main(int argc, char *argv[])
{
  tstrun("Primary Test Suite") {
    
    tstcase("Equality Checks %d, %d", 1, 1) {
      tstcheck(1 == 1, "Mismatch: %d != %d", 1, 1);
      tstcheck(1 != 1, "Failed on purpose");
    }
    
    tstcase("Time Complexity Analysis") {
      tstclk("Check counting time") {
        volatile int b = 1;
        // Code to analyze...
        for (int a = 1; a < 100 ; a++) b = a + b;
      }
    }
    
    tstcase("Grouped Checks: Edge Cases") {
      tstgroup(1 == 2, "Inequality" ) {  // Will be skipped!
        tstcheck(0 < 1, "0 should be less than 1");
        tstassert(1 >= 1, "1 should be equal to 1");
      }

      tstgroup(1 != 2,"Equality") {  // Will be executed!
        tstcheck(0 < 1, "0 should be less than 1");
        tstassert(1 >= 1, "1 should be equal to 1");
      }
    }
    
    tstdata("Useful Debug Data") {
       myData(stderr);
    }
    
    tstnote("Testing Complete. Review for any FAIL flags.");
  }
}
```
## Test Results:
```
FILE ▷ tst_test.c Primary Test Suite
CASE┬── Equality Checks 1, 1 » tst_test.c:12
PASS│  1 == 1 » tst_test.c:13
FAIL├┬ 1 != 1 » tst_test.c:14
    │╰ Failed on purpose
    ╰── 1 KO | 1 OK | 0 SKIP
CASE┬── Time Complexity Analysis » tst_test.c:17
CLCK│⚑ 0.001000 ms. Check counting time » tst_test.c:18
    ╰── 0 KO | 0 OK | 0 SKIP
CASE┬── Grouped Checks: Edge Cases » tst_test.c:25
SKIP├┬ 1 == 2 » tst_test.c:26
    │╰ Inequality
PASS│  1 != 2 » tst_test.c:31
PASS│  0 < 1 » tst_test.c:32
PASS│  1 >= 1 » tst_test.c:33
    ╰── 0 KO | 2 OK | 1 SKIP
DATA│ ▽▽▽ Useful Debug Data » tst_test.c:37

MYDATA MYDATA MYDATA MYDATA MYDATA
 MYDATA MYDATA MYDATA MYDATA 

DATA│ △△△
NOTE 🗎 Testing Complete. Review for any FAIL flags. » tst_test.c:41
RSLT ▷ 1 KO | 3 OK | 1 SKIP
```
## Conclusion
`tst` offers a user-friendly syntax to facilitate streamlined testing without exhaustive setup or dependencies. Developers may swiftly integrate, run, and diagnose tests, ensuring the robustness and reliability of their C code.

**Note**: Always consider possible improvements or expansions to the tool to match the specific needs of your project, and feel free to contribute to its development a this is an open-source tool.


