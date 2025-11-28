# TST Source Directory

This directory contains the core components of the TST (Test) framework - a lightweight, single-header C testing framework with support for both C and shell-based tests.

## Contents

### Core Framework

#### `tst.h`
The heart of the TST framework - a single-header C library that provides all testing macros and functionality.

**Features:**
- Variadic macros for test assertions (`pass`, `fail`, `skip`)
- Test case organization (`case`, `section`)
- Test suite management (`suite`)
- Tag-based test filtering
- Clock timing measurement
- Output capture (`screen`)
- List mode for test discovery
- Cross-compiler support (gcc, cl, g++, clang, mingw-gcc)

**Usage:**
```c
#include "tst.h"

suite ("My Test Suite") {
  case ("Test description") {
    pass(1 + 1 == 2);
    fail(1 + 1 == 3, "Math is broken!");
  }
}
```

**Documentation:**
- Reference: [../docs/ref_manual.md](../docs/ref_manual.md)
- Internals: [../docs/maintainer/tst_h_internals.md](../docs/maintainer/tst_h_internals.md)
- Tutorial: [../tutorial/README.md](../tutorial/README.md)

#### `tst.sh`
A bash library that provides TST-compatible testing functions for shell scripts.

**Features:**
- Shell functions mirroring tst.h API
- Automatic line number tracking via BASH_LINENO
- Suite/case/section management
- Pass/fail/skip assertions
- Clock timing
- Output conforming to TST log format

**Usage:**
```bash
source tst.sh

tstsuite_begin "Shell Test Suite" "$0"
  tstcase_begin "Test description"
    tstpass "Condition passed"
    tstfail "Condition failed" "Error message"
  tstcase_end
tstsuite_end
```

**Documentation:**
- Internals: [../docs/maintainer/tst_sh_internals.md](../docs/maintainer/tst_sh_internals.md)
- Example: [../test/t_tag_filters.sh](../test/t_tag_filters.sh)

### Utilities

#### `t2h.c`
TST log to HTML converter - transforms test output into interactive HTML dashboards.

**Features:**
- Single-pass parsing with state machine
- Multi-suite log support
- Source code extraction with syntax highlighting
- Interactive HTML with expandable test cases

**Usage:**
```bash
# From stdin
./t2h < test.log > report.html

# From files
./t2h test1.log test2.log > report.html

# Multiple suites
cat suite1.log suite2.log | ./t2h > report.html
```

**Documentation:**
- Architecture: [../docs/maintainer/t2h_architecture.md](../docs/maintainer/t2h_architecture.md)

### Build System

#### `makefile`
Cross-platform makefile for building TST utilities.

**Targets:**
- `make` or `make all` - Build t2h
- `make clean` - Remove build artifacts

**Platform Detection:**
- Automatically detects Windows (via COMSPEC)
- Adjusts compiler flags and executable extensions
- Supports gcc with static linking on Linux
- Handles .exe extension on Windows

**Variables:**
- `CC` - Compiler (default: gcc)
- `CFLAGS` - Compilation flags (-O2 -Wall)
- `STATIC` - Static linking flag (Linux only)
- `_EXE` - Executable extension (.exe on Windows)


## File Relationships

```
tst.h ─────────┐
               ├──> Test executables ──> .log files ──> t2h ──> .html reports
tst.sh ────────┘
```

1. Tests use either `tst.h` (C/C++) or `tst.sh` (bash)
2. Tests output TST-formatted logs to stderr
3. Logs are converted to HTML using `t2h`
4. HTML reports provide interactive test visualization

## Dependencies

### tst.h
- **External:** None (single-header, standard library only)
- **Standards:** C99 compatible
- **Platforms:** Linux, Windows, WSL, macOS

### tst.sh
- **External:** bash, standard Unix utilities (date, printf)
- **Standards:** Bash 3.0+
- **Platforms:** Linux, WSL, macOS, Git Bash (Windows)

### t2h
- **External:** None (standard library only)
- **Standards:** C99 compatible
- **Platforms:** Linux, Windows, WSL, macOS

## Version Information

- **tst.h:** Version 0.8.1-beta (see `TST_VERSION` macro: 0x0008001B)
- **t2h:** Version 1.0.0 (see `VERSION` constant)
- **tst.sh:** No version constant (tracks tst.h releases)

## Cross-Platform Notes

### Compiler Support
- **gcc** - Primary compiler, fully supported
- **cl** (MSVC) - Supported with pragma warnings disabled
- **g++** - Supported for C++ test code
- **clang** - Compatible (same flags as gcc)
- **mingw-gcc** - Windows support via MinGW

### Platform-Specific Code
- **Windows detection:** Via `COMSPEC` environment variable in makefiles
- **Static linking:** Linux only (prevents glibc version issues)
- **Warning suppression:** MSVC-specific pragmas in tst.h

## Development Workflow

1. **Modify tst.h or tst.sh** - Update test framework
2. **Run tests** - `cd test && make runtest`
3. **Modify t2h.c** - Update HTML generator
4. **Test t2h** - `cd src && make test-t2h`
5. **Review HTML** - Open `test_output.html` in browser

## See Also

- **Framework documentation:** [../docs/](../docs/)
- **Test examples:** [../test/](../test/)
- **Tutorial:** [../tutorial/](../tutorial/)
