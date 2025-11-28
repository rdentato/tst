# TST Maintainer Guide: `tst.h` Internals

> Audience: C developers maintaining or extending `src/tst.h:1`.
>
> This document explains how the single‑header TST framework works
> internally: tag filtering, macro‑based control flow, global state,
> and log‑format coupling. It focuses on *why* the design looks like
> this and what must remain stable.

## 1. Overview

`tst.h` is a header‑only test framework that:

- Defines the full `main()` entry point for a test binary.
- Provides macros to organize tests into suites, cases, and sections.
- Emits a structured, machine‑parseable log format.
- Implements tag‑based filtering and conditional skipping.

Public behavior is documented in `docs/ref_manual.md:66` and
`docs/prog_manual.md:45`. This file explains the implementation that
backs that behavior.

Key constraints:

- Header‑only, no separate library.
- C89/C99‑friendly; must compile as C++ as well.
- No dynamic allocation in the framework itself.
- Single‑threaded design; not safe for concurrent suites in one
  process.


## 2. Tag System Architecture

Tag filtering is implemented in `tst_check_tags`:

- Function: `static inline int tst_check_tags(const char *tags_str)`
- Location: `src/tst.h:101`

It decides whether a tagged test case should run based on
command‑line filters stored in:

- `static int tst_argc;` / `static char **tst_argv;` — set in `tst_main`
  (`src/tst.h:79-80`, `src/tst.h:156-162`).

### 2.1 Inputs

- `tags_str` — stringified `tstcase` tag list, e.g. `"+RequiresDB, -Slow"`.
  Created by the macro pair:
  - `#define tstcase(tst_case_msg, ...) tstcase__(tst_case_msg, "" # __VA_ARGS__)`
    (`src/tst.h:249-250`).
- CLI filters — each `argv[i]` starting with `+` or `-`, for example:
  - `+RequiresDB`
  - `-SlowTests`
  - `+*`

Untagged tests use `tags_str == NULL` or `tags_str[0] == '\0'` and are
always executed (`src/tst.h:101-103`), regardless of CLI filters.

### 2.2 Algorithm

The function implements the semantics documented in
`docs/ref_manual.md:529` and `docs/prog_manual.md:610`.

1. **Untagged tests**
   ```c
   if (!tags_str || !tags_str[0]) return 1;
   ```
   Untagged cases run even when no filters are specified.

2. **Tagged tests, no filters**
   ```c
   if (tst_argc <= 1) return 0;
   ```
   Tagged tests are disabled by default when you do not pass any
   filters. This matches the manual: *"Tagged tests are disabled by
   default; must be explicitly enabled"*.

3. **Filter loop** (`src/tst.h:110-152`)

   - For each CLI argument `filter = tst_argv[i]`, if it starts with
     `+` or `-`, compute:
     - `filter_sign = filter[0];` → `+` or `-`.
     - `fname = filter + 1;` — filter name.
     - `is_wildcard = (filter_sign == '+' && fname[0] == '*' && fname[1] == '\0');`
   - For each tag in `tags_str`:
     - Tags are separated by spaces or commas.
     - Valid tags start with `+` or `-` and are followed by a name.

4. **Matching rules** (`src/tst.h:137-145`)

   For each filter, all tags in `tags_str` are scanned:

   - Wildcard `+*`:
     ```c
     if (is_wildcard) {
       if (tag_sign == '+') { state = 1; break; }
     }
     ```
     Any `+Tag` on the test causes `state=1` (run). `-Tag` tests are
     not affected by `+*`.

   - Named filters:
     ```c
     else if (flen == tlen && strncmp(fname, tname, tlen) == 0) {
       state = (filter_sign == tag_sign);
       break;
     }
     ```
     Filters apply when the filter name and tag name are equal.

     - `+TAG` + `+TAG` → `state = 1`.
     - `+TAG` + `-TAG` → `state = 0`.
     - `-TAG` + `-TAG` → `state = 1`.
     - `-TAG` + `+TAG` → `state = 0`.

5. **Left‑to‑right overrides**

   - `state` is updated as filters are processed in order.
   - Later matching filters overwrite previous decisions but only if
     they match at least one tag.
   - If a filter does not match any tag in `tags_str`, it leaves
     `state` unchanged.

6. **Return value**

   - `return state;` means:
     - `1` → test is enabled and will run.
     - `0` → test is skipped at `tstcase` level.

### 2.3 Complexity

Let:

- `F` — number of CLI filters with `+`/`-` prefixes.
- `T` — number of tags on the test case.
- `L` — average length of tag names.

Worst‑case complexity:

- Time: `O(F × T × L)`, because every filter scans every tag and does a
  linear string comparison.
- Space: `O(1)`, all work is done in‑place.

In practice, F and T are small (≤ 10), so this is fine.

### 2.4 Edge Cases and Invariants

- Untagged tests always run and are *not* subject to tag filters.
- Tagged tests are *disabled by default* if no filters are provided.
- Malformed `tags_str` entries without a `+`/`-` prefix are simply
  skipped.
- Filters without `+` or `-` are ignored.
- `+*` only affects `+Tag` tests.

> **WARNING:** Any semantic change to `tst_check_tags` must be reflected
> in:
> - Tag tables in `docs/ref_manual.md` and `docs/prog_manual.md`.
> - Examples in those manuals.
> - Downstream tools or scripts that rely on existing tag semantics.


## 3. Macro‑Based Control Flow

### 3.1 `tst_main`, `tstsuite`, and `tst_suite`

Entry point generation is centralized in the `tst_main` macro
(`src/tst.h:156-175`):

```c
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
    if (argc > 1 && strcmp(argv[1], "--list") == 0) {tst__run(1); exit(0); }\
    ...
```

Key points:

- `tst__run(int tst_list_opt)` is the generated test function.
- `tst_list_opt` is `1` when `--list` is passed; this makes `tstcase`
  print only test names and tags.
- `tst_` argument is used to enable/disable running tests entirely:
  - `tstsuite` passes `(!tst_zero)` → enabled.
  - `tst_suite` passes `( tst_zero)` → disabled.

`src/tst.h:177-178`:

```c
#define tstsuite(tst_title, ...)  tst_main((!tst_zero), tst_title)
#define tst_suite(tst_title, ...) tst_main(( tst_zero), tst_title)
```

When a suite is disabled (`tst_suite`), `tst_` evaluates false so the
body of `tst__run` is never executed, but the binary still prints the
suite header and summary.

### 3.2 `tstcase` Flow

`tstcase` wraps its body in one or more `for` loops, controlling:

- Case header printing.
- Per‑case counter initialization.
- Per‑section iteration.
- Final case summary printing.

Definition (simplified) from `src/tst.h:249-269`:

```c
#define tstcase(tst_case_msg, ...) \
  tstcase__(tst_case_msg, "" # __VA_ARGS__)

#define tstcase__(tst_case_msg, tst_tags_str) \
   if (tst_case_nested[0]) ; \
   else if (tst_list_opt) { ... list mode ... } \
   else if (tst_tags_str[0] && !tst_check_tags(tst_tags_str)) { ... skip ... } \
   else \
     for (int tst_case_nested = (tst_case_ln = __LINE__) ; \
            tst_case_nested && !(tst_prtln(tst_str_case), \
                                  tst_prtf(tst_case_msg)); \
            tst_case_nested = (tst_case_ln = 0))\
       for (short tst_vars[6] = {0, tst_sect_not_last, 0, 0, 0, 0}; \
            ((tst_sect_counter == tst_sect_not_last) && \
               (tst_sect_counter = -1)) || \
             (tst_prtln(tst_str_case_end), \
              tst_prt_results(tst_case_fail, tst_case_pass, tst_case_skip), \
              tst_zero &= (short)fputc('\n',stderr));\
            tst_sect_iterator += 1)
```

Important pieces:

- `tst_case_nested` is a *local* loop variable that shadows the static
  array `tst_case_nested[1]`. Its role is to ensure that the body of the
  `for` executes once, and then the loop terminates by setting
  `tst_case_nested = 0`.
- `tst_case_ln` stores the start line of the case for use when printing
  summaries and for tools like `t2h`.
- `tst_vars[6]` is a per‑case array used as:
  - `tst_sect_iterator  = tst_vars[0]`
  - `tst_sect_counter   = tst_vars[1]`
  - `tst_case_pass      = tst_vars[2]`
  - `tst_case_fail      = tst_vars[3]`
  - `tst_case_skip      = tst_vars[4]`
  - `tst_skip_test      = tst_vars[5]`

These aliases are defined around `src/tst.h:237-245`.

The inner `for` loop ensures:

- `tst_vars` are zero‑initialized at case entry.
- The case summary line is printed exactly once when the loop exits.
- `tst_sect_iterator` is incremented on each iteration, driving
  `tstsection` logic.

> **WARNING:** `tst_vars` is sized exactly 6. Adding new aliases or
> repurposing indices without updating all uses will corrupt case
> counters and skip logic.

### 3.3 `tstsection` Flow

`tstsection` is nested inside the `tstcase` loops and is defined
(`src/tst.h:276-282`) roughly as:

```c
#define tstsection(...) \
  for (int tst_sect = 1; \
       tst_sect && ((tst_sect_counter > tst_sect_not_last) || \
                    !(tst_sect_counter = tst_sect_not_last))\
                && (++tst_sect_counter == tst_sect_iterator) \
                && !(tst_prtln(tst_str_sctn), \
                     tst_prtf(" " __VA_ARGS__)); \
       tst_sect = 0, \
       tst_sect_counter = tst_sect_last, \
       tst_prtln(tst_str_sctn_end), \
       fputc('\n',stderr)) \
    for (int tst_data_count = 0; \
         tst_data_count < tst_data_size; \
         tst_data_count++)
```

Key invariants:

- `tst_sect_iterator` increments at the `tstcase` level.
- Each `tstsection` runs when `++tst_sect_counter == tst_sect_iterator`.
- `tst_sect_not_last` and `tst_sect_last` are special sentinels used to
  distinguish *no section* vs. *section in progress vs. final section*.
- The inner `tst_data_count` loop drives data‑driven testing via:
  - `tstcurdata` → `tstdata[tst_data_count]`.
  - `tst_data_size` computed from `sizeof(tstdata)/sizeof(tstdata[0])`.

As described in `docs/prog_manual.md:351`, each section is effectively
run once per `tstdata` element.

> **WARNING:** Sections are not nestable. Trying to introduce nested
> `tstsection` behavior will break the `tst_sect_counter` /
> `tst_sect_iterator` invariants.

### 3.4 Disabled Macros

Disabled variants tie into the same code paths by compiling to no‑ops:

```c
#define tst_check(...)
#define tst_assert(...)
#define tst_note(...)
#define tst_skpif(...)    if ( tst_zero) ; else
#define tst_clock(...)    if ( tst_zero) ; else
#define tst_case(...)     if (!tst_zero) ; else
#define tst_section(...)  if (!tst_zero) ; else
```

These ensure syntax remains valid, but no code is executed or emitted.


## 4. Static State Design

### 4.1 Globals and Their Roles

At the top of `src/tst.h:29-38`:

```c
static volatile short tst_zero = 0;
static short tst_result     = 0;
static short tst_report_err = 0;

static int tst_pass = 0;
static int tst_fail = 0;
static int tst_skip = 0;
static const char* tst_title = NULL;
static int tst_case_ln = 0;
```

Plus later:

- `static int tst_argc;`
- `static char **tst_argv;`
- `static const char *tst_clock_unit;`
- `static clock_t tstelapsed = 0;`
- `static volatile unsigned short tstdata[1] = {0};`
- `static int tst_case_nested[1] = {0};`

Characteristics:

- **Lifetime:** static storage duration; they live for the entire
  process.
- **Scope:** internal to the translation unit that includes `tst.h`.
- **Thread‑safety:** not thread‑safe. Tests assume single‑threaded
  execution.

`tst_zero` is a special flag used in several ways:

- As a trivial `volatile` variable to inhibit certain compiler
  optimizations and keep static variables “used”.
- As part of the `tst_usestatic` macro (`src/tst.h:182`) to reference
  static arrays and prevent “unused variable” warnings.

```c
#define tst_usestatic ((short)(  tst_result & tstdata[0] & (int)tstelapsed))
```

The expression doesn’t have semantic effect on test logic; it is a
compact trick to touch otherwise only‑read or only‑written statics.

### 4.2 Result Query Functions

These query `tst_result` (`src/tst.h:186-188`):

```c
static inline int tstfailed(void)  {return !tst_result;}
static inline int tstpassed(void)  {return  tst_result;}
static inline int tstskipped(void) {return (tst_result < 0);}
```

`tst_result` is set by `tst` and all check/assert macros, including
skip handling via negative values.

### 4.3 Clock Handling

Clock state (`src/tst.h:221-229`):

- `static const char *tst_clock_unit;`
- `static clock_t tstelapsed = 0;`
- `#define tstelapsed() tstelapsed`

`tstclock` chooses units based on `CLOCKS_PER_SEC` in `tst_main`
(`src/tst.h:166-169`) and stores elapsed ticks in `tstelapsed`.

> **WARNING:** Any change in unit selection must keep the
> documentation in `docs/ref_manual.md:693` and `docs/prog_manual.md:1076`
> accurate.


## 5. Output Format Coupling

The strings at `src/tst.h:39-55` define the exact log markers:

```c
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
```

Downstream tools (including `t2h`) depend on these literal prefixes:

- `t2h` classifies lines by scanning for `"PASS|"`, `"FAIL|"`,
  `"SKIP|"`, `"SKPT|"`, `"CASE,--"`, `"SCTN|,--"`, `"CLCK:"`, `"NOTE:"`,
  `"RSLT \\"`, `"ABRT \\"`, and `"SUIT /"`.
- The reference manual’s “Output Format Specification” uses the same
  markers (`docs/ref_manual.md:1216`).

> **WARNING:** Changing these strings will break both documented
> behavior and `t2h`’s parsing logic. Any change here requires a
> coordinated update across:
> 
> - `src/t2h.c` classification (`classify_line`, parsers for SUIT/RSLT/ABRT).
> - `docs/ref_manual.md` and `docs/prog_manual.md` examples.
> - Any external scripts consuming logs.


## 6. Macro and printf Safety Patterns

### 6.1 `tst` and Result‑Setting

`tst` is implemented as (`src/tst.h:184`):

```c
#define tst(x) (tst_result = (short)(!!(x)))
```

- Sets `tst_result` to `1` for true, `0` for false.
- Used by more complex macros to track LAST result.

### 6.2 `tstcheck_` and Friends

Core work is done by `tstcheck_` (`src/tst.h:190-208`):

```c
#define tstcheck_(tst_abrt,tst_str,tst_res,...) \
  if (!tst_case_nested) ; else { \
    tst_result = (short)(tst_skip_test? -1 : !!(tst_res)); \
    switch (tst_result) { ... } \
    if (tst_result == 0 || tst_abrt >=0) fprintf(stderr, "%s", tst_str); \
    if (tst_result == 0) { \
      fprintf(stderr," \"" __VA_ARGS__); fputc('"',stderr); \
      if (tst_abrt == 1)  { ... abort suite ... } \
    } \
    if (tst_result == 0 || tst_abrt >=0) fputc('\n', stderr); \
  }
```

Front‑end macros:

```c
#define tstcheck(t_,...)     tstcheck_(0,#t_,t_,__VA_ARGS__)
#define tstassert(t_,...)    tstcheck_(1,#t_,t_,__VA_ARGS__)
#define tstexpect(t_,...)    tstcheck_(-1,#t_,t_,__VA_ARGS__)
```

Notes:

- `#t_` stringifies the expression exactly as written, matching the
  manuals’ examples.
- `__VA_ARGS__` is concatenated after a leading space and opening quote
  `" "`. When no extra args are present, C99 variadic macros allow
  this to compile as if the comma were absent.
- `tst_abrt` controls both logging detail and whether a failing check
  aborts the suite.

> **WARNING:** When adding new macros that forward to `tstcheck_`, ensure
> that:
> 
> - Expressions are evaluated exactly once.
> - Stringification (`#expr`) is used where expression text matters.
> - You propagate `__VA_ARGS__` so formatted messages keep working.

### 6.3 Outer‑Error Wrapper and Notes

`tstnote` and `tstouterr` provide structured, multi‑line diagnostics.

- `tstnote` (`src/tst.h:231`):
  ```c
  #define tstnote(...) (tst_prtln(tst_str_note), tst_prtf( " " __VA_ARGS__))
  ```
- `tstouterr` (`src/tst.h:233-235`) wraps a block in outer delimiters
  `<<<<<` and `>>>>>`.

These macros assume:

- `tst_prtln` and `tst_prtf` print to `stderr` and also ensure a
  trailing newline.
- Messages are treated as raw strings; they do not affect test counts.


## 7. Design Rationale & Maintenance Tips

### 7.1 Why Macros and Statics?

- No runtime dependencies, no separate build target.
- Maximum portability: just include `tst.h` and compile.
- Macros preserve source expressions verbatim in logs.

Trade‑offs:

- Debugging macro expansions is harder (see `debugging_and_review.md`).
- Not thread‑safe; per‑process static state.

### 7.2 Safe Change Patterns

When modifying `tst.h`:

- **Do:**
  - Keep public macros (`tstsuite`, `tstcase`, `tstcheck`, etc.) stable
    in name and top‑level behavior.
  - Add new behavior via new macros rather than changing existing
    semantics when possible.
  - Preserve log markers or coordinate changes with `t2h` and docs.

- **Don’t:**
  - Change `tst_str_*` literals casually.
  - Assume re‑entrancy or multi‑threaded execution.
  - Introduce extra evaluations of user‑supplied expressions.


## 8. Quick Reference (Maintainer View)

| Topic          | Where                | Notes |
|----------------|----------------------|-------|
| Tag engine     | `src/tst.h:101`      | Implements CLI filters over `+Tag`/`-Tag`. |
| Suite entry    | `src/tst.h:156-178`  | `tst_main`, `tstsuite`, `tst_suite`. |
| Case loop      | `src/tst.h:249-269`  | `tstcase__`, nested `for` loops, `tst_vars`. |
| Sections       | `src/tst.h:276-282`  | Drive per‑section and data iteration. |
| Counters       | `src/tst.h:33-35, 237-245` | Suite and case counts. |
| Skip blocks    | `src/tst.h:214-219`  | `tstskipif` uses `tst_skip_test`. |
| Timing         | `src/tst.h:221-229, 166-169` | `tstclock`, `tstelapsed`, units. |
| Log markers    | `src/tst.h:39-55`    | Coupled to `t2h` and manuals. |

For end‑user semantics of all macros, refer to `docs/ref_manual.md:110`
and `docs/prog_manual.md:225`.
