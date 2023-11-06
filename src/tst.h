//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT
//  SPDX-PackageVersion: 0.4.2-rc

#ifndef TST_VERSION
#define TST_VERSION 0x0004002C

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

static volatile short tst_zero = 0;
static          short tst_result = 0;
static int tst_pass = 0;
static int tst_fail = 0;
static int tst_skip = 0;
static const char* tst_title = NULL;

#ifndef TST_STR_COMPACT
  #define TST_STR_PASS      "PASS│  "
  #define TST_STR_FAIL_1    "FAIL│  "
  #define TST_STR_FAIL      "FAIL├┬ "
  #define TST_STR_FAIL_2ND  "    │╰"
  #define TST_STR_SKIP_CHK  "SKIP│  %s"
  #define TST_STR_CASE      "CASE┬──"
  #define TST_STR_CASE_END  "    ╰── %d KO | %d OK | %d SKIP"
  #define TST_STR_FILE      "FILE ▷"
  #define TST_STR_FILE_END  "RSLT ▷ %d KO | %d OK | %d SKIP"
  #define TST_STR_CLCK      "CLCK ⚑ %ld %ss "
  #define TST_STR_NOTE      "NOTE:"
  #define TST_STR_SCTN      "SCTN├┬─"
  #define TST_STR_SCTN_END  "    │└─"
#else
  #define TST_STR_PASS      "P "
  #define TST_STR_FAIL_1    "F "
  #define TST_STR_FAIL      "F "
  #define TST_STR_FAIL_2ND  "f"
  #define TST_STR_SKIP_CHK  "S %s"
  #define TST_STR_CASE      "{"
  #define TST_STR_CASE_END  "} %d %d %d"
  #define TST_STR_FILE      "R"
  #define TST_STR_FILE_END  "r %d %d %d"
  #define TST_STR_CLCK      "T %ld %ss "
  #define TST_STR_NOTE      "N"
  #define TST_STR_SCTN      "("
  #define TST_STR_SCTN_END  ")"
  #ifndef TST_STR_NOCOLOR
    #define TST_STR_NOCOLOR
  #endif
#endif

#ifndef TST_STR_NOCOLOR
#define TST_STR_GREEN  "\033[1;32m"
#define TST_STR_RED    "\033[1;31m"
#define TST_STR_NORMAL "\033[0m"
#endif

#define tst_prtf(...) (fprintf(stderr, __VA_ARGS__), tst_zero &= fputc('\n',stderr))
#define tst_prtln(s)  fprintf(stderr, "%5d %s" , __LINE__, s)

#define tst__cnt(_1,_2,_3,_4,_5,_6,_7,_8,_9,_N, ...) _N
#define tst__argn(...)  tst__cnt(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define tst__cat2(x,y)  x ## y
#define tst__cat(x,y)   tst__cat2(x,y)
#define tst_vrg(tst__f,...) tst__cat(tst__f, tst__argn(__VA_ARGS__))(__VA_ARGS__)

#define tst__arg1(...)  tst__cnt(__VA_ARGS__, _, _, _, _, _, _, _, _, 1, 0)
#define tst_vrg1(tst__f,...) tst__cat(tst__f, tst__arg1(__VA_ARGS__))(__VA_ARGS__)

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

static unsigned char tst_tags_val = 0x00; // All tags are "off" by default

#define tsttag(...) tst_vrg(tsttag_,__VA_ARGS__)
#define tsttag_1(t_) (!!((tst_tag_ ## t_) & tst_tags_val))
#define tsttag_2(t_,x_) ((x_)? (tst_tags_val |= (tst_tag_ ## t_)), !(tst_zero = 0) \
                             : (tst_tags_val &= ~(tst_tag_ ## t_)), tst_zero = 0)

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
      if (*arg == '+') {arg++; v=0xFF;}
      
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

static inline int tstfailed()  {return !tst_result;}
static inline int tstpassed()  {return  tst_result;}
static inline int tstskipped() {return  (tst_result < 0);}

#define tstcheck(t_,...) do { const char* tst_s = #t_;  \
                              if (tst_skip_test) {\
                                tst_result = -1; \
                                tst_prtln(""); tst_prtf(TST_STR_SKIP_CHK,tst_s);\
                                tst_skip++; tst_case_skip++; \
                              } \
                              else { \
                                tst_result = !!(t_); \
                                tst_vrg1(tst__check_,__VA_ARGS__); \
                              } \
                            } while(0);

#define tst__check_1(t_)  do { int tst_z = (#t_[0] != '\0'); tstcheck_(tst_z,t_) } while (0)
#define tst__check__(...) do { tstcheck_(1, __VA_ARGS__) } while(0)

#define tstcheck_(f_, ...)  \
        if (tst_result) { tst_pass++; tst_case_pass++; tst_prtln(TST_STR_PASS); fputs(TST_STR_GREEN,stderr);} \
        else {tst_fail++; tst_case_fail++; tst_prtln(f_ ? TST_STR_FAIL : TST_STR_FAIL_1); fputs(TST_STR_RED,stderr); } \
        tst_prtf("%s" TST_STR_NORMAL, tst_s); \
        if (f_ && !tst_result) { \
          fputs("      " TST_STR_FAIL_2ND, stderr); \
          fprintf(stderr," " __VA_ARGS__); fputc('\n',stderr); \
        }

#define tstassert(...) \
   do { tstcheck(__VA_ARGS__); \
     if (!tst_result) { \
       fprintf(stderr,"^^^^^ " TST_STR_FILE_END " (Aborted)\n", tst_fail, tst_pass, tst_skip); \
       abort();\
     } \
   } while(0)

// This is only used to avoid that the compiler complaining about unused static variables.
#define tst_usestatic ((  tst_result & tst_case_pass & tst_case_fail & tst_case_skip \
                        & tst_vars[0] & tstdata[0] & (int)tstelapsed))

#define tstrun_(tst_, title_,...) \
  tst_tags(0,__VA_ARGS__); void tst__run(int n); \
  int main(int argc, const char **argv) { \
    tst_title = title_; \
    int report_err = 1; \
    report_err = tst_parsetags(argc,argv); \
    if (CLOCKS_PER_SEC == ((clock_t)1000000000)) tst_clock_unit = "n"; \
    else if(CLOCKS_PER_SEC == ((clock_t)1000000)) tst_clock_unit = "µ"; \
    else if(CLOCKS_PER_SEC == ((clock_t)1000)) tst_clock_unit = "m"; \
    fprintf(stderr, "----- %s %s \"%s\"%s\n", TST_STR_FILE, __FILE__, tst_title, (tst_?"":" (disabled)"));\
    if (tst_) tst__run(tst_usestatic); \
    fprintf(stderr,"^^^^^ " TST_STR_FILE_END "\n", tst_fail, tst_pass, tst_skip); \
    return ((tst_fail > 0) * report_err); \
  } void tst__run(int tst_n) 

#define tstrun(title_,...)  tstrun_((!tst_zero), title_, __VA_ARGS__)
#define tst_run(title_,...) tstrun_(( tst_zero), title_, __VA_ARGS__)

static unsigned short tst_vars[6] = {0}; // Ensures that `tstcheck` can be used outside a `tstcase` block.

#define tst_skip_test tst_vars[5]

#define tstskipif(tst_) for (int tst_k = 1 ; tst_k &&  ((tst_skip_test = !!(tst_)),1); tst_k--, tst_skip_test = 0) \
                          if (tst_skip_test && tstnote("SKIP condition: `%s`",#tst_)) ; else

static const char *tst_clock_unit="?";
static clock_t tstelapsed = 0;
#define tstelapsed() tstelapsed

#define tstclock(...) \
   for(clock_t tst_clk = clock(); \
       tst_clk; \
       tstelapsed=(clock()-tst_clk), \
         tst_prtln(""), fprintf(stderr, TST_STR_CLCK, tstelapsed, tst_clock_unit), tst_clk=tst_prtf(__VA_ARGS__))

#define tstnote(...) (tst_prtln(TST_STR_NOTE), tst_prtf( " " __VA_ARGS__))

#define tst_sect_iterator  tst_vars[0]
#define tst_sect_counter   tst_vars[1]
#define tst_sect_not_last  -2
#define tst_sect_last      -3

#define tst_case_pass tst_vars[2]
#define tst_case_fail tst_vars[3]
#define tst_case_skip tst_vars[4]

#define tstcase(...) \
   if (tst_prtln(TST_STR_CASE), tst_prtf(" " __VA_ARGS__)) ; \
   else for (short tst_vars[6] = {0, tst_sect_not_last, 0, 0, 0, 0}; \
             ((tst_sect_counter == tst_sect_not_last) && (tst_sect_counter = -1)) || \
                       (tst_prtln(""), tst_prtf(TST_STR_CASE_END, tst_case_fail, tst_case_pass, tst_case_skip)); \
             tst_sect_iterator += 1)

static volatile unsigned short  tstdata[1]={0};
#define tstcurdata tstdata[tst_data_count]
#define tst_data_size ((int)(sizeof(tstdata)/sizeof(tstdata[0])))

#define tstsection(...) \
              for (int tst_sect = 1; \
                tst_sect && ((tst_sect_counter > tst_sect_not_last) || !(tst_sect_counter = tst_sect_not_last))\
                         && (++tst_sect_counter == tst_sect_iterator) \
                         && !(tst_prtln(TST_STR_SCTN), tst_prtf(" " __VA_ARGS__)); \
                tst_sect = 0, tst_sect_counter = tst_sect_last, tst_prtf("      %s",TST_STR_SCTN_END)) \
                for (int tst_data_count = 0; tst_data_count < tst_data_size; tst_data_count++) 

#define tst_check(...)
#define tst_assert(...)
#define tst_note(...)
#define tst_case(...)     if (!tst_zero) ; else
#define tst_if(...)       if (!tst_zero) ; else
#define tst_section(...)  if (!tst_zero) ; else
#define tst_clock(...)    if ( tst_zero) ; else

#ifdef __cplusplus
}
#endif

#endif // TST_VERSION
