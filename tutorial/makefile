#  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
#  SPDX-License-Identifier: MIT

# Uncomment the following line to generate a minimal log.
# COMPACT="-DTST_STR_COMPACT"

# Uncomment the following line to disable ANSI colors
# NOCOLOR="-DTST_STR_NOCOLOR"

# Uncomment the following line to enable debugging
# DEBUG="-DDEBUG"

_EXE=.exe
STATIC=-static

ifeq "$(COMSPEC)" ""
_EXE=
STATIC=
endif

CXX_AVAILABLE := $(shell $(CXX) --version 2>/dev/null)

CFLAGS=-O2 -Wall -I../src -I. $(STATIC) $(COMPACT) $(NOCOLOR) $(DEBUG)
LIBS=

TESTS_SRC_C=$(wildcard t_*.c)
TESTS_C=$(TESTS_SRC_C:.c=)

ifneq ($(CXX_AVAILABLE),)
  TESTS_SRC_CPP=$(wildcard t_*.cpp)
  TESTS_CPP=$(TESTS_SRC_CPP:.cpp=)
	CXXFLAGS=$(CFLAGS)
endif

TESTS_SRC=$(TST_SRC_C) $(TST_SRC_CPP)
TESTS=$(TESTS_C) $(TESTS_CPP)

# targets
all: $(TESTS)

SRCDIR=../src

runtest: all
	../src/tstrun

MAKEFLAGS += --no-builtin-rules

%.o: %.c
	$(CC) $(CFLAGS) -o $*.o -c $< 

%: %.o functions.o
	$(CC) -o $* $< functions.o -lm

.PRECIOUS: %.o

clean:
	rm -f $(TESTS) $(TESTS:=.exe) $(TESTS:=.o) test.log functions.o

