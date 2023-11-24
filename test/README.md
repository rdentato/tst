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

