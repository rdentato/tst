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

# Comment the following line if you don't want to compile with C++
CXX_AVAILABLE := $(shell $(CXX) --version 2>/dev/null)

CFLAGS=-O2 -Wall -I../src -I. $(STATIC) $(COMPACT) $(DEBUG)
LIBS=

TESTS_SRC_C=$(wildcard t_*.c)
TESTS_C=$(TESTS_SRC_C:.c=)

ifneq ($(CXX_AVAILABLE),)
#  TESTS_SRC_CPP=$(wildcard t_*.cpp)
  TESTS_SRC_CPP := $(addsuffix ++.cpp,$(TESTS_C))
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
	-@for f in $(TESTS); do ./$$f $(TSTTAGS) 2>> test.log; done 
	@echo -n "LOG: test.log  FAIL: `grep '^[0-9 ]*FAIL' test.log | wc -l` "
	@echo -n "PASS: `grep '^[0-9 ]*PASS' test.log | wc -l`  "
	@echo    "SKIP: `grep '^[0-9 ]*SKIP' test.log | wc -l`"
	@echo "EXPECTED 4 FAIL HERE △"

.PRECIOUS: %.o
.PRECIOUS: %++.cpp
.PRECIOUS: %++

%++.cpp:%.c
	@echo "#include <iostream>" > $*++.cpp
	@echo "#include <tst.h>" >> $*++.cpp
	@cat $*.c >> $*++.cpp

%++:%++.cpp
	$(CXX) $(CXXFLAGS) $*++.cpp -o $*++

clean:
	rm -f $(TESTS) $(TESTS:=.exe) $(TESTS:=.o) *++.cpp test.log

