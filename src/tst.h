//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#ifndef TST_VERSION // 0.1.1-beta
#define TST_VERSION    0x0001001B

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

#define tst__cnt(_1,_2,_3,_4,_5,_6,_7,_8,_N, ...) _N
#define tst__argn(...)  tst__cnt(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define tst__cat2(x,y)  x ## y
#define tst__cat(x,y)   tst__cat2(x,y)

#define tst_vrg(tst__f,...) tst__cat(tst__f, tst__argn(__VA_ARGS__))(__VA_ARGS__)

#define tsttags(...) tst_vrg(tst_tags_,__VA_ARGS__)
#define tst_tags_1(_1)                      tst_tags__(1,_1,_2,_3,_4,_5,_6,_7,_8)
#define tst_tags_2(_1,_2)                   tst_tags__(2,_1,_2,_3,_4,_5,_6,_7,_8)
#define tst_tags_3(_1,_2,_3)                tst_tags__(3,_1,_2,_3,_4,_5,_6,_7,_8)
#define tst_tags_4(_1,_2,_3,_4)             tst_tags__(4,_1,_2,_3,_4,_5,_6,_7,_8) 
#define tst_tags_5(_1,_2,_3,_4,_5)          tst_tags__(5,_1,_2,_3,_4,_5,_6,_7,_8) 
#define tst_tags_6(_1,_2,_3,_4,_5,_6)       tst_tags__(6,_1,_2,_3,_4,_5,_6,_7,_8) 
#define tst_tags_7(_1,_2,_3,_4,_5,_6,_7)    tst_tags__(7,_1,_2,_3,_4,_5,_6,_7,_8) 
#define tst_tags_8(_1,_2,_3,_4,_5,_6,_7,_8) tst_tags__(8,_1,_2,_3,_4,_5,_6,_7,_8) 

#define tst_tags__(n_,_1,_2,_3,_4,_5,_6,_7,_8) \
   static int tst_tag_##_1=1; static int tst_tag_##_2=1; static int tst_tag_##_3=1; static int tst_tag_##_4=1; \
   static int tst_tag_##_5=1; static int tst_tag_##_6=1; static int tst_tag_##_7=1; static int tst_tag_##_8=1; \
   static int  *tst_tag_states[8] = {&tst_tag_##_1,&tst_tag_##_2,&tst_tag_##_3,&tst_tag_##_4, \
                                     &tst_tag_##_5,&tst_tag_##_6,&tst_tag_##_7,&tst_tag_##_8}; \
   static char *tst_tag_names[8]  = {#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8}; \
   static inline void tstsettags(int argc, char *argv[]) {tst_set_tags(argc,argv, n_, tst_tag_states, tst_tag_names);}

static inline void tst_set_tags(int argc, char *argv[], int ntags, int*states[], char *names[]) {
  int v=1; char *arg;
  for (int n=1; n<argc; n++) {
    arg = argv[n]; v = 1;
    if (*arg == '?') {
      fprintf(stderr,"%s [? | [+/-]tag ...]\ntags: ",argv[0]);
      for (int k=0; k<ntags; k++) fprintf(stderr,"%s ",names[k]);
      fputc('\n',stderr);
      exit(1);
    }
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
  static inline char *tst_filename(const char *fname) 
  {  char *ret = (char *)fname;
     char *s ;
     if ((s = strrchr(ret,'/')) || (s = strrchr(ret, '\\'))) ret = s+1;
     return ret;
  }
#endif

#define tst(x) (tst_result = !!(x))

static inline int tstfailed(char *s) {return !tst_result;}
static inline int tstpassed(char *s) {return  tst_result;}

#define tsttag(t_) tst_tag_ ## t_

#define tst_usestatic ((((void *)tst_set_tags == NULL) * tst_result * tst_case * tst_zero) == 0)

#define tst_init_case() (tst_case_pass=tst_case_fail=tst_case_skip=0)
#define tst_init_run()  (tst_pass=tst_fail=tst_skip=tst_init_case())

#define tst_prtf(...) \
   (fprintf(stderr, __VA_ARGS__), fprintf(stderr, " » %s:%d\n", tst_filename(__FILE__), __LINE__), tst_zero=0)

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

#define tstrun(...) \
   for (int tst = !(fprintf(stderr,"FILE ▷ %s", tst_filename(__FILE__)), fprintf(stderr," " __VA_ARGS__ ), fputc('\n',stderr), tst_init_run()); \
        tst && tst_usestatic; \
        tst = 0, fprintf(stderr,"RSLT ▷ %d KO | %d OK | %d SKIP\n", tst_fail, tst_pass, tst_skip))

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
