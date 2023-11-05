&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
<img height="150" src="https://github.com/rdentato/tst/assets/48629/248f5856-13bd-4e35-8d9f-0b74a0ecb010"> <br/>
# tst
A vary somple, single-header, unit test framework for C (and C++). (Join us on [Discord](https://discord.gg/BqsZjDaUxg)!)

## Introduction
`tst` is a lightweight unit testing framework designed for C programs (but works for C++ as well). 
It provides a suite of functionalities to define, group, and validate test cases, while offering utilities
for expressive reporting and diagnostic messaging. With minimal syntax, `tst` fosters easy test integration into C projects.

I never understood why some of the most common unit test frameworks had so many files and complex build.
If you want to use `tst`, just include `tst.h` and you're ready to write your test cases.

Check the [tutorial](#tutorial/tutorial.md) for a detailed description of how to use `tst`,

Fell free to provide ideas, bugs, and suggestions!

## A small Example
```c
#include "tst.h"  // Ensure the tst framework is included

tstrun("Primary Test Suite")
{    
    tstcase("Equality Checks %d, %d", 1, 1) {
      tstcheck(1 == 1, "Mismatch: %d != %d", 1, 1);
      tstcheck(1 != 1, "Failed on purpose");
    }
    
    tstcase("Time Complexity Analysis") {
      tstclock("Check counting time") {
        volatile int b = 1;
        // Code to analyze...
        for (int a = 1; a < 100 ; a++) b = a + b;
      }
    }
    
    tstcase("Grouped Checks: Edge Cases") {
      tstif(1 == 2, "Inequality" ) {  // Will be skipped!
        tstcheck(0 < 1, "0 should be less than 1");
        tstassert(1 >= 1, "1 should be equal to 1");
      }

      tstif(1 != 2,"Equality") {  // Will be executed!
        tstcheck(0 < 1, "0 should be less than 1");
        tstassert(1 >= 1, "1 should be equal to 1");
      }
    }
    
    tstnote("Testing Complete. Review for any FAIL flags.");
}
```

Compile and run the above program (no need for a `main()` function)
to execute all the tests.

## Test Results:
```
------ FILE ▷ tst_test.c "Primary Test Suite"
    12 CASE┬── Equality Checks 1, 1
    13 PASS│  1 == 1
    14 FAIL├┬ 1 != 1
           │╰ Failed on purpose
    12     ╰── 1 KO | 1 OK | 0 SKIP
    17 CASE┬── Time Complexity Analysis
    18 CLCK│⚑ 0.001000 ms. Check counting time
    17     ╰── 0 KO | 0 OK | 0 SKIP
    25 CASE┬── Grouped Checks: Edge Cases
    26 SKIP├┬ 1 == 2
           │╰ Inequality
    31 PASS│  1 != 2
    32 PASS│  0 < 1
    33 PASS│  1 >= 1
    25     ╰── 0 KO | 2 OK | 1 SKIP
    41 NOTE 🗎 Testing Complete. Review for any FAIL flags.
^^^^^^ RSLT ▷ 1 KO | 3 OK | 1 SKIP
```
