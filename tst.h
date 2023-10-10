//  (C) by Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#ifndef TST_VERSION // 0.1.0-beta
#define TST_VERSION    0x0001000B

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

static volatile int tst_zero = 0;
static int tst_case_pass = 0;
static int tst_case_fail = 0;
static int tst_case_skip = 0;
static int tst_pass = 0;
static int tst_fail = 0;
static int tst_skip = 0;
static int tst_case = 0;

#define tst_init_case()   (tst_case_pass=tst_case_fail=tst_case_skip=0)
#define tst_init_run()    (tst_pass=tst_fail=tst_skip=tst_init_case())

#define tst_prtf(...)     (fprintf(stderr, __VA_ARGS__), fprintf(stderr, " » %s:%d\n", __FILE__, __LINE__), tst_zero=0)

#define tstcheck(x_,...)  do { errno = !(x_); tst_prtf("%s %s", errno?(tst_fail++, tst_case_fail++,"FAIL├┬") \
                                                                     :(tst_pass++, tst_case_pass++,"PASS│ "), #x_);\
                               if (errno) {fprintf(stderr,"    │╰ " __VA_ARGS__); fputc('\n',stderr);} \
                          } while(0)

#define tstassert(...) do { tstcheck(__VA_ARGS__); if (errno) abort();} while(0)

#define tstrun(...)    for (int tst=!(fputs ("FILE ▷ " __FILE__, stderr), fprintf(stderr," " __VA_ARGS__ ), fputc('\n',stderr), tst_init_run()); \
                             tst ; tst = 0, fprintf(stderr,"RSLT ▷ %d KO | %d OK | %d SKIP\n", tst_fail, tst_pass, tst_skip))

#define tstcase(...)   for (tst_case=!(tst_prtf("CASE┬── " __VA_ARGS__),tst_init_case());  tst_case ;\
                             fprintf(stderr,"    ╰── %d KO | %d OK | %d SKIP\n", tst_case_fail, tst_case_pass, tst_case_skip), tst_case = 0) 

#define tstgroup(x_,...)  if (!(x_) && (tst_prtf("SKIP├┬ " #x_), fprintf(stderr,"    │╰ " __VA_ARGS__), fputc('\n',stderr), ++tst_skip, ++tst_case_skip)) ; \
                          else if (tst_prtf("PASS│  " #x_)) ; else

#define tstclk(...) \
  for(clock_t clk=clock(); clk; clk=clock()-clk,fprintf(stderr,"CLCK│⚑ %f ms. ",((float)clk)/ ((float)CLOCKS_PER_SEC/1000.0)), \
                                clk=tst_prtf(__VA_ARGS__))

#define tstdata(...) \
  for(int tst = 1 + (fflush(stdout) , tst_prtf("DATA│ ▽▽▽ " __VA_ARGS__)); tst; tst = 0, fflush(stdout), fputs("\nDATA│ △△△\n",stderr))

#define tstnote(...) (fprintf(stderr,"NOTE%s🗎",tst_case?"│":" "), tst_prtf(" " __VA_ARGS__))

#define tst_check(...)
#define tst_assert(...)
#define tst_note(...)
#define tst_case(...)  if (!tst_zero) ; else
#define tst_group(...) if (!tst_zero) ; else
#define tst_clk(...)   if (tst_zero)  ; else


#endif // TST_VERSION
