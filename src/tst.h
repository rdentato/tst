//  SPDX-FileCopyrightText: © 2023 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT
//  SPDX-PackageVersion: 0.8.1-beta

#ifndef TST_VERSION
#define TST_VERSION 0x0008001B

#ifdef _MSC_VER
  /* Microsoft cl compiler */
  #pragma warning(disable:4100)
  #pragma warning(disable:4189)
  #pragma warning(disable:4152)
  #pragma warning(disable:4244)
  #pragma warning(disable:4459)
  #pragma warning(disable:4996)  
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

static volatile short tst_zero = 0;
static short tst_result     = 0;
static short tst_report_err = 0;

static int tst_pass = 0;
static int tst_fail = 0;
static int tst_skip = 0;
static const char* tst_title = NULL;
static int tst_case_ln = 0;
int tst_list_opt = 0;

const char *tst_str_skip      = "SKIP|  ";
const char *tst_str_fail      = "FAIL|  ";
const char *tst_str_pass      = "PASS|  ";
const char *tst_str_skip_tst  = "SKPT|,-(%s)";
const char *tst_str_skip_end  = "    |`---";
const char *tst_str_case      = "CASE,--";
const char *tst_str_case_end  = "    `--- ";
const char *tst_str_file      = "SUIT /";
const char *tst_str_file_end  = "^^^^^ RSLT \\ ";
const char *tst_str_file_abr  = "\n^^^^^ ABRT \\ ";
const char *tst_str_clck      = "CLCK:  %ld %ss ";
const char *tst_str_note      = "NOTE:";
const char *tst_str_sctn      = "SCTN|,--";
const char *tst_str_sctn_end  = "    |`---";
const char *tst_str_scrn      = "<<<<< ";
const char *tst_str_scrn_end  = ">>>>>\n";

#define tstprintf(...) fprintf(stderr,__VA_ARGS__)
#define tst_prtf(...) (fprintf(stderr, __VA_ARGS__), tst_zero &= (short)fputc('\n',stderr))
#define tst_prtln_n(n,s)  fprintf(stderr, "%5d %s" , (n)? tst_case_ln : __LINE__, s)
#define tst_prtln(s)  tst_prtln_n(0,s)

static int tst_prt_results(int fail, int pass, int skip) {
   fprintf(stderr,"%d FAIL | ", fail);
   fprintf(stderr,"%d PASS | ", pass);
   fprintf(stderr,"%d SKIP", skip);
   return 0;
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

static int tst_argc = 0;
static char** tst_argv = NULL;

// Check if test case should run based on command line tags
// Returns 1 if should run, 0 if should skip
// Rules:
//   - Untagged tests always run
//   - Tagged tests start as disabled
//   - CLI filters processed left-to-right, each overrides previous decision
//   - "+*" enables all tests with +TAG (but not -TAG)
//   - "+TAG" enables tests with +TAG, disables tests with -TAG
//   - "-TAG" disables tests with +TAG, enables tests with -TAG
//
// | Test Tags | No Filter | +*      | +TAG     | -TAG    | +*, -TAG | +*, +TAG   | -TAG, +TAG  |
// |-----------|-----------|---------|----------|---------|----------|------------|-------------|
// | (no tags) | ✅ RUN    | ✅ RUN  | ✅ RUN  | ✅ RUN  | ✅ RUN   | ✅ RUN    | ✅ RUN      |
// | +TAG      | ❌ SKIP   | ✅ RUN  | ✅ RUN  | ❌ SKIP | ❌ SKIP  | ✅ RUN    | ✅ RUN      |
// | -TAG      | ❌ SKIP   | ❌ SKIP | ❌ SKIP | ✅ RUN  | ✅ RUN   | ❌ SKIP   | ❌ SKIP     |
// | +OTHER    | ❌ SKIP   | ✅ RUN  | ❌ SKIP | ❌ SKIP | ✅ RUN   | ✅ RUN    | ❌ SKIP     |
// | -OTHER    | ❌ SKIP   | ❌ SKIP | ❌ SKIP | ❌ SKIP | ❌ SKIP  | ❌ SKIP   | ❌ SKIP     |


static inline int tst_check_tags(const char* tags_str) {
  // 1. Untagged tests always run
  if (!tags_str || !tags_str[0]) return 1;
  
  // 2. No filters = tagged tests disabled by default
  if (tst_argc <= 1) return 0;
  
  int state = 0;
  
  // 3. Process command line filters left to right
  for (int i = 1; i < tst_argc; i++) {
    const char* filter = tst_argv[i];
    char filter_sign = filter ? filter[0] : 0;
    
    if (filter_sign != '+' && filter_sign != '-') continue;
    
    const char* fname = filter + 1;
    int flen = strlen(fname);
    int is_wildcard = (filter_sign == '+' && fname[0] == '*' && fname[1] == '\0');

    // Iterate through tags in the test string
    const char* p = tags_str;
    while (*p) {
      // Skip separators
      while (*p == ' ' || *p == ',') p++;
      if (!*p) break;

      char tag_sign = *p;
      // Ensure we are looking at a valid tag start
      if (tag_sign == '+' || tag_sign == '-') {
        const char* tname = p + 1;
        // Find end of current tag
        const char* tend = tname;
        while (*tend && *tend != ' ' && *tend != ',') tend++;
        int tlen = (int)(tend - tname);

        // LOGIC:
        // 1. If wildcard (+*), any '+' tag enables the test
        // 2. If names match, enable if signs match (+Tag matches +Tag, -Tag matches -Tag)
        if (is_wildcard) {
           if (tag_sign == '+') { state = 1; break; }
        } else if (flen == tlen && strncmp(fname, tname, tlen) == 0) {
           state = (filter_sign == tag_sign);
           break; // Found specific match, stop scanning tags for this filter
        }
        p = tend;
      } else {
        // Skip invalid garbage
        while (*p && *p != ' ' && *p != ',') p++;
      }
    }
  }
  return state;
}

#define tst_main(tst_, tst_title_) \
  void tst__run(); \
  int main(int argc, char **argv) { \
    tst_title = tst_title_; \
    tst_argc = argc; \
    tst_argv = argv; \
    (void)argc; (void)argv; \
    for (int i = 1; i < argc; i++) { \
      if (strcmp(argv[i], "--report-error") == 0) tst_report_err = 1; \
    } \
    if (argc > 1 && strcmp(argv[1], "--list") == 0) {tst_list_opt = 1; tst__run(); exit(0); }\
    if (CLOCKS_PER_SEC > ((clock_t)1000000) + tst_zero) tst_clock_unit = "n"; \
    else if(CLOCKS_PER_SEC > ((clock_t)1000) + tst_zero) tst_clock_unit = "u"; \
    else tst_clock_unit = "m"; \
    fprintf(stderr, "----- %s %s \"%s\" %s%s\n", tst_str_file, __FILE__, tst_title, tst_time(), (tst_?"":" (disabled)"));\
    if (tst_) tst__run(); \
    tst_zero &= tst_usestatic; \
    fputs(tst_str_file_end,stderr); tst_prt_results(tst_fail, tst_pass, tst_skip); fprintf(stderr," %s\n",tst_time());\
    return ((tst_fail > 0) * tst_report_err); \
  } void tst__run() 

#define tstsuite(tst_title, ...)  tst_main((!tst_zero), tst_title) 
#define tst_suite(tst_title, ...) tst_main(( tst_zero), tst_title)

// This is only used to avoid that the compiler complains about unused static variables.
// #define tst_usestatic ((short)(  tst_result & tst_case_pass & tst_case_fail & tst_case_skip & tst_vars[0] & tstdata[0] & (int)tstelapsed))
#define tst_usestatic ((short)(  tst_result & tstdata[0] & (int)tstelapsed))

#define tst(x) (tst_result = (short)(!!(x)))

static inline int tstfailed(void)  {return !tst_result;}
static inline int tstpassed(void)  {return  tst_result;}
static inline int tstskipped(void) {return (tst_result < 0);}

#define tstcheck_(tst_abrt,tst_str,tst_res,...) \
  if (!tst_case_nested) ; else { \
    tst_result = (short)(tst_skip_test? -1 : !!(tst_res)); \
    switch (tst_result) { \
      case -1: tst_skip++; tst_case_skip++; tst_prtln(tst_str_skip); break; \
      case  0: tst_fail++; tst_case_fail++; tst_prtln(tst_str_fail); break; \
      case  1: tst_pass++; tst_case_pass++; if (tst_abrt<0) break; tst_prtln(tst_str_pass); break; \
    } \
    if (tst_result == 0 || tst_abrt >=0) fprintf(stderr, "%s", tst_str); \
    if (tst_result == 0) { \
      fprintf(stderr," \"" __VA_ARGS__); fputc('"',stderr); \
      if (tst_abrt == 1)  { \
        fputc('\n', stderr); tst_prtln_n(tst_case_ln,tst_str_case_end); tst_prt_results(tst_case_fail, tst_case_pass, tst_case_skip); \
        fputs(tst_str_file_abr,stderr); tst_prt_results(tst_fail, tst_pass, tst_skip); fprintf(stderr," %s\n",tst_time()); \
        exit(0);\
      } \
    } \
    if (tst_result == 0 || tst_abrt >=0) fputc('\n', stderr); \
  }

#define tstcheck(t_,...)     tstcheck_(0,#t_,t_,__VA_ARGS__)
#define tstassert(t_,...)    tstcheck_(1,#t_,t_,__VA_ARGS__)
#define tstexpect(t_,...)    tstcheck_(-1,#t_,t_,__VA_ARGS__)

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

static int tst_case_nested[1] = {0};


// Stringify bare tokens: tstcase("test", +Tag1, -Tag2) -> tstcase__("test", "+Tag1, -Tag2")
#define tstcase(tst_case_msg, ...) tstcase__(tst_case_msg, "" # __VA_ARGS__)

#define tstcase__(tst_case_msg, tst_tags_str) \
   if (tst_case_nested[0]) ; \
   else if (tst_list_opt) {\
     fprintf(stderr,"\"%s\"", tst_case_msg); \
     if (tst_tags_str[0]) fprintf(stderr," %s", tst_tags_str); \
     fputc('\n',stderr); \
   } \
   else if (tst_tags_str[0] && !tst_check_tags(tst_tags_str)) { \
     tst_skip++; \
     fprintf(stderr, "%5d ", tst_case_ln ? tst_case_ln : __LINE__); \
     fprintf(stderr, tst_str_skip_tst, tst_case_msg); \
     fputc('\n', stderr); \
   } else \
     for (int tst_case_nested = (tst_case_ln = __LINE__) ; tst_case_nested && !(tst_prtln(tst_str_case), tst_prtf(tst_case_msg)); tst_case_nested = (tst_case_ln = 0))\
        for (short tst_vars[6] = {0, tst_sect_not_last, 0, 0, 0, 0}; \
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

#define tst_check(...)   (void)tst(1)
#define tst_expect(...)  (void)tst(1)
#define tst_assert(...)  (void)tst(1)
#define tst_note(...)
#define tst_skpif(...)    if ( tst_zero) ; else
#define tst_clock(...)    if ( tst_zero) ; else
#define tst_case(...)     if (!tst_zero) ; else
#define tst_section(...)  if (!tst_zero) ; else

#ifdef __cplusplus
}
#endif

#endif // TST_VERSION
