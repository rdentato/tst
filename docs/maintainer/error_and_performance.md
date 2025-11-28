# TST Maintainer Guide: Error Handling & Performance

> Audience: C maintainers interested in robustness, edge cases, and
> performance characteristics of `tst.h` and `t2h.c`.

This document summarizes:

- How TST (`tst.h`) and `t2h` behave on edge cases.
- Error handling strategies and failure modes.
- Performance characteristics and scalability considerations.

For semantic details of APIs, see `docs/ref_manual.md` and
`docs/prog_manual.md`. For internal design, see
`tst_h_internals.md` and `t2h_architecture.md`.


## 1. `tst.h` Edge Cases and Error Behavior

`tst.h` is a header‑only framework with minimal explicit error
handling. Most behavior on edge conditions is defined by how macros and
counters work.

### 1.1 Zero Assertions in a Test

A `tstcase` or `tstsuite` block with no assertions:

- Still emits CASE/SUIT headers and summaries.
- Case summary will show `0 FAIL | 0 PASS | 0 SKIP`.
- Suite summary aggregates across all cases; a suite with only empty
  cases has all zero counts.

There is no special error – this is considered a valid (if not very
useful) test.

**Suggested tests:**

- Create a suite with a case containing only `tstnote` or
  `tstouterr`, no checks, and confirm output.

### 1.2 Empty `tstdata` Arrays

A data‑driven section relies on `tst_data_size`:

```c
#define tst_data_size ((int)(sizeof(tstdata)/sizeof(tstdata[0])))
```

If `tstdata` is an empty array (`{}`), then `tst_data_size == 0`:

- The inner `for (int tst_data_count = 0; tst_data_count < tst_data_size; ...)`
  body is never executed.
- Section header (`SCTN|,--`) is still printed, followed directly by
  the section end line with no contained checks.

There’s no runtime error, but this usually indicates a test design
problem.

**Suggested test:**

- Deliberately define `int tstdata[0];` inside a case and inspect
  logs.

### 1.3 `tstskipif` with Side Effects

`tstskipif(cond)` uses a `for` loop and `tst_skip_test` flag
(aliased to `tst_vars[5]`) to mark enclosed checks as skipped.

If `cond` has side effects, they are evaluated once per skip block.
Within the block, all checks are treated as SKIP.

**Guidance:**

- Avoid side effects in `cond` other than simple checks of external
  state (e.g., `!db_available`).
- Do not rely on `cond` being re‑evaluated for each check.

### 1.4 Counter Overflow

Counters are:

- Suite‑level: `int tst_pass`, `tst_fail`, `tst_skip`.
- Case‑level: `short tst_case_pass`, `tst_case_fail`, `tst_case_skip`.

In practice, even very large suites rarely approach `INT_MAX`. Case
counters (`short`) may overflow for pathological logs with tens of
thousands of checks in a single case.

Overflow effects:

- Only displayed counts and any logic that uses them are affected.
- No bounds checks are performed; behavior is standard signed overflow
  (undefined at the language level, but well‑behaved on typical
  platforms).

**Recommendation:**

- Keep per‑case check counts under a few thousand.
- For huge loops, prefer `tstexpect` to log only failures.


## 2. `t2h` Robustness and Edge Cases

`src/t2h.c` explicitly handles various malformed or extreme inputs.

### 2.1 Incomplete Logs (Missing RSLT/ABRT)

If a suite begins with `SUIT /` but never sees a `RSLT \\` or
`ABRT \\` line:

- `parse_logs` still increments `state->suite_count` when the SUIT
  line is seen.
- `suite->total_*` may remain zero if no case result lines were parsed.
- The suite is rendered in HTML with whatever per‑case data is
  available.
- There is no explicit error; the run looks like a partially
  executed suite.

**Suggestion:**

- For CI contexts, use the raw log or `t2h` warnings to detect such
  cases rather than relying on totals alone.

### 2.2 Missing or Malformed Markers

- SUIT without CASE: allowed; `t2h` shows an empty suite with only
  totals.
- CASE without result line: `current_case` remains non‑NULL at EOF.
  The last `log_end` is updated as lines arrive, so the log viewer will
  show lines up to the end of the suite, but pass/fail counts might not
  be filled.
- Lines missing numeric prefixes (line numbers): `extract_line_number`
  returns 0; `add_log_line` falls back to `line_count` as the numeric
  prefix.

### 2.3 Allocation Failures

- `init_state` returning NULL (`src/t2h.c:271-285`):
  - `main` prints `"t2h: fatal: failed to allocate memory"` and
    exits with code 1.

- `parse_logs` OOM on suites (`realloc` for `suites[]`):
  - Prints `"t2h: error: out of memory growing suites"` and returns 0.

- `parse_logs` OOM on `cases` or `log_lines` for a suite:
  - Prints `"t2h: error: out of memory allocating suite"` and returns 0.

- `grow_cases` reaching `MAX_CASES` (`src/t2h.c:309-316`):
  - Prints a warning and returns 0. Additional CASE markers in that
    suite are silently skipped.

- `grow_log_lines` reaching `MAX_LOG_LINES` (`src/t2h.c:339-345`):
  - Sets `suite->truncated = 1` and returns 0. Additional log lines are
    dropped.

In all these non‑fatal cases, the HTML output may be incomplete, but
`t2h` does not crash.

### 2.4 Long Lines (> `MAX_LINE`)

`fgets(line, sizeof(line), input)` reads at most `MAX_LINE-1` bytes per
call. Longer physical lines are split into logical “chunks” from the
parser’s point of view.

Implications:

- Only the first 4095 characters of a long line are stored in
  `LogLine.content`.
- Line classification may fail if the marker or critical text lies
  beyond that limit.
- In practice, TST log lines are small; this only matters for
  pathological inputs.

**Mitigation:**

- Ensure TST itself does not emit lines that exceed this length.

### 2.5 Non‑Text or Binary Source Files

`extract_suite_sources` does not try to detect binary files beyond
extension checks:

- If a file with `.c`/`.cpp` etc. is binary (contains NUL or
  non‑UTF‑8), `fgets` will still produce lines until EOF or an embedded
  NUL.
- These lines are duplicated and stored, and will be HTML‑escaped.
- Syntax highlighting may misbehave visually but remains safe.

If the file is not C/C++ (extension mismatch):

- All cases in that suite get `source.available = -1` and no source
  viewer is shown.

### 2.6 Empty Suites and Logs

- No suites found (`state->suite_count == 0`): `main` prints a warning
  `"t2h: warning: no test suites found in input"` and still exits 0.
- Suites with zero cases: still rendered with header and totals (0s).

### 2.7 All Tests Skipped

If all checks in a suite are skipped:

- `tst_pass == 0`, `tst_fail == 0`, `tst_skip > 0`.
- `t2h` renders yellow styling for cases based on skip counts and
  `has_skpt`.
- Percentage calculations described in `docs/ref_manual.md:1064` treat
  skipped tests as excluded from pass/fail percentages.


## 3. Performance Characteristics

### 3.1 Tag Filtering (`tst_check_tags`)

See `tst_h_internals.md` for details.

- Time: `O(F × T × L)` where F is number of filters, T is tags per
  case, L is average tag length.
- Typical values are small enough that this is negligible compared to
  test execution.

**Potential bottleneck:** large numbers of filters with many
multi‑tagged tests. If this becomes a problem, consider caching
filter results or changing the CLI convention rather than rewiring the
algorithm.

### 3.2 Log Parsing (`parse_logs`)

- Time: O(N) where N is number of input lines.
- Per‑line cost:
  - Strip newline.
  - Small number of `strncmp` calls in `classify_line`.
  - Possible dynamic growth of arrays, but amortized O(1) per append.

**Hot spots:**

- `classify_line` is called on every line. Avoid adding expensive
  operations there.
- `strstr` calls for markers; they scan the (bounded) line.

### 3.3 Source Extraction

- Time: O(F × M) where F is number of source lines and M is average
  characters per line.
- Additional factor: number of cases, but the algorithm is careful to
  walk the file only once.

Hot paths:

- Character‑by‑character loops in `is_in_comment` and
  `is_in_string_literal`.
- Brace counting within `extract_suite_sources`.

Known optimization:

- `is_in_comment` was reworked to O(n) (`src/t2h.c:965-1020`), avoiding
  repeated rescanning for each position.

### 3.4 HTML Generation

- Time: roughly proportional to number of suites, cases, log lines,
  and source lines.
- Operations per line: formatting and HTML escaping, which are
  relatively cheap compared to parsing.

**I/O considerations:**

- HTML is written via many `fprintf`/`fputc` calls to stdout.
- On slow I/O (e.g., remote filesystems), this can become noticeable.
  Using the OS’s standard buffering is usually sufficient.


## 4. Scalability Guidelines

### 4.1 Recommended Limits

Given default constants and typical hardware:

- Suites: up to a few dozen per run.
- Cases per suite: up to several hundred.
- Log lines per suite: thousands to low tens of thousands.
- Source lines per case: a few hundred (entire file segments) work
  well; very large test functions still work, but report size grows.

### 4.2 Generating Stress Tests

To test scalability, you can:

- Use a script to generate synthetic logs that mimic TST format with:
  - Many suites.
  - Many cases per suite.
  - Many PASS/FAIL lines per case.
- Run `t2h` on these logs and inspect:
  - Runtime.
  - Peak memory usage.
  - Any warnings about truncation or limits.

### 4.3 Profiling Recommendations

- Use `valgrind --tool=massif` to profile heap usage of `t2h`.
- Use `perf` or `gprof` to examine CPU hot spots:
  - Look at time in `parse_logs`, `extract_suite_sources`, and
    `output_source_line_highlighted`.
- For TST itself, use `tstclock` around critical code sections to
  instrument test suites, as described in `docs/prog_manual.md:1059`.


## 5. Edge Case Matrix

The table below summarizes key scenarios and expected outcomes.

| Scenario                           | Expected Behavior |
|-----------------------------------|-------------------|
| No suites in log                  | `t2h`: warning, no suites rendered. Exit 0. |
| Suite without RSLT/ABRT           | Suite rendered with partial data, totals may be 0. |
| CASE without result line          | Case log lines shown; counts may not reflect reality. |
| All tests skipped                 | Totals show only SKIP; HTML uses yellow/border styles. |
| Empty `tstdata`                   | Section header and footer, no checks executed. |
| Very long log line (> MAX_LINE)   | Line truncated to 4095 chars before parsing. |
| MAX_LOG_LINES reached             | Remaining lines dropped; `suite->truncated = 1`. |
| Non‑C source file for suite       | Source extraction disabled, code tab hidden. |
| Source file not found             | All `source.available = -1`; no source views. |
| OOM in `init_state`               | `t2h` prints fatal error and exits 1. |
| OOM in suite growth               | `t2h` prints error and returns 0 from `parse_logs`. |
| OOM during source extraction      | Partial extraction for that case; `available = -1`. |

Use this matrix as a reference when debugging unexpected behavior and
when writing new tests to cover corner cases.
