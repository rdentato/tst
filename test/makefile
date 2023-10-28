#  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
#  SPDX-License-Identifier: MIT

_EXE=.exe
STATIC=-static

ifeq "$(COMSPEC)" ""
_EXE=
STATIC=
endif

CXX_AVAILABLE := $(shell $(CXX) --version 2>/dev/null)

CFLAGS=-O2 -Wall -I../src -I. $(STATIC) -DDEBUG
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
	@rm -f test.log
	@for f in $(TESTS); do ./$$f $(TSTTAGS) 2>> test.log; done 
	@echo -n "LOG: test.log  FAIL: `grep '^FAIL' test.log | wc -l` "
	@echo -n "PASS: `grep '^PASS' test.log | wc -l`  "
	@echo    "SKIP: `grep '^SKIP' test.log | wc -l`"
	@echo "EXPECTED 1 FAIL HERE △"


#.PRECIOUS: %.o

clean:
	rm -f $(TESTS) $(TESTS:=.exe) $(TESTS:=.o) test.log

