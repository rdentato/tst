#  SPDX-Creator-Person: Remo Dentato (rdentato@gmail.com)
#  SPDX-License-Identifier: MIT

_EXE=.exe
STATIC=-static

ifeq "$(COMSPEC)" ""
_EXE=
STATIC=
endif

CFLAGS=-O2 -Wall -I../src $(STATIC) -DDEBUG
LIBS=

TESTS_SRC=$(wildcard t_*.c)
TESTS=$(TESTS_SRC:.c=)

# implicit rules
MAKEFLAGS += --no-builtin-rules

# targets
all: $(TESTS)

SRCDIR=../src

runtest: all
	@rm -f test.log
	@for f in $(TESTS); do ./$$f 2>> test.log; done 
	@echo -n "LOG: test.log  FAIL: `grep '^FAIL' test.log | wc -l`  "
	@echo -n "PASS: `grep '^PASS' test.log | wc -l`  "
	@echo    "SKIP: `grep '^SKIP' test.log | wc -l`"


#%.o: %.c 
#	$(CC) $(CFLAGS) -c $< -o $@

%: %.c 
	$(CC) $(CFLAGS) $< -o $* $(LIBS)

#.PRECIOUS: %.o

clean:
	rm -f $(TESTS) $(TESTS:=.exe) $(TESTS:=.o) test.log

