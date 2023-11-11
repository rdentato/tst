# Checking the Microsoft Visual Studio C/C++ compiler.

When used in a Visual Studio project, `tst.h` will simply be added to your list of headers
with no need for any additional configuration.

The `runtest` script in this directory only serves as check that everything works fine with the
Microsoft C and C++ compilers.

If your antivirus is as aggressve as mine, it takes some time before allowing the test 
programs to run for the first time. You just have to wait a little bit.

I've tested it with `cl` version 19.37.32825 on PowerShell and the standard CMD shell.

