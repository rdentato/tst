&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
<img height="150" src="https://github.com/rdentato/tst/assets/48629/248f5856-13bd-4e35-8d9f-0b74a0ecb010"> <br/>
# tst
A vary simple, single-header, unit test framework for C (and C++). (Join us on [Discord](https://discord.gg/BqsZjDaUxg)!)

## Introduction
`tst` is a lightweight unit testing framework designed for C programs (but works for C++ as well). 
It provides a suite of functionalities to define, group, and validate test cases, while offering utilities
for expressive reporting and diagnostic messaging. With minimal syntax, `tst` fosters easy test integration into C projects.

I never understood why some of the most common unit test frameworks had so many files and complex build.
If you want to use `tst`, just include `tst.h` and you're ready to write your test cases.

Check the **tutorial** for a detailed description of how to use `tst`,

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
    
    tstclock("Check counting time") {
      volatile int b = 1;
      // Code to analyze...
      for (int a = 1; a < 100 ; a++) b = a + b;
    }
    
    tstcase("Edge Cases") {
      tstskipif(1 == 1) {  // Next tests will be skipped!
        tstcheck(0 < 1, "0 should be less than 1");
        tstassert(1 >= 1, "1 should be equal to 1");
      }

      tstskipif(1 != 2) {  // Next tests will be executed!
        tstcheck(0 < 1, "0 should be less than 1");
        tstassert(1 >= 1, "1 should be equal to 1");
      }
    }
    
    tstnote("Testing Complete. Review for any FAIL flags.");
}
```

Compile and run the above program (no need for a `main()` function)
to execute all the tests and generate a log like this:

```
----- FILE ▷ t_small_example.c "Primary Test Suite"
    5 CASE┬── Equality Checks 1, 1
    6 PASS│  1 == 1
    7 FAIL├┬ 1 != 1
          │╰ Failed on purpose
    5     ╰── 1 KO | 1 OK | 0 SKIP
   10 CLCK⚑  1 µs Check counting time
   16 CASE┬── Edge Cases
   17 NOTE: SKIP condition: `1 == 1`
   18 SKIP│  0 < 1
   19 SKIP│  1 >= 1
   22 NOTE: SKIP condition: `1 != 2`
   23 SKIP│  0 < 1
   24 SKIP│  1 >= 1
   16     ╰── 0 KO | 0 OK | 4 SKIP
   28 NOTE: Testing Complete. Review for any FAIL flags.
^^^^^ RSLT ▷ 1 KO | 1 OK | 4 SKIP
```
