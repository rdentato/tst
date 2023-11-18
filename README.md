&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
<img height="150" src="https://github.com/rdentato/tst/assets/48629/248f5856-13bd-4e35-8d9f-0b74a0ecb010"> <br/>
# tst
A very simple, small, single-header, unit test framework for C (and C++).<br/>
(Join us on our [Discord](https://discord.gg/BqsZjDaUxg) channel for queries and feedback)

## Introduction
`tst` is a lightweight unit testing framework designed for C programs (but works for C++ as well). 
It provides a suite of functionalities to define, group, and validate test cases, while offering utilities
for expressive reporting and diagnostic messaging. With minimal syntax, `tst` fosters easy test integration into C projects.

I never understood why some of the most common unit test frameworks had so many files and complex build dependencies so
I decided to write a small (less than 300 lines of code), no-dependencies unit testing framework that still has most of the 
feautures you can find in much more complex ones.

If you want to use `tst`, just include `tst.h` and you're ready to write your test cases.

Check the [**tutorial**](tutorial/) for a detailed description of how to use `tst` or the [**reference manual**](tutorial/reference.md)
for a short description of each function.\

Fell free to provide ideas, bugs, suggestions or even full Pull Requests if you feel inclined to do so!

## Supported platforms

`tst.h` has been tested and confirmed to work on the following combination of platforms/compilers. If you find
any issue on using `tst.h` with your compiler, please let me know.

- `gcc`         On Linux and Windows WSL2 
- `g++`         On Linux and Windows WSL2 
- `clang`       On Linux and Windows WSL2 
- `cl (C)`      On Windows (see the `test-cl` directory)
- `cl (C++)`    On Windows (see the `test-cl` directory)
- `mingw-gcc`   On Windows


## A small Example
```c
#include "tst.h"  // Ensure the tst framework is included

tstsuite("Primary Test Suite")
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
----- SUIT ▷ t_small_example.c "Primary Test Suite"
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
## Running your tests

There is no limitation on how you organize and run your tests.
Just for the purpose of self-testing (and to provide an example on how you could 
organize your testing workflow) the `tstrun` script in the `src` directory provides
a convenient way to launch groups of tests and pass them arguments.
