//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT
//  SPDX-PackageVersion: 0.4.0-beta

#ifndef TST_VERSION
#define TST_VERSION 0x0004000B

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

static volatile short tst_zero = 0;
static unsigned short tst_case_pass = 0; // A test case can't have more than 65535 checks!
static unsigned short tst_case_fail = 0;
static unsigned short tst_case_skip = 0;
static int tst_pass = 0; // A test run can have up to 2 Billions checks!
static int tst_fail = 0;
static int tst_skip = 0;
static short tst_result = 0;
static const char* tst_title;

#define tst__cnt(_1,_2,_3,_4,_5,_6,_7,_8,_9,_N, ...) _N
#define tst__argn(...)  tst__cnt(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define tst__cat2(x,y)  x ## y
#define tst__cat(x,y)   tst__cat2(x,y)
#define tst_vrg(tst__f,...) tst__cat(tst__f, tst__argn(__VA_ARGS__))(__VA_ARGS__)

// TAGS
#define tst_tags(...) tst_vrg(tst_tags_,__VA_ARGS__)
#define tst_tags_1(_0)                         tst_tags__(0,_1,_2,_3,_4,_5,_6,_7,_8)  
#define tst_tags_2(_0,_1)                      tst_tags__(1,_1,_2,_3,_4,_5,_6,_7,_8)
#define tst_tags_3(_0,_1,_2)                   tst_tags__(2,_1,_2,_3,_4,_5,_6,_7,_8)
#define tst_tags_4(_0,_1,_2,_3)                tst_tags__(3,_1,_2,_3,_4,_5,_6,_7,_8)
#define tst_tags_5(_0,_1,_2,_3,_4)             tst_tags__(4,_1,_2,_3,_4,_5,_6,_7,_8) 
#define tst_tags_6(_0,_1,_2,_3,_4,_5)          tst_tags__(5,_1,_2,_3,_4,_5,_6,_7,_8) 
#define tst_tags_7(_0,_1,_2,_3,_4,_5,_6)       tst_tags__(6,_1,_2,_3,_4,_5,_6,_7,_8) 
#define tst_tags_8(_0,_1,_2,_3,_4,_5,_6,_7)    tst_tags__(7,_1,_2,_3,_4,_5,_6,_7,_8) 
#define tst_tags_9(_0,_1,_2,_3,_4,_5,_6,_7,_8) tst_tags__(8,_1,_2,_3,_4,_5,_6,_7,_8) 

static unsigned char tst_tags_val = 0xFF;

#define tsttag(t_) (!!((tst_tag_ ## t_) & tst_tags_val))

#define tst_tags__(n_,_1,_2,_3,_4,_5,_6,_7,_8) \
   static unsigned char tst_tag_##_1=0x01; static unsigned char tst_tag_##_2=0x02; \
   static unsigned char tst_tag_##_3=0x04; static unsigned char tst_tag_##_4=0x08; \
   static unsigned char tst_tag_##_5=0x10; static unsigned char tst_tag_##_6=0x20; \
   static unsigned char tst_tag_##_7=0x40; static unsigned char tst_tag_##_8=0x80; \
   static const char *tst_tag_names[8] = {#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8}; \
   static inline int tst_parsetags(int argc, const char **argv) {return tst_parse_tags(argc,argv, n_, tst_tag_names);}\
   static inline int tst_tags_zero() { return tst_zero & (tst_tag_##_1 | tst_tag_##_2 | tst_tag_##_3| tst_tag_##_4| \
                                                          tst_tag_##_5 | tst_tag_##_6 | tst_tag_##_7| tst_tag_##_8); }

static inline int tst_tags_zero(); // tst_tags_zero() always returns 0 and is used just to avoid compiler warnings.

static inline int tst_parse_tags(int argc, const char **argv, int ntags, const char **names) {
  unsigned char v=1; const char *arg;
  if (names[0][0] == '\0') ntags=tst_tags_zero(); 
  if (argc > 1 && argv[1][0] == '?') {
    fprintf(stderr,"Test Scenario: \"%s\"\n%s %s",tst_title, argv[0], ntags>0? "[? | [=] [+/-]tag ... ]\ntags: " : "[? | =]");
    for (int k=0; k<ntags; k++) fprintf(stderr,"%s ",names[k]);
    fputc('\n',stderr);
    exit(1);
  }
  if (ntags > 0) {
    for (int n=1; n<argc; n++) {
      arg = argv[n];
      v = 0xFF;
      if (*arg == '-') {arg++; v=0x00;}
      if (*arg == '+') {arg++;}
      
      if (*arg == '*') {tst_tags_val = v; continue;}
      if (*arg == '\0') continue;
  
      for (int k=0; k<ntags; k++) {
        if (strcmp(arg,names[k])==0) {
          if (v) tst_tags_val |=  (1<<k);
          else   tst_tags_val &= ~(1<<k);
        }
      }
    }
  }
  // Return 1 if errors are to be reported as program failure
  return (!((argc > 1) && (argv[1][0] == '=')));
}

#define tst(x) (tst_result = !!(x))

static inline int tstfailed() {return !tst_result;}
static inline int tstpassed() {return  tst_result;}

// This is only used to avoid that the compiler complaining about unused static variables.
#define tst_usestatic (tst_result | tst_case_pass | tst_case_fail | tst_case_skip)
#define tst_init_case() (tst_case_pass=tst_case_fail=tst_case_skip=0)

#ifndef TST_STR_COMPACT
  #define TST_STR_PASS     "PASS│ "
  #define TST_STR_FAIL     "FAIL├┬"
  #define TST_STR_FAIL_2ND "    │╰ "
  #define TST_STR_SKIP     "SKIP├┬ "
  #define TST_STR_SKIP_2ND "    │╰ "
  #define TST_STR_CASE     "CASE┬── "
  #define TST_STR_CASE_END "    ╰── %d KO | %d OK | %d SKIP"
  #define TST_STR_FILE     "FILE ▷"
  #define TST_STR_FILE_END "RSLT ▷ %d KO | %d OK | %d SKIP"
  #define TST_STR_CLCK     "CLCK⚑  %f ms. "
  #define TST_STR_DATA     "DATA ▽▽▽ "
  #define TST_STR_DATA_END "\nDATA △△△" TST_STR_LINE
  #define TST_STR_NOTE     "NOTE: "
  #define TST_STR_GRUP     "GRUP┼── " 
  #define TST_STR_SCTN     "SCTN┼── "
  #define TST_STR_LINE     " :%d\n"
#else
  #define TST_STR_PASS     "P"
  #define TST_STR_FAIL     "F"
  #define TST_STR_FAIL_2ND "f "
  #define TST_STR_SKIP     "S "
  #define TST_STR_SKIP_2ND "s "
  #define TST_STR_CASE     "C "
  #define TST_STR_CASE_END "c %d %d %d"
  #define TST_STR_FILE     "R"
  #define TST_STR_FILE_END "r %d %d %d"
  #define TST_STR_CLCK     "T %f "
  #define TST_STR_DATA     "D "
  #define TST_STR_DATA_END "\nd " TST_STR_LINE
  #define TST_STR_NOTE     "N "
  #define TST_STR_GRUP     "G " 
  #define TST_STR_SCTN     "N "
  #define TST_STR_LINE     ":\xF%d\n"
#endif

#define tst_prtf(...) \
   (fprintf(stderr, __VA_ARGS__), fprintf(stderr, TST_STR_LINE , __LINE__), tst_zero=0)

#define tstcheck(tst_,...)  \
   do { tst(tst_); \
        tst_prtf("%s %s", tst_result ? (tst_pass++, tst_case_pass++,TST_STR_PASS) \
                                     : (tst_fail++, tst_case_fail++,TST_STR_FAIL), #tst_); \
        if (!tst_result) {fprintf(stderr,TST_STR_FAIL_2ND __VA_ARGS__); fputc('\n',stderr);} \
   } while(0)

// Duplicated to avoid double expansion of the `tst_` argument
#define tstassert(tst_,...)  \
   do { tst(tst_); \
        tst_prtf("%s %s", tst_result ? (tst_pass++, tst_case_pass++,TST_STR_PASS) \
                                     : (tst_fail++, tst_case_fail++,TST_STR_FAIL), #tst_); \
        if (!tst_result) {fprintf(stderr,TST_STR_FAIL_2ND __VA_ARGS__); fputc('\n',stderr); abort();} \
   } while(0)

#define tstrun_(tst_, title_,...) \
  tst_tags(0,__VA_ARGS__); void tst__run(int n); \
  int main(int argc, const char **argv) { \
    tst_title = title_; \
    int report_err = 1; \
    report_err = tst_parsetags(argc,argv); \
    fprintf(stderr,TST_STR_FILE " %s \"%s\"%s\n", __FILE__, tst_title, (tst_?"":" (disabled)"));\
    if (tst_) tst__run(tst_usestatic); \
    fprintf(stderr,TST_STR_FILE_END "\n", tst_fail, tst_pass, tst_skip);\
    return ((tst_fail > 0) * report_err); \
  } void tst__run(int n) 

#define tstrun(title_,...)  tstrun_((!tst_zero), title_, __VA_ARGS__)
#define tst_run(title_,...) tstrun_(( tst_zero), title_, __VA_ARGS__)

#define tstcase(...) \
   for (int tst_ = (tst_prtf(TST_STR_CASE __VA_ARGS__),tst_init_case());  \
        tst_ == 0; \
        tst_ = 1, fprintf(stderr,TST_STR_CASE_END "\n", tst_case_fail, tst_case_pass, tst_case_skip)) 

#define tstif(tst_,...) \
   if (!(tst_) && (tst_prtf(TST_STR_SKIP #tst_), fprintf(stderr,TST_STR_SKIP_2ND __VA_ARGS__), fputc('\n',stderr), ++tst_skip, ++tst_case_skip)) ; \
   else

#define tstclock(...) \
   for(clock_t clk = clock(); \
       clk; \
       clk = clock()-clk,fprintf(stderr, TST_STR_CLCK, ((double)clk)/((double)(CLOCKS_PER_SEC/1000))), clk=tst_prtf(__VA_ARGS__))

#define tstdata(...) \
   for(int tst_ = (fflush(stdout) , tst_prtf(TST_STR_DATA __VA_ARGS__)); \
       tst_ == 0; \
       tst_ = 1, fflush(stdout), fprintf(stderr,TST_STR_DATA_END ,__LINE__))

#define tstnote(...) (tst_prtf(TST_STR_NOTE __VA_ARGS__))

#define tstgroup(...)    if (tst_prtf(TST_STR_GRUP __VA_ARGS__)) ; \
                         else for ( short tst_vars[2] = {0,-2} ; (tst_vars[1] == -2) && (tst_vars[1] = -1) ; tst_vars[0] += 1) 

#define tstsection(...)    if (!((tst_vars[1] != -2) && (++tst_vars[1] == tst_vars[0]) && (tst_vars[1] = -2) && !tst_prtf(TST_STR_SCTN __VA_ARGS__))) ;\
                           else 

#define tst_check(...)
#define tst_assert(...)
#define tst_note(...)
#define tst_data(...)     if (!tst_zero) ; else
#define tst_case(...)     if (!tst_zero) ; else
#define tst_if(...)       if (!tst_zero) ; else
#define tst_section(...)  if (!tst_zero) ; else
#define tst_group(...)    if (!tst_zero) ; else
#define tst_clock(...)    if ( tst_zero) ; else

#ifdef __cplusplus
}
#endif

#endif // TST_VERSION
