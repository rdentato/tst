# TST Maintainer Playbook: Common Changes

> Audience: C maintainers of `tst.h` and `t2h.c`.
>
> This playbook provides step‑by‑step guidance for typical
> maintenance tasks: extending tags, adding markers, adjusting
> limits, and dealing with platform quirks. It assumes you have
> read `tst_h_internals.md` and `t2h_architecture.md`.

## 1. Overview

Common maintenance scenarios include:

- Evolving the **tag system** (new semantics or filters).
- Adding new **log markers** and surfacing them in HTML.
- Increasing or tuning **hard limits** (`MAX_*` constants).
- Handling **platform‑specific** issues (MSVC, C++, time units).
- Coordinating **documentation updates**.

For user‑visible semantics, cross‑check with:

- `docs/ref_manual.md` (Reference Manual).
- `docs/prog_manual.md` (Programmer's Manual).


## 2. Extending or Modifying the Tag System

### 2.1 Where the Tag System Lives

- Runtime evaluation: `tst_check_tags` in `src/tst.h:101`.
- CLI behavior: described in
  - `docs/ref_manual.md:529` (Tag‑Based Filtering).
  - `docs/prog_manual.md:610` (Tags and Selective Test Execution).
- Log listing: `--list` in `tst_main` / `tstcase__`.

### 2.2 Adding New Filter Semantics

Suppose you want to introduce new filter operators, e.g. logical
combinations like `+TAG1,+TAG2` meaning `TAG1 AND TAG2` in a single
argument.

**Checklist:**

1. **Clarify semantics first**
   - Define precisely what the new filters mean.
   - Sketch truth tables for combinations of tags and filters.

2. **Update the runtime implementation**
   - Modify `tst_check_tags` in `src/tst.h`.
   - Keep complexity roughly `O(F × T × L)`; avoid allocations.
   - Ensure unrecognized filters continue to be ignored safely.

3. **Update documentation**
   - Update tag tables and examples in:
     - `docs/ref_manual.md` (Tag‑Based Filtering).
     - `docs/prog_manual.md` (Tags and Selective Test Execution,
       Command Line Options).

4. **Add tests**
   - Write small suites that exercise:
     - Untagged tests (should keep behavior).
     - Single tags, multiple tags, and new syntax.
   - Use `--list` to verify tag strings.
   - Run with combinations of filters verifying expected cases run.

5. **Preserve backward compatibility when possible**
   - Existing semantics with `+TAG`, `-TAG`, `+*` should continue to
     work as documented, unless you deliberately break them and bump
     a visible version.

> **WARNING:** Changing default behavior for tagged tests (currently
> disabled without filters) has wide impact. If you need different
> defaults, consider adding a new flag instead (e.g. `--all-tags`).


## 3. Adding New Log Markers

You may want to introduce a new marker, for example `WARNING|` for
non‑fatal concerns.

### 3.1 Steps in `tst.h`

1. **Define the string constant**
   - Add a `tst_str_warn` or similar near `tst_str_pass`/
     `tst_str_fail` in `src/tst.h:39-55`.

2. **Add macros for emission**
   - Create a macro similar to `tstnote` or a variant of `tstcheck_`
     that emits `WARNING|` while not affecting pass/fail counts, or
     decide how it should be counted.

3. **Update documentation**
   - Add the new marker to tables in `docs/ref_manual.md:1216` and
     examples in the manuals.

### 3.2 Steps in `t2h.c`

1. **Extend the `LogLine` enum**
   - Add `LINE_WARNING` (for example) in `src/t2h.c:103-114`.

2. **Update `classify_line`** (`src/t2h.c:371-396`)
   - Detect the new marker by prefix, e.g. `"WARN|"`.

3. **Update rendering**
   - In `output_test_case`, map `LINE_WARNING` to an appropriate CSS
     class in the log viewer (e.g. `log-warning`).
   - Add CSS rules in `output_html_header` for the new class.

4. **Optional: HTML badges or stats**
   - Decide whether `WARNING` contributes to numeric summaries.
   - If yes, add counters to `TestCase` / `TestSuite` and surface them
     in HTML.

5. **Validate log format**
   - Update log format tables in `docs/ref_manual.md`.
   - Add tests that emit and parse warnings.

> **WARNING:** Keep marker prefixes (`WARN|`, `WARNING|`, etc.) stable
> once released. They effectively become part of a public log format
> contract.


## 4. Adjusting Limits and Buffers

You may need to raise or lower limits such as `MAX_CASES` or
`MAX_LOG_LINES` in `src/t2h.c:44-56`.

### 4.1 Checklist for Changing `MAX_*`

1. **Identify the constant(s)**
   - `MAX_LINE`, `MAX_CASES`, `MAX_SUITES`, `MAX_NAME`, `MAX_LOG_LINES`,
     `MAX_SOURCE_LINES`, `MAX_PATH`.

2. **Assess impact**
   - Stack usage (`MAX_SOURCE_LINES` affects
     `SourceLine temp_lines[MAX_SOURCE_LINES]`).
   - Heap usage per suite and globally.

3. **Update any derived logic**
   - Comments or help text referring to hard limits.
   - Warnings printed when limits are reached (e.g. warning messages in
     `grow_cases` and `grow_log_lines`).

4. **Run scalability tests**
   - Generate large synthetic logs (many suites, many cases, many
     lines) and run `t2h` to observe memory and CPU usage.
   - Use tools like Valgrind or `time` to measure impact.

5. **Update documentation where limits are mentioned**
   - E.g., warnings about log truncation in `docs/prog_manual.md:1007`.

> **WARNING:** Raising `MAX_SOURCE_LINES` beyond a few tens of
> thousands may cause stack overflows on some platforms. Consider a
> heap‑allocated structure if you need unbounded source extraction.


## 5. Platform and Compiler Considerations

### 5.1 MSVC Warnings

At the top of `src/tst.h:8-15`:

```c
#ifdef _MSC_VER
  #pragma warning(disable:4100)
  #pragma warning(disable:4189)
  #pragma warning(disable:4152)
  #pragma warning(disable:4244)
  #pragma warning(disable:4459)
  #pragma warning(disable:4996)
#endif
```

These pragmas silence common MSVC warnings related to:

- Unused parameters.
- Unused variables.
- Pointer conversions.
- Narrowing conversions.
- Shadowed declarations.
- Deprecated C functions.

If you add new constructs that trigger warnings, prefer fixing the
code for clarity instead of adding more pragma disables.

### 5.2 C++ Compatibility

`tst.h` wraps definitions in `extern "C"` guards
(`src/tst.h:18-20, 292-294`):

```c
#ifdef __cplusplus
extern "C" {
#endif
...
#ifdef __cplusplus
}
#endif
```

This allows including `tst.h` in C++ tests without name mangling. When
adding new non‑macro functions, place them inside these guards.

### 5.3 `CLOCKS_PER_SEC` and Units

`tst_main` picks an appropriate clock unit (`n`, `u`, `m`) based on
`CLOCKS_PER_SEC` (`src/tst.h:167-169`):

- > 1,000,000 → nanoseconds (`n`).
- > 1,000 → microseconds (`u`).
- Else → milliseconds (`m`).

`tst_str_clck` uses `%ld %ss` (`src/tst.h:49`), so units are printed as
`ns`, `us`, or `ms` in logs.

If porting to a platform with unusual `CLOCKS_PER_SEC`, make sure this
heuristic still produces sensible units.

### 5.4 Path and Filesystem Considerations

`find_source_file` in `src/t2h.c:895-935` tries multiple paths:

- Current directory.
- Directory of log file.
- `test/`, `src/`, `tests/`, `./` prefixes.

On Windows, prefer POSIX‑style forward slashes in logs; backslashes are
not currently special‑cased.


## 6. Documentation and Coordination

Any change that affects **user‑visible behavior** requires a small
coordination checklist.

### 6.1 When to Update Which Docs

- `docs/ref_manual.md` (Reference Manual):
  - Changes to macro semantics (`tstcase`, `tstskipif`, `tstclock`).
  - New markers, output format, or tag rules.
  - CLI options for test executables or `t2h`.

- `docs/prog_manual.md` (Programmer's Manual):
  - New recommended patterns or best practices.
  - Changes to example code and workflows.

- `docs/design/*.md`:
  - Architectural decisions, parsing strategies, data structures.
  - Long‑term design rationale.

- `docs/maintainer/*.md`:
  - Deep internals, change procedures, invariants.

### 6.2 Suggested Workflow for Behavior Changes

1. Implement code changes in a feature branch.
2. Update or add tests to cover the new behavior.
3. Update `docs/ref_manual.md` and `docs/prog_manual.md` for external
   behavior.
4. Update `docs/maintainer/*.md` if internals or invariants changed.
5. Run the full test suite (`make runtest`, `cd src && make test-t2h`).
6. Manually inspect logs and generated HTML for at least one run.


## 7. Example Scenarios

### 7.1 Adding a WARNING Marker: Step‑by‑Step

1. **In `tst.h`**:
   - Add `const char *tst_str_warn = "WARN|  ";` near existing
     marker strings.
   - Implement a macro, e.g.:
     ```c
     #define tstwarn(...) (tst_prtln(tst_str_warn), tst_prtf(" " __VA_ARGS__))
     ```

2. **In `t2h.c`**:
   - Extend `enum` with `LINE_WARN`.
   - In `classify_line`, detect `"WARN|"` and return `LINE_WARN`.
   - In `output_test_case`, map `LINE_WARN` to a `.log-warn` CSS class.
   - In `output_html_header`, add CSS for `.log-warn`.

3. **Docs**:
   - Add `WARN|` row to log marker table (`docs/ref_manual.md`).
   - Show an example of a warning in the output format section.

4. **Tests**:
   - Create a small test emitting `tstwarn`.
   - Verify log shows `WARN|` lines.
   - Run `t2h` and check the resulting HTML shows a distinct style.

### 7.2 Raising `MAX_LOG_LINES` to 100,000

1. Change `#define MAX_LOG_LINES 20000` to `100000`.
2. Rebuild `t2h`.
3. Generate a synthetic log with ~90k lines and run `t2h`.
4. Inspect memory usage; ensure no OOM or extreme slowdown.
5. Confirm truncation warnings still behave correctly when hitting
   the new limit.
6. Update any docs that mention 20k as a limit.


## 8. Quick Playbook Checklist

Before merging changes that touch TST internals:

- [ ] Have you identified which public behaviors are affected?
- [ ] Have you updated `docs/ref_manual.md` and/or `docs/prog_manual.md`?
- [ ] Do existing log format examples still apply or need revision?
- [ ] Have you run `make runtest` and `cd src && make test-t2h`?
- [ ] For tag or marker changes, have you verified `t2h` still parses
      logs correctly?
- [ ] For limit changes, have you tested with large inputs?
- [ ] Have you checked behavior on at least one non‑Linux compiler if
      applicable (e.g. MSVC, clang)?
