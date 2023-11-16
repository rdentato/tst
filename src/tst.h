//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT
//  SPDX-PackageVersion: 0.5.4-rc

#ifndef TST_VERSION
#define TST_VERSION 0x0005004C

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

static volatile short tst_zero = 0;
static short tst_result = 0;
static short tst_color = 1;
static short tst_report_err = 1;

static int tst_pass = 0;
static int tst_fail = 0;
static int tst_skip = 0;
static const char* tst_title = NULL;

static const char *tst_str_red    = "\0\033[1;31m";
static const char *tst_str_green  = "\0\033[0;32m";
static const char *tst_str_yellow = "\0\033[0;33m";
static const char *tst_str_normal = "\0\033[0m";

#define tst_str_skip      "SKIP|  "
#define tst_str_fail      "FAIL|  "
#define tst_str_pass      "PASS|  "
#define tst_str_skip_tst  "SKPT|,-(%s)"
#define tst_str_skip_end  "    |`---"
#define tst_str_case      "CASE,--"
#define tst_str_case_end  "    `--- "
#define tst_str_file      "----- FILE >"
#define tst_str_file_end  "^^^^^ RSLT > "
#define tst_str_file_abr  "^^^^^ ABRT > "
#define tst_str_clck      "CLCK:  %ld %ss "
#define tst_str_note      "NOTE:"
#define tst_str_sctn      "SCTN|,--"
#define tst_str_sctn_end  "    |`---"
#define tst_str_scrn      "<<<<< "
#define tst_str_scrn_end  ">>>>>\n"

#define tstprintf(...) fprintf(stderr,__VA_ARGS__)
#define tst_prtf(...) (fprintf(stderr, __VA_ARGS__), tst_zero &= (short)fputc('\n',stderr))
#define tst_prtln(s)  fprintf(stderr, "%5d %s" , __LINE__, s)

static int tst_prt_results(int fail, int pass, int skip) {
   fprintf(stderr,"%s%d FAIL%s | ",tst_color+tst_str_red   , fail, tst_str_normal+tst_color);
   fprintf(stderr,"%s%d PASS%s | ",tst_color+tst_str_green , pass, tst_str_normal+tst_color);
   fprintf(stderr,"%s%d SKIP%s"   ,tst_color+tst_str_yellow, skip, tst_str_normal+tst_color);
   return 0;
} 

// The macros `exp1`, `exp2` and `cat2` have been introduced to support Micorsoft `cl` compiler
#define tst__count(_1,_2,_3,_4,_5,_6,_7,_8,_9,_A,_B,_C,_D,_E,_F,_N, ...) _N
#define tst__argn(...)  tst__count tst__exp1((__VA_ARGS__, F, E, D, C, B, A, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#define tst__exp2(x,y)  x y
#define tst__exp1(x)    x
#define tst__cat0(x,y)  x ## y
#define tst__cat1(x,y)  tst__cat0(x,y)
#define tst__cat2(x,y)  tst__cat1(x,y)
#define tst_vrg(_f,...) tst__exp2(tst__cat2(_f, tst__argn(__VA_ARGS__)),tst__exp1((__VA_ARGS__)))

#define tst_tags(...)                          tst_vrg(tst_tags_,__VA_ARGS__)
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
#define tsttag_2(t_,x_) ((x_) ? (tst_tags_val |= (tst_tag_ ## t_)), !(tst_zero = 0) \
                              : (tst_tags_val &= ~(tst_tag_ ## t_)), (tst_zero = 0))

#define tst_tags__(n_,_1,_2,_3,_4,_5,_6,_7,_8) \
   static unsigned char tst_tag_##_1=0x01; static unsigned char tst_tag_##_2=0x02; \
   static unsigned char tst_tag_##_3=0x04; static unsigned char tst_tag_##_4=0x08; \
   static unsigned char tst_tag_##_5=0x10; static unsigned char tst_tag_##_6=0x20; \
   static unsigned char tst_tag_##_7=0x40; static unsigned char tst_tag_##_8=0x80; \
   static const char *tst_tag_names[8] = {#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8}; \
   static inline int tst_parsetags(int argc, const char **argv) {return tst_parse_tags(argc,argv, n_, tst_tag_names);}\
   static inline int tst_tags_zero(void) { return tst_zero & (tst_tag_##_1 | tst_tag_##_2 | tst_tag_##_3| tst_tag_##_4| \
                                                              tst_tag_##_5 | tst_tag_##_6 | tst_tag_##_7| tst_tag_##_8); }

static inline int tst_tags_zero(); // tst_tags_zero() always returns 0 and is used just to avoid compiler warnings.

#define TST_STR_HELP_NOTAGS "[--help] [--color-off] [--report-error] [--list]"
#define TST_STR_HELP_TAGS   " [+/-]tag ... ]\ntags:" 

static inline short tst_parse_tags(int argc, const char **argv, int ntags, const char **names) {
  unsigned char v;
  const char *arg;
  short report_error = 0;
  if (names[0][0] == '\0') ntags=tst_tags_zero(); 
  for (int n=0; n<argc; n++) {
    arg = argv[n];
    v = 0xFF;
    if ((arg[0] == '-') && (arg[1] == '-')) {
      switch (arg[2]) {
        case 'r': report_error = 1; break;
        case 'c': tst_color = 0; break;
        case 'h': fprintf(stderr,"Test suite: \"%s\"\n%s %s", tst_title, argv[0], TST_STR_HELP_NOTAGS);
                  if (ntags>0) fputs(TST_STR_HELP_TAGS,stderr);
                  goto prttags;
        case 'l': fprintf(stderr,"%s \"%s\"", argv[0], tst_title);
         prttags: for (int k=0; k<ntags; k++) fprintf(stderr," %s",names[k]);
                  fputc('\n',stderr);
                  exit(0);
      }
      continue;
    }
    if (*arg == '-') {arg++; v=0x00;}
    if (*arg == '+') {arg++; v=0xFF;}
    if (*arg == '*') {tst_tags_val = v; continue;}
    if (*arg == '\0') continue;
  
    for (int k=0; k<ntags; k++) {
      if (strcmp(arg,names[k])==0) {
        if (v) tst_tags_val |= (unsigned char)(1<<k);
        else   tst_tags_val &= (unsigned char)(~(1<<k));
      }
    }
  }
  // Return 1 if errors are to be reported as program failure
  return (report_error);
}

static inline char *tst_time(void)
{
  time_t cur_tm;    
  struct tm *tm; 
  static char tstr[32];

  time(&cur_tm); tm=localtime(&cur_tm);
  strftime(tstr,32,"%Y-%m-%d %H:%M:%S",tm);
  return tstr;
}

#define tstrun_(tst_, title_,...) \
  tst_tags(0,__VA_ARGS__); void tst__run(int n); \
  int main(int argc, char **argv) { \
    tst_title = title_; \
    tst_report_err = (short)tst_parsetags(argc,(const char **)argv); \
    if (CLOCKS_PER_SEC > ((clock_t)1000000) + tst_zero) tst_clock_unit = "n"; \
    else if(CLOCKS_PER_SEC > ((clock_t)1000) + tst_zero) tst_clock_unit = "u"; \
    else tst_clock_unit = "m"; \
    fprintf(stderr, "%s %s \"%s\" %s%s\n", tst_str_file, __FILE__, tst_title, tst_time(), (tst_?"":" (disabled)"));\
    if (tst_) tst__run(tst_usestatic); \
    fputs(tst_str_file_end,stderr); tst_prt_results(tst_fail, tst_pass, tst_skip); fprintf(stderr," %s\n",tst_time());\
    return ((tst_fail > 0) * tst_report_err); \
  } void tst__run(int tst_n) 

#define tstrun(title_,...)  tstrun_((!tst_zero), title_, __VA_ARGS__)
#define tst_run(title_,...) tstrun_(( tst_zero), title_, __VA_ARGS__)

static short tst_vars[6] = {0}; // Ensures that `tstcheck` can be used outside a `tstcase` block.

// This is only used to avoid that the compiler complains about unused static variables.
#define tst_usestatic ((  tst_result & tst_case_pass & tst_case_fail & tst_case_skip \
                        & tst_vars[0] & tstdata[0] & (int)tstelapsed))

#define tst(x) (tst_result = (short)(!!(x)))

static inline int tstfailed(void)  {return !tst_result;}
static inline int tstpassed(void)  {return  tst_result;}
static inline int tstskipped(void) {return (tst_result < 0);}

#define tstcheck(t_,...) \
  do { const char* tst_s = #t_;  \
    tst_result = (short)(tst_skip_test? -1 : !!(t_)); \
    switch (tst_result) { \
      case -1: tst_skip++; tst_case_skip++; tst_prtln(tst_str_skip); fputs(tst_color+tst_str_yellow, stderr); break; \
      case  0: tst_fail++; tst_case_fail++; tst_prtln(tst_str_fail); fputs(tst_color+tst_str_red   , stderr); break; \
      case  1: tst_pass++; tst_case_pass++; tst_prtln(tst_str_pass); fputs(tst_color+tst_str_green , stderr); break; \
    } \
    fprintf(stderr, "%s%s", tst_s, tst_str_normal+tst_color); \
    if (tst_result == 0) { fprintf(stderr," \"" __VA_ARGS__); fputc('"',stderr); } \
    fputc('\n', stderr); \
  } while(0);

#define tstassert(...) \
  do { tstcheck(__VA_ARGS__); \
    if (tst_result == 0) { \
      fputs(tst_str_file_abr,stderr); tst_prt_results(tst_fail, tst_pass, tst_skip); fprintf(stderr," %s\n",tst_time()); \
      exit(0);\
    } \
  } while(0)

#define tst_skip_test tst_vars[5]
#define tstskipif(tst_) \
  for (int tst_k = 1 ; \
       tst_k &&  ((tst_skip_test = (short)(!!(tst_))),1); \
       tst_k = (tst_skip_test = (short)(tst_skip_test? (tst_prtln(""), tst_prtf("%s",tst_str_skip_end)):0))) \
    if (tst_skip_test && (tst_prtln(""), tst_prtf(tst_str_skip_tst,#tst_))) ; else

static const char *tst_clock_unit;
static clock_t tstelapsed = 0;
#define tstelapsed() tstelapsed

#define tstclock(...) \
  for(clock_t tst_clk = clock(); \
      tst_clk; \
      tstelapsed=(clock()-tst_clk), \
        tst_prtln(""), fprintf(stderr, tst_str_clck, tstelapsed, tst_clock_unit), tst_clk=tst_prtf(__VA_ARGS__))

#define tstnote(...) (tst_prtln(tst_str_note), tst_prtf( " " __VA_ARGS__))

#define tstouterr(...) for (int tst_k = (tst_prtln(tst_str_scrn),tst_prtf(" " __VA_ARGS__ ),1); \
                         tst_k; \
                         tst_k = 0,fputc('\n',stderr),tst_prtln(tst_str_scrn_end) )

#define tst_sect_iterator  tst_vars[0]
#define tst_sect_counter   tst_vars[1]
#define tst_sect_not_last  -2
#define tst_sect_last      -3

#define tst_case_pass tst_vars[2]
#define tst_case_fail tst_vars[3]
#define tst_case_skip tst_vars[4]

#define tstcase(...) \
   if (tst_prtln(tst_str_case), tst_prtf(" " __VA_ARGS__)) ; \
   else for (short tst_vars[6] = {0, tst_sect_not_last, 0, 0, 0, 0}; \
             ((tst_sect_counter == tst_sect_not_last) && (tst_sect_counter = -1)) || \
                        (tst_prtln(tst_str_case_end), tst_prt_results(tst_case_fail, tst_case_pass, tst_case_skip), tst_zero &= (short)fputc('\n',stderr));\
             tst_sect_iterator += 1)

static volatile unsigned short tstdata[1]={0};

#define tstcurdata tstdata[tst_data_count]
#define tst_data_size ((int)(sizeof(tstdata)/sizeof(tstdata[0])))

#define tstsection(...) \
              for (int tst_sect = 1; \
                tst_sect && ((tst_sect_counter > tst_sect_not_last) || !(tst_sect_counter = tst_sect_not_last))\
                         && (++tst_sect_counter == tst_sect_iterator) \
                         && !(tst_prtln(tst_str_sctn), tst_prtf(" " __VA_ARGS__)); \
                tst_sect = 0, tst_sect_counter = tst_sect_last, tst_prtln(tst_str_sctn_end), fputc('\n',stderr)) \
                for (int tst_data_count = 0; tst_data_count < tst_data_size; tst_data_count++) 

#define tst_check(...)
#define tst_assert(...)
#define tst_note(...)
#define tst_skpif(...)    if ( tst_zero) ; else
#define tst_clock(...)    if ( tst_zero) ; else
#define tst_case(...)     if (!tst_zero) ; else
#define tst_section(...)  if (!tst_zero) ; else

#ifdef __cplusplus
}
#endif

#endif // TST_VERSION
