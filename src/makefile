# SPDX-FileCopyrightText: © 2025 Remo Dentato <rdentato@gmail.com>
# SPDX-License-Identifier: MIT

# Detect platform
_EXE=.exe
STATIC=-static

ifeq "$(COMSPEC)" ""
_EXE=
STATIC=
endif

# Compiler settings
CC=gcc
CFLAGS=-O2 -Wall $(STATIC)
LIBS=-lm

# Default target - build both
all: t2h

# Test t2h with example log
test-t2h: t2h
	@echo "=== Testing t2h with ../test/test.log ==="
	./t2h ../test/test.log > test_output.html
	@echo "✓ HTML generated: test_output.html"
	@echo "  Suites: $$(grep -c 'suite-section' test_output.html)"
	@echo "  Test cases: $$(grep -c 'test-case\"' test_output.html)"
	@echo "  Source tabs: $$(grep -c 'Source Code' test_output.html)"

# Clean build artifacts
clean:
	rm -f t2h t2h.exe t2h.o
	rm -f test_output.html

.PHONY: all clean test-t2h
