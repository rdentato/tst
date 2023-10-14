//  (C) by Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#ifndef TST_VERSION // 0.1.0-beta
#define TST_VERSION    0x0001000B

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
static int tst_error = 0;

#define tst__cnt(_1,_2,_3,_4,_5,_6,_7,_8,_N, ...) _N
#define tst__argn(...)  tst__cnt(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define tst__cat0(x,y)  x ## y
#define tst__cat(x,y)   tst__cat0(x,y)

#define tst_vrg(tst__f,...) tst__cat(tst__f, tst__argn(__VA_ARGS__))(__VA_ARGS__)

static int   tst_tag     = 1;
static char *tst_tag_str = "";

#define tsttags(...)          tst_vrg(tst_tags_,__VA_ARGS__)
#define tst_tags_1(_1) \
   static int tsttag_##_1=1; \
   static inline void tstsettags(int argc, char *argv[]) { \
     tst_set_tags(argc,argv,&tsttag_##_1,#_1,&tst_tag,tst_tag_str,&tst_tag,tst_tag_str,&tst_tag,tst_tag_str); \
   }

#define tst_tags_2(_1,_2) \
   static int tsttag_##_1=1; static int tsttag_##_2=1; \
   static inline void tstsettags(int argc, char *argv[]) { \
     tst_set_tags(argc,argv,&tsttag_##_1,#_1,&tsttag_##_2,#_2,&tst_tag,tst_tag_str,&tst_tag,tst_tag_str); \
   }

#define tst_tags_3(_1,_2,_3) \
   static int tsttag_##_1=1; static int tsttag_##_2=1; static int tsttag_##_3=1; \
   static inline void tstsettags(int argc, char *argv[]) { \
     tst_set_tags(argc,argv,&tsttag_##_1,#_1,&tsttag_##_2,#_2,&tsttag_##_3,#_3,&tst_tag,tst_tag_str); \
   }

#define tst_tags_4(_1,_2,_3,_4) \
   static int tsttag_##_1=1; static int tsttag_##_2=1; static int tsttag_##_3=1; static int tsttag_##_4=1; \
   static inline void tstsettags(int argc, char *argv[]) { \
   tst_set_tags(argc,argv,&tsttag_##_1,#_1,&tsttag_##_2,#_2,&tsttag_##_3,#_3,&tsttag_##_4,#_4);}

static inline void tst_set_tags(int argc, char *argv[],int *_1,char *_1_str,int *_2,char *_2_str,int *_3,char *_3_str,int *_4,char *_4_str) {
  int v=1; char *arg;
  for (int n=1; n<argc; n++) {
    arg = argv[n]; v = 1;
    if (*arg == '?') {
      fprintf(stderr,"%s [? | [+/-]tag ...]\n",argv[0]);
      fprintf(stderr,"tags: %s %s %s %s\n",_1_str,_2_str,_3_str,_4_str);
      exit(1);
    }
    if (*arg == '-') {v=0; arg++;}
    if (*arg == '+') {arg++;}
    if (*arg == '\0') /* DO NOTHING */;
    else if (*arg == '*') {*_1= *_2= *_3= *_4=v; }
    else if (strcmp(arg,_1_str)==0) *_1=v;
    else if (strcmp(arg,_2_str)==0) *_2=v;
    else if (strcmp(arg,_3_str)==0) *_3=v;
    else if (strcmp(arg,_4_str)==0) *_4=v;
  }
}

#define tsttag(t_) tsttag_ ## t_

#define tst_usestatic ((((void *)tst_set_tags == (void *)tst_tag_str) * tst_tag * tst_error * tst_case * tst_zero) == 0)

#define tst_init_case() (tst_case_pass=tst_case_fail=tst_case_skip=0)
#define tst_init_run()  (tst_pass=tst_fail=tst_skip=tst_init_case())

#define tst_prtf(...) \
   (fprintf(stderr, __VA_ARGS__), fprintf(stderr, " » %s:%d\n", __FILE__, __LINE__), tst_zero=0)

#define tstcheck(tst_,...)  \
   do { tst_error = !(tst_); \
        tst_prtf("%s %s", tst_error ? (tst_fail++, tst_case_fail++,"FAIL├┬") \
                                    : (tst_pass++, tst_case_pass++,"PASS│ "), #tst_); \
        if (tst_error) {fprintf(stderr,"    │╰ " __VA_ARGS__); fputc('\n',stderr);} \
   } while(0)

#define tstassert(...) \
   do { tstcheck(__VA_ARGS__); if (tst_error) abort(); } while(0)

#define tstrun(...) \
   for (int tst=!(fputs ("FILE ▷ " __FILE__, stderr), fprintf(stderr," " __VA_ARGS__ ), fputc('\n',stderr), tst_init_run()); \
        tst && tst_usestatic; \
        tst=0, fprintf(stderr,"RSLT ▷ %d KO | %d OK | %d SKIP\n", tst_fail, tst_pass, tst_skip))

#define tstcase(...) \
   for (tst_case=!(tst_prtf("CASE┬── " __VA_ARGS__),tst_init_case());  \
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

#endif // TST_VERSION
