//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT
//  SPDX-PackageVersion: 0.2.0 release candidate

#ifndef TST_VERSION
#define TST_VERSION 0x0002000C

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

static volatile int tst_zero = 0;
static int tst_case_pass = 0;
static int tst_case_fail = 0;
static int tst_case_skip = 0;
static int tst_pass = 0;
static int tst_fail = 0;
static int tst_skip = 0;
static int tst_case = 0;
static int tst_result = 0;
static const char* tst_title;

#define tst__cnt(_1,_2,_3,_4,_5,_6,_7,_8,_9,_N, ...) _N
#define tst__argn(...)  tst__cnt(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define tst__cat2(x,y)  x ## y
#define tst__cat(x,y)   tst__cat2(x,y)
#define tst_vrg(tst__f,...) tst__cat(tst__f, tst__argn(__VA_ARGS__))(__VA_ARGS__)

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

#define tsttag(t_) tst_tag_ ## t_

#define tst_tags__(n_,_1,_2,_3,_4,_5,_6,_7,_8) \
   static int tst_tag_##_1=1; static int tst_tag_##_2=1; static int tst_tag_##_3=1; static int tst_tag_##_4=1; \
   static int tst_tag_##_5=1; static int tst_tag_##_6=1; static int tst_tag_##_7=1; static int tst_tag_##_8=1; \
   static int  *tst_tag_states[8] = {&tst_tag_##_1,&tst_tag_##_2,&tst_tag_##_3,&tst_tag_##_4, \
                                     &tst_tag_##_5,&tst_tag_##_6,&tst_tag_##_7,&tst_tag_##_8}; \
   static const char *tst_tag_names[8]  = {#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8}; \
   static inline void tst_parsetags(int argc, const char **argv) {tst_parse_tags(argc,argv, n_, tst_tag_states, tst_tag_names);}

static inline void tst_parsetags(int argc, const char **argv);

static inline void tst_parse_tags(int argc, const char **argv, int ntags, int*states[], const char **names) {
  int v=1; const char *arg;
  if (names[0][0] == '\0') ntags=0;
  for (int n=1; n<argc; n++) {
    arg = argv[n];
    if (*arg == '?') {
      fprintf(stderr,"Test Scenario: \"%s\"\n%s %s",tst_title, argv[0], ntags>0? "[? | [+/-]tag ...]\ntags: " : "[?]");
      for (int k=0; k<ntags; k++) fprintf(stderr,"%s ",names[k]);
      fputc('\n',stderr);
      exit(1);
    }
    v = 1;
    if (*arg == '-') {v=0; arg++;}
    if (*arg == '+') {arg++;}
    if (*arg == '\0') /* DO NOTHING */ ;
    else if (*arg == '*') {
      for (int k=0; k<ntags; k++) *(states[k])=v; 
    }
    else for (int k=0; k<ntags; k++)
      if (strcmp(arg,names[k])==0) *(states[k])=v;
  }
}

#ifdef TSTFULLPATH
  #define tst_filename(f) f
#else
  static inline char *tst_filename(char *fname) {
    char *s;
    if ((s = strrchr(fname,'/')) || (s = strrchr(fname, '\\'))) return s+1;
    return fname;
  }
#endif

#define tst(x) (tst_result = !!(x))

static inline int tstfailed(char *s) {return !tst_result;}
static inline int tstpassed(char *s) {return  tst_result;}

// This is only used to avoid that the compiler could complain about unused static variables.
#define tst_usestatic (tst_result | tst_case | tst_case_pass | tst_case_fail | tst_case_skip)

#define tst_init_case() (tst_case_pass=tst_case_fail=tst_case_skip=0)

#define tst_prtf(...) \
   (fprintf(stderr, __VA_ARGS__), fprintf(stderr, " » %s:%d\n", tst_filename((char *)__FILE__), __LINE__), tst_zero=0)

#define tstcheck(tst_,...)  \
   do { tst(tst_); \
        tst_prtf("%s %s", tst_result ? (tst_pass++, tst_case_pass++,"PASS│ ") \
                                     : (tst_fail++, tst_case_fail++,"FAIL├┬"), #tst_); \
        if (!tst_result) {fprintf(stderr,"    │╰ " __VA_ARGS__); fputc('\n',stderr);} \
   } while(0)

// Duplicated to avoid double expansion of the `tst_` argument
#define tstassert(tst_,...)  \
   do { tst(tst_); \
        tst_prtf("%s %s", tst_result ? (tst_pass++, tst_case_pass++,"PASS│ ") \
                                     : (tst_fail++, tst_case_fail++,"FAIL├┬"), #tst_); \
        if (!tst_result) {fprintf(stderr,"    │╰ " __VA_ARGS__); fputc('\n',stderr); abort();} \
   } while(0)

#define tstrun_(tst_, title_,...) \
  tst_tags(0,__VA_ARGS__); void tst__run(int n); \
  int main(int argc, const char **argv) { \
    tst_title = title_; \
    tst_parsetags(argc,argv); \
    fprintf(stderr,"FILE ▷ %s \"%s%s\"\n", tst_filename((char *)__FILE__), tst_title, (tst_?"":" (disabled)"));\
    if (tst_) tst__run(tst_usestatic); \
    fprintf(stderr,"RSLT ▷ %d KO | %d OK | %d SKIP\n", tst_fail, tst_pass, tst_skip);\
    return (tst_fail > 0); \
  } void tst__run(int n) 

#define tstrun(title_,...)  tstrun_((!tst_zero), title_, __VA_ARGS__)
#define tst_run(title_,...) tstrun_(( tst_zero), title_, __VA_ARGS__)

#define tstcase(...) \
   for (tst_case = !(tst_prtf("CASE┬── " __VA_ARGS__),tst_init_case());  \
        tst_case; \
        fprintf(stderr,"    ╰── %d KO | %d OK | %d SKIP\n", tst_case_fail, tst_case_pass, tst_case_skip), tst_case = 0) 

#define tstgroup(tst_,...) \
   if (!(tst_) && (tst_prtf("SKIP├┬ " #tst_), fprintf(stderr,"    │╰ " __VA_ARGS__), fputc('\n',stderr), ++tst_skip, ++tst_case_skip)) ; \
   else

#define tstclock(...) \
   for(clock_t clk=clock(); \
       clk; \
       clk=clock()-clk,fprintf(stderr,"CLCK│⚑ %f ms. ",((double)clk)/((double)CLOCKS_PER_SEC/1000.0)), clk=tst_prtf(__VA_ARGS__))

#define tstdata(...) \
   for(int tst = !(fflush(stdout) , tst_prtf("DATA│ ▽▽▽ " __VA_ARGS__)); \
       tst; \
       tst=0, fflush(stdout), fputs("\nDATA│ △△△\n",stderr))

#define tstnote(...) (fprintf(stderr,"NOTE%s🗎",tst_case?"│":" "), tst_prtf(" " __VA_ARGS__))

#define tst_check(...)
#define tst_assert(...)
#define tst_note(...)
#define tst_data(...)  if (!tst_zero) ; else
#define tst_case(...)  if (!tst_zero) ; else
#define tst_group(...) if (!tst_zero) ; else
#define tst_clock(...) if ( tst_zero) ; else

#ifdef __cplusplus
}
#endif

#endif // TST_VERSION
