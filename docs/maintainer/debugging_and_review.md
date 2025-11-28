# TST Maintainer Guide: Debugging & Code Review

> Audience: C maintainers working on `tst.h` and `t2h.c`.
>
> This guide covers practical debugging techniques for macro‑heavy
> code and complex parsing, and a checklist for reviewing changes.


## 1. Debugging Macro‑Heavy Code (`tst.h`)

`tst.h` relies heavily on macros and nested `for` loops. Understanding
what the compiler sees is essential when debugging.

### 1.1 Using `gcc -E` to Inspect Expansions

To see expanded code for a particular test file:

```bash
gcc -E -I. -o t_tst01.i test/t_tst01.c
```

- `t_tst01.i` will contain the preprocessed source with all includes
  and macros expanded.
- Search for `tstcase(` or `tstsuite(` and inspect the resulting `for`
  loops.

Tips:

- Use `rg` or your editor to search for `tst__run` and expand from
  there.
- Compare multiple versions to understand how a new macro changes
  expansion.

### 1.2 Inserting Temporary Debug Prints

Because macros are in a header, temporary debug prints will affect all
files including `tst.h`.

Examples:

- Trace tag decisions in `tst_check_tags`:
  ```c
  fprintf(stderr, "[tst_check_tags] tags_str='%s'\n", tags_str);
  ```

- Debug case start/end in `tstcase__`:
  ```c
  tst_prtln("DEBUG: entering case");
  ```

Remember to **remove or `#if 0`** these statements before committing.

### 1.3 GDB Tips for Macro Code

In GDB, breakpoints on macro names don’t work directly, but you can:

- Break on `main` of a test binary.
- Use `info line` to locate expanded lines of interest.
- Set breakpoints on the underlying functions, e.g.:
  - `tst_time` (if made non‑inline for debugging).
  - `tst_check_tags`.

When stepping, GDB may jump around due to macro expansions; use
`nexti`/`stepi` sparingly and rely on well‑placed breakpoints.


## 2. Debugging `t2h`

### 2.1 Logging Parser Decisions

`parse_logs` is a central place to add temporary logging.

Examples:

- Log each recognized marker:
  ```c
  if (strstr(line, "SUIT /")) {
      fprintf(stderr, "[t2h] SUIT: %s\n", line);
      ...
  }
  ```

- Log case transitions:
  ```c
  if (strstr(line, "CASE,--")) {
      fprintf(stderr, "[t2h] CASE: %s\n", line);
      ...
  }
  ```

- Log source extraction failures in `extract_suite_sources`:
  ```c
  if (!fp) {
      fprintf(stderr, "[t2h] cannot find source for '%s'\n", suite->filename);
      ...
  }
  ```

Remove or guard logs with a debug macro (e.g. `#ifdef T2H_DEBUG`) when
not needed.

### 2.2 Investigating Missing Suites or Cases

If `t2h` reports "no test suites found" or generates an HTML file
without expected cases:

1. Check the input log:
   - Ensure it contains `SUIT /` lines (not truncated by a pipeline or
     earlier tool).
   - Ensure markers match those in `tst_str_*` (`SUIT /`, `CASE,--`,
     `RSLT \\`, `ABRT \\`).

2. Run `t2h` with debug logging enabled (see above) to confirm
   detection of SUIT/CASE markers.

3. Use a debugger:
   - Break on `parse_suit_line` and `parse_case_line`.
   - Inspect `ProgramState.suites[i]` and `suites[i].cases[j]`.

### 2.3 Debugging Source Extraction Problems

If source code is missing from the HTML report for some cases:

1. Confirm the SUIT line includes the correct filename. Example:
   ```
   ----- SUIT / t_tst01.c "Suite Title" 2025-11-27 ...
   ```

2. Check that the file exists and is readable in one of the locations
   `find_source_file` tries.

3. In a debugger or with logging:
   - Inspect `suite->filename` and `tc->line_number`.
   - Log `current_case_idx`, `brace_count`, `found_opening` inside
     `extract_suite_sources`.

4. Validate that the case’s function body indeed starts near the
   logged line number and uses standard brace style.

### 2.4 HTML Validation and Inspection

To validate generated HTML:

1. Save output:
   ```bash
   ./t_tst01 | src/t2h > /tmp/report.html
   ```

2. Use browser dev tools:
   - Inspect DOM for `.suite-header`, `.test-case`, `.log-line`, and
     `.src-*` elements.
   - Check that IDs (`log-case-*`, `src-case-*`) match button handlers
     in JS.

3. Optionally run through an HTML validator to catch stray tags.


## 3. Tool‑Specific Tips

### 3.1 GDB / LLDB

Useful breakpoints in `t2h`:

- `parse_logs`
- `parse_suit_line`
- `parse_case_line`
- `extract_suite_sources`
- `generate_html`

In TST:

- If you temporarily wrap parts of `tst.h` in non‑inline functions for
  debugging, you can break on them directly.

### 3.2 Valgrind / Sanitizers

To check for leaks in `t2h`:

```bash
valgrind --leak-check=full src/t2h test/all_out.log > /dev/null
```

- Ensure `free_state` covers all ownership paths.
- Watch for mismatched `malloc`/`free` in new code.

For runtime errors:

- Compile with `-fsanitize=address,undefined` where supported.
- Run both test executables and `t2h` under sanitizers.


## 4. Common Issue Checklists

### 4.1 "Test Not Running"

Symptoms:

- Case does not appear in `--list` output.
- Case appears in `--list` but not in actual execution.

Steps:

1. Confirm `tstsuite` vs `tst_suite`.
2. Run with `--list`:
   ```bash
   ./test_program --list
   ```
   - If not listed: check `tstcase` location and compilation.

3. Check tags and filters:
   - Ensure case is not tagged only with filters you’re not enabling.
   - Try `./test_program +*` to enable all tagged tests.

4. Inspect `tstcase` declaration:
   - Confirm the macro is invoked and not gated by `#if 0`.

5. If still unclear, temporarily log from `tst_check_tags`.

### 4.2 "Wrong Section Name Displayed"

Symptoms:

- Section description in log differs from expected format.

Steps:

1. Inspect the `tstsection` call site:
   - Verify format string and arguments.
2. Remember that `tstsection` uses `printf`‑style formatting.
3. Check that the number and types of arguments match the format
   string.
4. Verify that `tstsection` is not in a macro that changes `__LINE__`
   or arguments unexpectedly.

### 4.3 "`t2h` Produces Empty HTML"

Symptoms:

- HTML skeleton without suites or cases.

Steps:

1. Check stderr for `t2h` warnings or errors (e.g. cannot open file,
   no test suites found).
2. Confirm input log actually contains SUIT markers.
3. If using a pipeline, ensure earlier commands don’t filter out these
   markers.
4. Run `t2h` on a known‑good example log from `examples/` to verify
   that the tool itself is functioning.

### 4.4 "Source Code Missing in Report"

Steps:

1. Confirm suite’s filename is correct and matches a real file.
2. Confirm `extract_suite_sources` is being called; add temporary logs.
3. Check `tc->source.available`:
   - `-1` means failure (file not found, non‑C file, or OOM).
4. Resolve path issues:
   - Consider running `t2h` from project root so `test/` and `src/`
     paths make sense.


## 5. Code Review Checklist

Use this as a pre‑merge checklist when reviewing changes.

### 5.1 General

- [ ] Does the change have a clear motivation and scope?
- [ ] Are unrelated refactors avoided or isolated?
- [ ] Are there tests or examples exercising the new behavior?

### 5.2 `tst.h` Changes

- [ ] Public macros (`tstsuite`, `tstcase`, `tstcheck`, `tstexpect`,
      `tstassert`, `tstskipif`, `tstclock`, `tstnote`, etc.) retain
      their documented behavior unless this is an intentional, documented
      breaking change.
- [ ] New macros:
  - [ ] Avoid multiple evaluation of arguments.
  - [ ] Use `#expr` for expression logging where appropriate.
  - [ ] Use `__VA_ARGS__` safely for optional messages.
- [ ] Tag semantics (`tst_check_tags`) still match documentation.
- [ ] Log markers (`tst_str_*`) remain compatible with `t2h` and
      manuals.
- [ ] Any new static/global state is justified and kept minimal.

### 5.3 `t2h.c` Changes

- [ ] Any changes to log parsing preserve SUIT/RSLT/ABRT/CASE semantics.
- [ ] New markers or types are fully wired:
  - [ ] Enum entries.
  - [ ] `classify_line` logic.
  - [ ] HTML/CSS rendering.
- [ ] Memory allocations have clear ownership and are freed in
      `free_suite_contents` / `free_state`.
- [ ] Hard limits (`MAX_*`) remain reasonable and documented.
- [ ] Security: All user‑controlled strings are escaped via
      `html_escape` or similar.

### 5.4 Testing and Platforms

- [ ] `make` from project root succeeds.
- [ ] `make runtest` (or equivalent) passes.
- [ ] `cd src && make test-t2h` passes (if available).
- [ ] At least one HTML report was manually inspected in a browser.
- [ ] For changes touching portability:
  - [ ] Code compiles with `gcc` and `clang`.
  - [ ] If applicable, code compiles with MSVC (or at least has no
        obvious MSVC‑specific problems).
  - [ ] C++ compilation was tested if new non‑macro symbols were added
        to `tst.h`.

### 5.5 Documentation

- [ ] Any user‑visible behavior changes are reflected in:
  - [ ] `docs/ref_manual.md`.
  - [ ] `docs/prog_manual.md`.
- [ ] Deep internal changes are described or at least referenced in
      `docs/maintainer/*.md`.
- [ ] Examples in docs still compile and match output.

Use this guide as a living document: when new failure modes or
maintenance patterns appear, extend these sections to capture the
institutional knowledge.
