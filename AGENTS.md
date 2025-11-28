# Agent Guidelines for TST Project

## Roles
You will perform different roles depending on the tasks
- **Software Architect**: When asked about modules architectures and program structures.
- **Developer**: When asked about producing code in any language.
- **Analyst**: When asked to create requirements from a problem statement.
- **Technical Writer**: When asked to create documentation and other text material.
- **ANY OTHER EXPLICIT ROLE**: When explicitly asked to perform a task from a point of view of a speific role.

## Build/Test Commands
- **Build all utilities**: `cd src && make` (builds `t2h`)
- **Run all tests**: `make runtest` (from root) or `cd test && make runtest`
- **Run single test**: `cd test && ./t_tst00` (replace with specific test name)
- **Build single test**: `cd test && make t_tst00`
- **Clean**: `make clean` (root) or `cd test && make clean`
- **Test t2h**: `cd src && make test-t2h`

## Code Style
- **Language**: C (C99 compatible), some C++ support for tests
- **Headers**: SPDX headers required (`SPDX-FileCopyrightText`, `SPDX-License-Identifier: MIT`)
- **Includes**: Standard library only (`stdio.h`, `stdlib.h`, `string.h`, etc.), use `tst.h` for tests
- **Naming**: Snake_case for functions/variables, UPPER_CASE for macros/defines
- **Formatting**: K&R style braces, 2-space indents (inferred from source)
- **Comments**: `//` for single-line, `/* */` for multi-line; detailed function headers
- **Error handling**: Return 0 on success, errno for out-of-range conditions
- **Macros**: Extensive use of variadic macros for test framework (see `tst.h`)
- **Platform**: Cross-platform (Linux/WSL/Windows), detect with `COMSPEC` in makefiles
- **Compiler**: gcc primary, support for cl (MSVC), g++, clang, mingw-gcc
- **Dependencies**: Single-header design (`tst.h`), no external dependencies

## Behaviour
- **Preserve Backup**: DO NOT modify any backup files (`*.bak`, `*_orig.*`, `*copy*`, `*backup*`, etc.)
  