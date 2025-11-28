# TST Maintainer Guide: `t2h` Architecture

> Audience: C developers maintaining or extending `src/t2h.c:1`.
>
> This document explains how the `t2h` utility parses TST logs,
> manages memory, extracts test source code, and generates HTML.

## 1. Overview

`t2h` converts TST log output into an interactive HTML report. The
high‑level flow (see `src/t2h.c:424-500`) is:

1. Parse CLI options (help, version, theme flags, input files).
2. Initialize `ProgramState` via `init_state`.
3. For each input (stdin or file):
   - Call `parse_logs` to build one or more `TestSuite` instances.
4. After all logs are parsed:
   - Call `extract_suite_sources` for each suite.
   - Call `generate_html` to write the report to stdout.
5. Cleanup with `free_state`.

`t2h` is single‑threaded and uses only the C standard library.


## 2. Core Data Structures and Ownership

Definitions around `src/t2h.c:62-153`:

### 2.1 `SourceLine`

```c
typedef struct {
    char *content;      // Dynamically allocated line content (via strdup)
    int linenum;        // Original line number from source file
} SourceLine;
```

- Ownership: each `SourceLine` owns its `content` pointer.
- Lifetime: allocated during source extraction; freed in
  `free_suite_contents` (`src/t2h.c:244-265`).

### 2.2 `SourceCode`

```c
typedef struct {
    char filepath[MAX_PATH];
    SourceLine *lines;           // Dynamically allocated array (calloc)
    int line_count;
    int start_line;
    int end_line;
    int available;               // 0=not loaded, 1=loaded, -1=error
} SourceCode;
```

- Owned by a `TestCase`.
- `lines` is a heap array of `line_count` entries.
- `available` encodes outcome of extraction:
  - `1` — success.
  - `0` — not attempted.
  - `-1` — error (e.g., file not found, OOM, non‑C file).

### 2.3 `TestCase`

```c
typedef struct {
    char name[MAX_NAME];
    int pass, fail, skip;
    int has_skpt;                // SKPT sections present
    int log_start, log_end;      // indices into suite->log_lines
    int line_number;             // CASE line number for source extraction
    SourceCode source;           // per‑case source code
} TestCase;
```

- Owned by `TestSuite.cases` (heap array).
- `log_start`/`log_end` delimit the slice of `log_lines` belonging to
  this case.
- `line_number` is taken from the TST log’s CASE line and used by
  `extract_suite_sources` (`src/t2h.c:1093-1097`).

### 2.4 `LogLine`

```c
typedef struct {
    char content[MAX_LINE];
    int linenum;
    enum { LINE_PASS, LINE_FAIL, LINE_SKIP, LINE_NOTE, LINE_CASE,
           LINE_SECTION, LINE_RESULT, LINE_CLOCK, LINE_ABORT,
           LINE_OTHER } type;
} LogLine;
```

- Owned by `TestSuite.log_lines` (heap array).
- `content` is a fixed‑size buffer containing a copy of the log line
  (with some normalization by `add_log_line`).
- `type` is assigned by `classify_line` (`src/t2h.c:371-396`).

### 2.5 `TestSuite`

```c
typedef struct {
    char title[MAX_NAME];
    char filename[MAX_PATH];
    char start_time[64];
    char end_time[64];
    int total_pass, total_fail, total_skip;
    TestCase *cases;
    int case_count, case_capacity;
    LogLine *log_lines;
    int log_line_count, log_capacity;
    int truncated;               // hit MAX_LOG_LINES
    int aborted;                 // ABRT instead of RSLT
} TestSuite;
```

- Owned by `ProgramState.suites` (heap array).
- Cases and log lines are heap arrays grown via `grow_cases` and
  `grow_log_lines`.
- `truncated` indicates that further log lines were dropped due to
  `MAX_LOG_LINES`.
- `aborted` is set when an `ABRT` marker (suite abort) is parsed.

### 2.6 `ProgramState`

```c
typedef struct {
    TestSuite *suites;
    int suite_count, suite_capacity;
    int total_pass, total_fail, total_skip;
    char batch_start[64];        // STIME
    char batch_end[64];          // ETIME
    int dark_theme;              // 0=light, 1=dark
} ProgramState;
```

- Created by `init_state` (`src/t2h.c:271-286`).
- Freed by `free_state` (`src/t2h.c:292-302`).
- Top‑level owner of all data.

### 2.7 Ownership Summary

| Object       | Allocated in             | Freed in                 |
|--------------|--------------------------|--------------------------|
| `ProgramState` | `init_state`            | `free_state`             |
| `suites[]`   | `init_state` / `realloc` | `free_state`             |
| `cases[]`    | `parse_logs` (SUIT)      | `free_suite_contents`    |
| `log_lines[]`| `parse_logs` (SUIT)      | `free_suite_contents`    |
| `SourceLine.content` | `extract_suite_sources` (`strdup`) | `free_suite_contents` |
| `SourceCode.lines` | `extract_suite_sources` (`calloc`)   | `free_suite_contents` |

> **WARNING:** Never free `TestSuite` or `TestCase` allocations
> directly; always go through `free_suite_contents` → `free_state` to
> avoid double‑frees or leaks.


## 3. Log Parser State Machine

Main parser: `static int parse_logs(ProgramState *state, FILE *input, const char *filename)` at `src/t2h.c:723`.

### 3.1 High‑Level Behavior

The parser reads input line by line:

1. Track line count (`line_count`) for fallback line numbers.
2. Handle batch metadata markers:
   - `STIME <timestamp>` → `state->batch_start`.
   - `ETIME <timestamp>` → `state->batch_end`.
3. Detect `SUIT /` lines to start new suites.
4. Within a suite:
   - Detect `ABRT \\` lines → aborted suite (`parse_abrt_line`).
   - Detect `RSLT \\` lines → completed suite (`parse_rslt_line`).
   - Detect `CASE,--` lines → start new test case.
   - Detect case result lines (`` `--- ``) → end test case.
   - For everything else: classify and store as `LogLine`.

### 3.2 State Variables

Local to `parse_logs`:

- `TestSuite *current_suite = NULL;`
- `TestCase *current_case = NULL;`
- `int line_count = 0;`

State transitions:

- `current_suite == NULL` → outside any suite.
- `current_case == NULL` → not inside a specific test case.

### 3.3 SUIT Handling

When a line contains `"SUIT /"` (`src/t2h.c:751-806`):

1. Finalize any previous suite by setting `current_suite`/`current_case`
   to `NULL`.
2. Grow `state->suites` if needed via `realloc` (subject to
   `MAX_SUITES`).
3. Initialize a new `TestSuite` with `memset`.
4. Allocate `cases` and `log_lines` with initial capacities
   (32 cases, 1000 log lines).
5. Parse suite metadata with `parse_suit_line`.
6. Store the SUIT line itself as a `LogLine` with no `linenum`.

**Error behavior:** if suite allocation fails, the function prints an
error and returns `0` (fatal error).

### 3.4 ABRT and RSLT

- `ABRT \\` handled at `src/t2h.c:813-823`:
  - `parse_abrt_line` fills `total_*` counters and end time.
  - `suite->aborted = 1`.
  - Global totals (`state->total_*`) are updated.
  - ABRT line is **not** stored as a `LogLine` (saves space).

- `RSLT \\` handled at `src/t2h.c:827-837`:
  - Same pattern via `parse_rslt_line`.
  - Not stored in `log_lines` for the same reason.

> **Note:** HTML output shows totals using `suite->total_*` and
> `state->total_*`; the raw RSLT/ABRT lines are not needed in the
> per‑case logs and would just duplicate summary information.

### 3.5 CASE and Result Lines

- CASE lines (`"CASE,--"`) at `src/t2h.c:841-855`:
  - If `grow_cases` succeeds, `current_case` is set to a new entry.
  - `parse_case_line` extracts `line_number` and case `name`.
  - `current_case->log_start` is set to current `log_line_count`.
  - The CASE line is **not** stored as a `LogLine`.

- Result lines (contain `` `--- `` without `"|`---"`) at
  `src/t2h.c:859-863`:
  - `parse_case_result` fills case `fail`, `pass`, `skip` counts.
  - `current_case->log_end` is set to current `log_line_count` (i.e.,
    just before this result line would have been appended).
  - Result line is not stored.
  - `current_case` becomes `NULL`.

This design keeps per‑case logs focused on individual PASS/FAIL/SKIP,
notes, etc., rather than the final summary line.

### 3.6 Regular Log Lines

For any other line while `current_suite` is non‑NULL:

1. Classify via `classify_line` (`src/t2h.c:368-396`).
2. Extract `linenum` using `extract_line_number`.
3. Append to `current_suite->log_lines` via `add_log_line`:
   - Stores normalized content (line number + original content sans
     initial numeric prefix).
4. If `current_case` is active and contains `"SKPT|"`, set
   `current_case->has_skpt = 1`.
5. Update `current_case->log_end` to be the last line index.

### 3.7 End‑of‑Input and Source Extraction

After reading all lines (`src/t2h.c:883-886`):

```c
for (int i = 0; i < state->suite_count; i++) {
    extract_suite_sources(&state->suites[i], filename);
}
```

Note: `filename` here is the last input filename passed to
`parse_logs`. For multi‑file runs, each call to `parse_logs` uses its
own `filename` and will perform extraction for suites it parsed from
that file.

> **WARNING:** If you change when or how `extract_suite_sources` is
> called, ensure you preserve the invariant that each suite is
> processed once after all of its log lines have been read.


## 4. Memory Management Deep Dive

### 4.1 Initialization and Cleanup

`init_state` (`src/t2h.c:271-286`):

- Allocates a zeroed `ProgramState` via `calloc`.
- Sets initial `suite_capacity = 16`.
- Allocates `suites` array via `calloc` (zeroed `TestSuite`s).
- Default theme: `dark_theme = 1`.

`free_state` (`src/t2h.c:292-302`):

- Null‑safe, returns if `state == NULL`.
- Calls `free_suite_contents` for each active suite.
- Frees `state->suites`, then `state` itself.

`free_suite_contents` (`src/t2h.c:244-265`):

- For each `TestCase` in the suite:
  - If `source.lines` is non‑NULL:
    - Free every `SourceLine.content` (allocated via `strdup`).
    - Free `source.lines`.
- Free `suite->cases` and `suite->log_lines`.

> **WARNING:** `free_suite_contents` assumes that ownership of
> `SourceLine.content` was fully transferred from temporary buffers
> during extraction. Never reuse `SourceLine` structs without either
> re‑initializing or freeing their contents.

### 4.2 Growing Cases and Log Lines

`grow_cases` (`src/t2h.c:309-332`):

- If `case_count < case_capacity`, nothing to do.
- If capacity ≥ `MAX_CASES` (512), print a warning and return 0; extra
  cases are skipped.
- Otherwise, double `case_capacity` (up to `MAX_CASES`) and `realloc`
  `cases`.
- Zero out newly allocated region with `memset`; this is important so
  newly used `TestCase` entries start in a known state.

`grow_log_lines` (`src/t2h.c:339-364`):

- Similar strategy, but when capacity ≥ `MAX_LOG_LINES` (20000):
  - Set `suite->truncated = 1`.
  - Return `0`, causing additional lines to be dropped.
- Zeroes new region with `memset` after `realloc`.

### 4.3 Hard Limits and Trade‑offs

Constants at `src/t2h.c:44-56`:

- `MAX_LINE 4096` — max length of one input line.
- `MAX_CASES 512` — hard limit per suite.
- `MAX_SUITES 64` — hard limit per run.
- `MAX_LOG_LINES 20000` — per‑suite log storage limit.
- `MAX_SOURCE_LINES 10000` — per‑case source lines (stack buffer).

Trade‑offs:

- Stack usage: `SourceLine temp_lines[MAX_SOURCE_LINES];` in
  `extract_suite_sources` consumes stack space proportional to
  `MAX_SOURCE_LINES` (`src/t2h.c:1111-1113`). Increasing this may break
  deeply nested call stacks or low‑stack environments.
- Heap usage: `MAX_LOG_LINES` × `sizeof(LogLine)` per suite; increasing
  this can significantly raise memory usage for large logs.

> **WARNING:** Before raising `MAX_SOURCE_LINES`, evaluate worst‑case
> stack depth and run tests under tools like Valgrind or ASan.


## 5. Source Extraction Algorithm

`extract_suite_sources(TestSuite *suite, const char *log_path)` at
`src/t2h.c:1070`.

### 5.1 Entry Conditions

- If `suite == NULL` or `suite->filename[0] == '\0'`, return early.
- If the filename extension is not a known C/C++ extension
  (`.c`, `.cpp`, `.cc`, `.cxx`, etc.), mark all `source.available = -1`
  and return (`src/t2h.c:1074-1080`).
- Try to open the source file:
  - Current directory.
  - Same directory as `log_path` (if not `<stdin>`).
  - `"test/"`, `"src/"`, `"tests/"`, `"./"` prefixes.

If the file cannot be opened, all cases’ `source.available` are set to
`-1` and the function returns.

### 5.2 Per‑Case Initialization

Loop over `suite->case_count` (`src/t2h.c:1093-1100`):

- Copy `suite->filename` into `tc->source.filepath`.
- Set `tc->source.start_line` from `tc->line_number`.
- Initialize `source.available = -1`, `lines = NULL`, `line_count = 0`.

Availability will be set to `1` upon successful extraction.

### 5.3 Single‑Pass Scan

Local variables (`src/t2h.c:1103-1113`):

- `char line[MAX_LINE];`
- `int current_line = 0;`
- `int current_case_idx = 0;`
- `int in_block_comment = 0;`
- `int brace_count = 0;`
- `int found_opening = 0;`
- `SourceLine temp_lines[MAX_SOURCE_LINES];`
- `int temp_line_count = 0;`

Algorithm:

1. Read source file line by line with `fgets`.
2. Increment `current_line`.
3. If `current_case_idx < suite->case_count`:
   - Let `tc = &suite->cases[current_case_idx]`.
   - If `current_line == tc->line_number`:
     - Reset `brace_count`, `found_opening`, `temp_line_count`, and
       `in_block_comment` (start of this case’s function body).
   - If `current_line >= tc->line_number` and `temp_line_count < MAX_SOURCE_LINES`:
     - Strip trailing newline.
     - Duplicate the line into `temp_lines[temp_line_count]`.
     - On OOM, free temp lines, mark `available = -1`, advance to next
       case, and continue.
     - Update `temp_line_count`.
     - If `!found_opening || brace_count > 0`, run brace‑tracking logic
       to detect when the function body is complete.

4. Brace tracking uses both `is_in_string_literal` and
   `is_in_comment` plus `in_block_comment` to avoid counting braces in
   comments or strings (`src/t2h.c:1154-1174`).

5. When `found_opening && brace_count == 0` (`src/t2h.c:1183-1206`):
   - Set `tc->source.end_line = current_line`.
   - Allocate `tc->source.lines` via `calloc(temp_line_count, sizeof(SourceLine))`.
   - Transfer ownership of `temp_lines[k].content` into
     `tc->source.lines[k]` via shallow copy.
   - Set `tc->source.line_count = temp_line_count` and `available = 1`.
   - Reset `temp_line_count = 0`.
   - `current_case_idx++` (move on to next case).

6. When all cases are processed (`current_case_idx >= suite->case_count`):
   - Free any uncommitted `temp_lines` (safety net) and stop reading.

### 5.4 EOF and Error Handling

At end of file (`src/t2h.c:1220-1243`):

- If `current_case_idx < suite->case_count` and `temp_line_count > 0`:
  - Treat this as a partial extraction (e.g., missing closing brace).
  - Allocate `tc->source.lines` and transfer `temp_lines` as above.
  - Set `source.available = 1` and `end_line = current_line`.
- Else if `temp_line_count > 0` but no case is pending:
  - Free `temp_lines` for safety (shouldn’t normally happen).

### 5.5 Limitations and Gotchas

- Brace detection assumes a fairly conventional style: function bodies
  start at or after the CASE line number and end when brace balance
  returns to zero.
- Lexing is not full C; comments and strings are approximated enough to
  avoid most false positives for braces, but unusual macro constructs
  may confuse it.
- Multi‑line string literals are not supported (C itself doesn’t allow
  them) — only backslash‑continued ones are recognized at the character
  level.

> **WARNING:** If you modify `is_in_string_literal` or `is_in_comment`,
> re‑verify that braces in comments or strings do not affect
> `brace_count`, or source ranges will become incorrect.


## 6. HTML Generation Pipeline

Main function: `generate_html(ProgramState *state, FILE *output)` at
`src/t2h.c:1775`.

### 6.1 Title and Theme

- Title selection (`src/t2h.c:1779-1784`):
  - `"Test Results"` by default.
  - If exactly one suite, use `suite[0].title`.
  - If more than one suite, use `"Consolidated Test Results"`.
- Theme:
  - `state->dark_theme` (set in `main` from `--light`/`--dark` flags)
    controls the `data-theme="dark"` attribute on `<body>` and
    initial CSS variables.

### 6.2 Header and Per‑Suite Sections

1. `output_html_header` (`src/t2h.c:1452-1548`):
   - Emits `<!DOCTYPE html>`, `<html>`, `<head>`.
   - Writes full inline `<style>` with CSS variables for light/dark
     themes and all layout classes.
   - Opens `<body>` and main `.container` div.

2. If `state->suite_count > 1`, emit a consolidated header card with
   totals and theme toggle button (`src/t2h.c:1789-1803`).

3. For each suite, call `output_suite` (`src/t2h.c:1695-1771`):
   - Constructs a header bar with title, file, timestamps, and badges.
   - Adds an aborted skull indicator `    ` when `suite->aborted`.
   - Provides per‑suite collapse/expand and top buttons for multi‑suite
     reports.
   - Wraps test cases in a `.card` within `.suite-content`.

4. End with `output_html_footer` (`src/t2h.c:1550-1562`):
   - Closes container and body.
   - Emits small inline `<script>` with JS helpers for collapse,
     toggling, tab switching, and theme switching.

### 6.3 Test Case Rendering

`output_test_case` (`src/t2h.c:1565-1692`) builds each case block:

- Determines `status_class` via `tc->fail`, `tc->pass`, `tc->skip`, and
  `tc->has_skpt`.
- Shows counts in badges; for aborted suites, uses a skull icon in the
  fail badge of the last case.
- Renders Log and Code buttons and tab controls if source is available.
- Log viewer:
  - Uses `tc->log_start`/`log_end` to slice `suite->log_lines`.
  - For each log line:
    - Map `type` to CSS class: `log-pass`, `log-fail`, etc.
    - HTML‑escape content, preserving leading spaces via `&nbsp;`.

- Source viewer:
  - For each `SourceLine`:
    - Print linenum in `.log-linenum` span.
    - Call `output_source_line_highlighted` to emit syntax‑highlighted
      code.

### 6.4 Syntax Highlighting

`output_source_line_highlighted` (`src/t2h.c:1286-1446`):

- Tokenizes the line in a single pass:
  - Tracks `in_string`, `in_char`, `in_line_comment`, `in_block_comment`.
  - Highlights:
    - C keywords (from a fixed list) as `.src-keyword`.
    - Common typedefs and types as `.src-type`.
    - Strings and chars as `.src-string`.
    - Comments as `.src-comment`.
    - Preprocessor lines as `.src-preprocessor`.
    - Numbers as `.src-number`.
- All `<`, `>`, and `&` are HTML‑escaped.

> **NOTE:** This is a best‑effort highlighter, not a full C parser. It
> is good enough for typical test code.


## 7. Security Considerations

### 7.1 HTML Escaping

- `html_escape(FILE *out, const char *s)` (`src/t2h.c:220-235`) escapes
  `&`, `<`, `>`, `"`, and `'`.
- Used for:
  - Suite titles (`output_suite`).
  - File names.
  - Test case names (`output_test_case`).
  - Log content and source when not already escaped.

### 7.2 XSS Risks

`t2h` treats logs and source as untrusted input. Escaping is applied in
all user‑visible text fields.

> **WARNING:** When adding new HTML output paths that include log or
> source content, always use `html_escape` or follow the patterns used
> in `output_source_line_highlighted`. Do not insert raw log content
> into attributes or inside `<script>` blocks.


## 8. Maintenance Tips and Cross‑References

When changing one part of the system, review related components:

| Change Type                | Also Review                       |
|---------------------------|------------------------------------|
| Log markers in `tst.h`    | `classify_line`, RSLT/ABRT parsers |
| New marker (e.g. WARNING) | `LINE_*` enum, HTML CSS classes    |
| Raising MAX_* limits      | Stack/heap usage, performance      |
| Source extraction logic   | HTML source rendering, docs        |
| Theme/CSS changes         | Class names in HTML/JS             |

For user‑visible CLI and behavior, refer to:

- `docs/ref_manual.md:963` (t2h utility reference).
- `docs/prog_manual.md:894` (HTML test reports with t2h).
