//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT
//
//  t2h - TST log to HTML converter
//
//  Usage: t2h [OPTIONS] [file1.log file2.log ...]
//         t2h < input.log > output.html
//
//  Options:
//    -h, --help     Show this help message
//    -v, --version  Show version information
//
//  OVERVIEW:
//  Converter from TST test framework logs to interactive HTML
//  dashboards. Includes source code extraction, multi-suite support, and robust
//  error handling for CI/CD integration.
//
//  FEATURES:
//  - Single and multi-file input modes (stdin or file paths)
//  - Multi-suite log parsing (multiple SUIT markers per file)
//  - Source code extraction with syntax highlighting (FUNC-006)
//  - Interactive HTML with expandable test cases
//  - SVG pie charts and progress bars
//  - Comprehensive error handling
//
//  ARCHITECTURE:
//  Based on design docs in docs/design/:
//  - Single-pass parsing with state machine (03_parsing.md)
//  - goto cleanup pattern for error handling (01_architecture.md)
//  - Mixed static/dynamic allocation (02_data_structures.md)
//  - Source file caching (04_source_extraction.md)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <limits.h>


#define VERSION "1.0.0"

// Configuration constants
// MAINTENANCE NOTE: These limits are designed for typical test suites.
// If increasing limits, verify:
//   - Stack usage (e.g., temp_lines[MAX_SOURCE_LINES] in extract_suite_sources)
//   - Memory allocation error paths (all grow_* functions)
//   - Performance impact (larger initial allocations)
#define MAX_LINE 4096           // Maximum length of a single log/source line
#define MAX_CASES 512           // Hard limit per suite (see grow_cases)
#define MAX_SUITES 64           // Hard limit per batch (see parse_logs)
#define MAX_NAME 512            // Test case/suite name buffer
#define MAX_LOG_LINES 20000     // Hard limit per suite (see grow_log_lines)
#define MAX_SOURCE_LINES 10000  // Max lines per test case (stack-allocated buffer)
#define MAX_PATH 1024           // File path buffer size

// ============================================================================
// Core Data Structures (per docs/design/02_data_structures.md)
// ============================================================================

// Source code line
// OWNERSHIP: content pointer owned by SourceLine, freed in free_suite_contents()
// LIFECYCLE: Created in extract_suite_sources via strdup, transferred to TestCase
typedef struct {
    char *content;      // Dynamically allocated line content (via strdup)
    int linenum;        // Original line number from source file
} SourceLine;

// Source code for a test case
// LIFECYCLE: Allocated during parse_logs, populated in extract_suite_sources,
//            freed in free_suite_contents via free_state
// OWNERSHIP: lines array and each line's content owned by this struct
typedef struct {
    char filepath[MAX_PATH];     // Path to source file
    SourceLine *lines;           // Dynamically allocated array (calloc)
    int line_count;              // Number of lines stored
    int start_line;              // First line number (from CASE marker)
    int end_line;                // Last line number (from case close)
    int available;               // 0=not loaded, 1=loaded, -1=error
} SourceCode;

// Single test case
// OWNERSHIP: Owned by TestSuite.cases array
// INVARIANT: log_start <= log_end (if >= 0), both are indices into suite->log_lines
typedef struct {
    char name[MAX_NAME];         // Test case description
    int pass;                    // Count of PASS checks
    int fail;                    // Count of FAIL checks
    int skip;                    // Count of SKIP checks
    int has_skpt;                // Flag: has SKPT sections (for border color only)
    int log_start;               // Index of first log line (or -1 if none)
    int log_end;                 // Index of last log line (or -1 if none)
    int line_number;             // Line number where CASE appears (for source extraction)
    SourceCode source;           // Source code for this case (FUNC-006)
} TestCase;

// Log line with metadata
// OWNERSHIP: Owned by TestSuite.log_lines array (statically allocated content)
typedef struct {
    char content[MAX_LINE];      // Full line content (stack-allocated)
    int linenum;                 // Line number from log
    enum {
        LINE_PASS,               // PASS| marker
        LINE_FAIL,               // FAIL| marker
        LINE_SKIP,               // SKIP|/SKPT| marker
        LINE_NOTE,               // NOTE: marker
        LINE_CASE,               // CASE,-- marker
        LINE_SECTION,            // SCTN|,-- marker
        LINE_RESULT,             // Result summary line (contains FAIL|PASS|SKIP)
        LINE_CLOCK,              // CLCK: marker
        LINE_ABORT,              // ABRT marker
        LINE_OTHER               // Unrecognized line type
    } type;                      // Line type for syntax highlighting
} LogLine;

// Complete test suite
// LIFECYCLE: Allocated in init_state, populated in parse_logs, freed in free_state
// OWNERSHIP: Owns cases and log_lines arrays (and all nested allocations)
// GROWTH STRATEGY: Exponential (2x) via grow_cases and grow_log_lines
typedef struct {
    char title[MAX_NAME];        // Suite description
    char filename[MAX_PATH];     // Source file path (e.g., t_tst00.c)
    char start_time[64];         // SUIT timestamp
    char end_time[64];           // RSLT/ABRT timestamp
    int total_pass;              // Total PASS count
    int total_fail;              // Total FAIL count
    int total_skip;              // Total SKIP count
    TestCase *cases;             // Dynamically allocated array (grows exponentially)
    int case_count;              // Number of test cases
    int case_capacity;           // Allocated capacity (for growth detection)
    LogLine *log_lines;          // Dynamically allocated array (grows exponentially)
    int log_line_count;          // Number of log lines
    int log_capacity;            // Allocated capacity (for growth detection)
    int truncated;               // Flag: log was truncated (hit MAX_LOG_LINES)
    int aborted;                 // Flag: suite was aborted (ABRT instead of RSLT)
} TestSuite;

// Program state for multi-suite processing
// LIFECYCLE: Created in main via init_state, freed at exit via free_state
// OWNERSHIP: Top-level owner of all program data (owns suites array)
// THREAD-SAFETY: Not thread-safe (single-threaded design)
typedef struct {
    TestSuite *suites;           // Array of suites (grows exponentially)
    int suite_count;             // Number of suites
    int suite_capacity;          // Allocated capacity
    int total_pass;              // Aggregate across all suites
    int total_fail;              // Aggregate across all suites
    int total_skip;              // Aggregate across all suites
    char batch_start[64];        // STIME marker (optional batch metadata)
    char batch_end[64];          // ETIME marker (optional batch metadata)
    int dark_theme;              // 0=light, 1=dark (default=1)
} ProgramState;

// ============================================================================
// Utility Functions
// ============================================================================

// Safe string copy with bounds checking
static void safe_strcpy(char *dest, const char *src, size_t dest_size) {
    if (!dest || !src || dest_size == 0) return;
    size_t len = strlen(src);
    if (len >= dest_size) len = dest_size - 1;
    memcpy(dest, src, len);
    dest[len] = '\0';
}

// Trim leading and trailing whitespace in place
static char *trim(char *str) {
    if (!str) return NULL;
    
    // Trim leading
    while (*str && isspace((unsigned char)*str)) str++;
    
    if (*str == '\0') return str;
    
    // Trim trailing
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    
    return str;
}

// Extract line number from TST log line (format: "   123 PASS| ...")
static int extract_line_number(const char *line) {
    if (!line) return 0;
    
    // Skip leading spaces
    while (*line && isspace((unsigned char)*line)) line++;
    
    // Parse number
    char *endptr;
    long num = strtol(line, &endptr, 10);
    
    // Verify we parsed a number and didn't overflow
    if (endptr == line || num < 0 || num > INT_MAX) return 0;
    
    return (int)num;
}

// Extract content from TST log line, removing line number but preserving indentation
// Input:  "    8     `--- 3 FAIL | 1 PASS | 0 SKIP"
// Output: "     `--- 3 FAIL | 1 PASS | 0 SKIP" (5 spaces preserved for tree structure)
static const char *extract_log_content(const char *line) {
    if (!line) return line;
    
    
    // Skip leading spaces
    while (*line && isspace((unsigned char)*line)) line++;
    
    // Skip line number
    while (*line && isdigit((unsigned char)*line)) line++;
    
    // Now we're at the content part - preserve all remaining spaces/structure
    // These spaces are part of the ASCII art tree structure
    return line;
}

// HTML escape a string to prevent XSS and markup corruption
// Escapes: < > & " '
static void html_escape(FILE *out, const char *s) {
    if (!out || !s) return;
    
    for (const char *p = s; *p; p++) {
        switch (*p) {
            case '<':  fprintf(out, "&lt;"); break;
            case '>':  fprintf(out, "&gt;"); break;
            case '&':  fprintf(out, "&amp;"); break;
            case '"':  fprintf(out, "&quot;"); break;
            case '\'': fprintf(out, "&#39;"); break;
            default:   fputc(*p, out); break;
        }
    }
}

// ============================================================================
// Memory Management Functions (02_data_structures.md)
// ============================================================================
// CRITICAL: Proper cleanup order is essential to avoid leaks and double-frees
// Order: free_suite_contents() → free(suites) → free(state)
// See NO_LEAK.md for detailed ownership and lifecycle documentation

// Free a test suite's internal resources (not the suite itself)
// PRECONDITION: suite != NULL
// POSTCONDITION: All nested allocations freed, pointers set to NULL
// OWNERSHIP: Frees suite->cases, suite->log_lines, and all nested content
static void free_suite_contents(TestSuite *suite) {
    if (!suite) return;
    
    // Free source code in each test case
    for (int i = 0; i < suite->case_count; i++) {
        TestCase *tc = &suite->cases[i];
        if (tc->source.lines) {
            // Free each line's content (allocated via strdup)
            for (int j = 0; j < tc->source.line_count; j++) {
                free(tc->source.lines[j].content);
            }
            free(tc->source.lines);
        }
    }
    
    free(suite->cases);
    free(suite->log_lines);
}

// Initialize program state
// RETURNS: Newly allocated ProgramState, or NULL on allocation failure
// POSTCONDITION: suites array allocated with initial capacity (16 suites)
// CALLER RESPONSIBILITY: Must call free_state() to avoid leak
static ProgramState *init_state(void) {
    ProgramState *state = calloc(1, sizeof(ProgramState));
    if (!state) return NULL;
    
    state->suite_capacity = 16;
    state->suites = calloc(state->suite_capacity, sizeof(TestSuite));
    
    if (!state->suites) {
        free(state);
        return NULL;
    }
    
    state->dark_theme = 1;  // Default to dark theme
    
    return state;
}

// Free entire program state
// PRECONDITION: state != NULL (but function is NULL-safe)
// POSTCONDITION: All memory freed, state pointer invalid
// CALL THIS: At program exit or on fatal error
static void free_state(ProgramState *state) {
    if (!state) return;
    
    // Free contents of each suite (but not the suite structures themselves)
    for (int i = 0; i < state->suite_count; i++) {
        free_suite_contents(&state->suites[i]);
    }
    
    free(state->suites);
    free(state);
}

// Grow test case array if needed
// ALGORITHM: Exponential growth (2x) for amortized O(1) append
// RETURNS: 1 on success, 0 on failure (either at limit or OOM)
// SIDE EFFECT: May print warning to stderr if MAX_CASES reached
// POSTCONDITION: New memory is zeroed (critical for TestCase initialization)
static int grow_cases(TestSuite *suite) {
    if (suite->case_count < suite->case_capacity) return 1;
    
    // Check if we've hit the hard limit
    if (suite->case_capacity >= MAX_CASES) {
        fprintf(stderr, "t2h: warning: reached MAX_CASES limit (%d), skipping additional cases\n", MAX_CASES);
        return 0;
    }
    
    int new_cap = suite->case_capacity * 2;
    if (new_cap > MAX_CASES) new_cap = MAX_CASES;
    
    TestCase *new_cases = realloc(suite->cases, new_cap * sizeof(TestCase));
    if (!new_cases) return 0;
    
    suite->cases = new_cases;
    suite->case_capacity = new_cap;
    
    // Zero out new memory (CRITICAL: ensures clean TestCase state)
    memset(suite->cases + suite->case_count, 0, 
           (new_cap - suite->case_count) * sizeof(TestCase));
    
    return 1;
}

// Grow log lines array if needed
// ALGORITHM: Exponential growth (2x) for amortized O(1) append
// RETURNS: 1 on success, 0 on failure (sets truncated flag)
// SIDE EFFECT: Sets suite->truncated = 1 on failure
// POSTCONDITION: New memory is zeroed for safety
static int grow_log_lines(TestSuite *suite) {
    if (suite->log_line_count < suite->log_capacity) return 1;
    
    // Check if we've hit the hard limit
    if (suite->log_capacity >= MAX_LOG_LINES) {
        suite->truncated = 1;
        return 0;
    }
    
    int new_cap = suite->log_capacity * 2;
    if (new_cap > MAX_LOG_LINES) new_cap = MAX_LOG_LINES;
    
    LogLine *new_lines = realloc(suite->log_lines, new_cap * sizeof(LogLine));
    if (!new_lines) {
        suite->truncated = 1;
        return 0;
    }
    
    suite->log_lines = new_lines;
    suite->log_capacity = new_cap;
    
    // Zero out new memory for safety and consistency with grow_cases
    memset(suite->log_lines + suite->log_line_count, 0,
           (new_cap - suite->log_line_count) * sizeof(LogLine));
    
    return 1;
}

// ============================================================================
// Line Type Classification (03_parsing.md)
// ============================================================================

static int classify_line(const char *line) {
    if (!line) return LINE_OTHER;
    
    // Skip line number prefix
    const char *p = line;
    while (*p && isspace((unsigned char)*p)) p++;
    while (*p && isdigit((unsigned char)*p)) p++;
    while (*p && isspace((unsigned char)*p)) p++;
    
    // Check for markers
    if (strncmp(p, "PASS|", 5) == 0) return LINE_PASS;
    if (strncmp(p, "FAIL|", 5) == 0) return LINE_FAIL;
    if (strncmp(p, "SKIP|", 5) == 0 || strncmp(p, "SKPT|", 5) == 0) return LINE_SKIP;
    if (strncmp(p, "NOTE:", 5) == 0) return LINE_NOTE;
    if (strncmp(p, "CASE,--", 7) == 0) return LINE_CASE;
    if (strncmp(p, "SCTN|,--", 8) == 0) return LINE_SECTION;
    if (strncmp(p, "CLCK:", 5) == 0) return LINE_CLOCK;
    if (strncmp(p, "ABRT", 4) == 0) return LINE_ABORT;
    
    // Check for result lines (e.g., "    `--- 2 FAIL | 3 PASS | 1 SKIP")
    if (strstr(p, "FAIL") && strstr(p, "PASS") && strstr(p, "SKIP")) {
        return LINE_RESULT;
    }
    
    return LINE_OTHER;
}

// ============================================================================
// Forward Declarations
// ============================================================================

static int compare_testcase_by_line(const void *a, const void *b);
static int parse_logs(ProgramState *state, FILE *input, const char *filename);
static int generate_html(ProgramState *state, FILE *output);
static void extract_suite_sources(TestSuite *suite, const char *log_path);
static void show_help(void);
static void show_version(void);

// ============================================================================
// Main Entry Point
// ============================================================================
// PROGRAM FLOW:
//   1. Parse arguments (help, version, theme flags)
//   2. Initialize state (allocates ProgramState)
//   3. Parse logs (stdin or files) → builds suite data
//   4. Generate HTML (writes to stdout)
//   5. Cleanup (free_state)
//
// EXIT CODES:
//   0: Success
//   1: Error (allocation failure, parse error, I/O error)
//
// MEMORY: All allocations freed via goto cleanup pattern

int main(int argc, char *argv[]) {
    ProgramState *state = NULL;
    int exit_code = 0;
    
    // Parse command-line options (help and version exit immediately)
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            show_help();
            return 0;
        }
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
            show_version();
            return 0;
        }
    }
    
    // Initialize program state (allocates suites array)
    state = init_state();
    if (!state) {
        fprintf(stderr, "t2h: fatal: failed to allocate memory\n");
        return 1;
    }
    
    // Process theme flag and determine file argument offset
    // INVARIANT: arg_offset points to first filename (or past argc if stdin mode)
    int arg_offset = 1;
    if (argc > 1) {
        if (strcmp(argv[1], "--light") == 0 || strcmp(argv[1], "--light-theme") == 0) {
            state->dark_theme = 0;  // Override default to light theme
            arg_offset = 2;         // Skip flag when processing files
        } else if (strcmp(argv[1], "--dark") == 0 || strcmp(argv[1], "--dark-theme") == 0) {
            state->dark_theme = 1;  // Keep dark theme (explicit override)
            arg_offset = 2;         // Skip flag when processing files
        }
    }
    
    // Mode selection: stdin (no args after flags) or file inputs
    if (argc <= arg_offset) {
        // STDIN MODE: Read from pipe or terminal
        if (!parse_logs(state, stdin, "<stdin>")) {
            fprintf(stderr, "t2h: error: failed to parse input from stdin\n");
            exit_code = 1;
            goto cleanup;
        }
    } else {
        // FILE MODE: Process each file argument (supports multiple logs)
        for (int i = arg_offset; i < argc; i++) {
            FILE *fp = fopen(argv[i], "r");
            if (!fp) {
                fprintf(stderr, "t2h: error: cannot open '%s': %s\n", 
                        argv[i], strerror(errno));
                exit_code = 1;
                continue;  // Try next file
            }
            
            if (!parse_logs(state, fp, argv[i])) {
                fprintf(stderr, "t2h: error: failed to parse '%s'\n", argv[i]);
                exit_code = 1;
            }
            
            fclose(fp);
        }
    }
    
    // Generate HTML output
    if (state->suite_count > 0) {
        if (!generate_html(state, stdout)) {
            fprintf(stderr, "t2h: error: failed to generate HTML output\n");
            exit_code = 1;
        }
    } else {
        fprintf(stderr, "t2h: warning: no test suites found in input\n");
    }
    
cleanup:
    free_state(state);
    return exit_code;
}

// ============================================================================
// Help and Version
// ============================================================================

static void show_help(void) {
    printf("t2h - TST log to HTML converter v%s\n\n", VERSION);
    printf("Usage: t2h [OPTIONS] [file1.log file2.log ...]\n");
    printf("       t2h < input.log > output.html\n\n");
    printf("Options:\n");
    printf("  -h, --help         Show this help message\n");
    printf("  -v, --version      Show version information\n");
    printf("  --light            Generate report with light theme\n");
    printf("  --light-theme      Same as --light\n");
    printf("  --dark             Generate report with dark theme (default)\n");
    printf("  --dark-theme       Same as --dark\n\n");
    printf("Description:\n");
    printf("  Converts TST test framework log files to interactive HTML reports.\n");
    printf("  Supports single and multi-suite logs with source code extraction.\n");
    printf("  Default theme is dark; use --light to override.\n\n");
    printf("Examples:\n");
    printf("  t2h test.log > report.html\n");
    printf("  t2h --light suite1.log suite2.log > consolidated.html\n");
    printf("  ./test_runner | t2h > results.html\n");
}

static void show_version(void) {
    printf("t2h version %s\n", VERSION);
    printf("Copyright © 2025 Remo Dentato\n");
    printf("License: MIT\n");
}

// ============================================================================
// Parsing Implementation (03_parsing.md)
// ============================================================================

// Parse SUIT line: "----- SUIT / t_test.c "Title" 2025-11-16 12:26:45"
static int parse_suit_line(const char *line, TestSuite *suite) {
    const char *p = strstr(line, "SUIT /");
    if (!p) return 0;
    
    p += 6;  // Skip "SUIT /"
    while (*p && isspace((unsigned char)*p)) p++;
    
    // Extract filename (until next space or quote)
    const char *file_start = p;
    while (*p && !isspace((unsigned char)*p) && *p != '"') p++;
    int file_len = p - file_start;
    if (file_len > 0 && file_len < MAX_PATH) {
        memcpy(suite->filename, file_start, file_len);
        suite->filename[file_len] = '\0';
    }
    
    // Find title (between quotes)
    const char *title_start = strchr(p, '"');
    if (title_start) {
        title_start++;
        const char *title_end = strchr(title_start, '"');
        if (title_end) {
            int title_len = title_end - title_start;
            if (title_len > 0 && title_len < MAX_NAME) {
                memcpy(suite->title, title_start, title_len);
                suite->title[title_len] = '\0';
            }
            
            // Extract timestamp (after closing quote)
            p = title_end + 1;
            while (*p && isspace((unsigned char)*p)) p++;
            safe_strcpy(suite->start_time, p, sizeof(suite->start_time));
            trim(suite->start_time);
        }
    }
    
    return 1;
}

// Parse RSLT line: "^^^^^ RSLT \ 3 FAIL | 3 PASS | 2 SKIP 2025-11-16 12:26:45"
static int parse_rslt_line(const char *line, TestSuite *suite) {
    const char *p = strstr(line, "RSLT \\");
    if (!p) return 0;
    
    p += 6;  // Skip "RSLT \"
    
    // Parse counts: N FAIL | M PASS | K SKIP
    int fail = 0, pass = 0, skip = 0;
    if (sscanf(p, "%d FAIL | %d PASS | %d SKIP", &fail, &pass, &skip) == 3) {
        suite->total_fail = fail;
        suite->total_pass = pass;
        suite->total_skip = skip;
    }
    
    // Extract timestamp (after counts)
    const char *time_start = strstr(p, "SKIP");
    if (time_start) {
        time_start += 4;
        while (*time_start && isspace((unsigned char)*time_start)) time_start++;
        safe_strcpy(suite->end_time, time_start, sizeof(suite->end_time));
        trim(suite->end_time);
    }
    
    return 1;
}

// Parse ABRT line: "^^^^^ ABRT \ 1 FAIL | 2 PASS | 0 SKIP 2025-11-23 23:57:19"
static int parse_abrt_line(const char *line, TestSuite *suite) {
    const char *p = strstr(line, "ABRT \\");
    if (!p) return 0;
    
    p += 6;  // Skip "ABRT \"
    
    // Parse counts: N FAIL | M PASS | K SKIP
    int fail = 0, pass = 0, skip = 0;
    if (sscanf(p, "%d FAIL | %d PASS | %d SKIP", &fail, &pass, &skip) == 3) {
        suite->total_fail = fail;
        suite->total_pass = pass;
        suite->total_skip = skip;
    }
    
    // Extract timestamp (after counts)
    const char *time_start = strstr(p, "SKIP");
    if (time_start) {
        time_start += 4;
        while (*time_start && isspace((unsigned char)*time_start)) time_start++;
        safe_strcpy(suite->end_time, time_start, sizeof(suite->end_time));
        trim(suite->end_time);
    }
    
    // Mark suite as aborted
    suite->aborted = 1;
    
    return 1;
}

// Parse CASE line: "    8 CASE,-- Equality Checks 1, 1"
static int parse_case_line(const char *line, TestCase *tc) {
    const char *p = strstr(line, "CASE,--");
    if (!p) return 0;
    
    tc->line_number = extract_line_number(line);
    
    p += 7;  // Skip "CASE,--"
    while (*p && isspace((unsigned char)*p)) p++;
    
    safe_strcpy(tc->name, p, sizeof(tc->name));
    trim(tc->name);
    
    tc->pass = tc->fail = tc->skip = tc->has_skpt = 0;
    tc->log_start = -1;
    tc->log_end = -1;
    
    return 1;
}

// Parse test case result line: "    8     `--- 3 FAIL | 1 PASS | 0 SKIP"
static int parse_case_result(const char *line, TestCase *tc) {
    if (!strstr(line, "`---")) return 0;
    
    const char *p = line;
    
    // Parse counts
    int fail = 0, pass = 0, skip = 0;
    if (sscanf(p, "%*d `--- %d FAIL | %d PASS | %d SKIP", &fail, &pass, &skip) == 3) {
        tc->fail = fail;
        tc->pass = pass;
        tc->skip = skip;
        return 1;
    }
    
    return 0;
}

// Add log line to current suite
static int add_log_line(TestSuite *suite, const char *line, int linenum, int type) {
    if (!grow_log_lines(suite)) {
        return 0;  // Truncation flag already set
    }
    
    LogLine *ll = &suite->log_lines[suite->log_line_count++];
    
    // Extract clean content (removes line number, preserves indentation)
    const char *content = extract_log_content(line);
    
    // Format: "linenum" + content (content already has its own spacing/indentation)
    // Note: snprintf truncation not checked because input is already bounded by
    // fgets(line, MAX_LINE, input) and line numbers are <6 digits, so overflow
    // is practically impossible within MAX_LINE (4096) buffer size.
    if (linenum > 0 && content && *content) {
        snprintf(ll->content, sizeof(ll->content), "%5d%s", linenum, content);
    } else {
        safe_strcpy(ll->content, line, sizeof(ll->content));
    }
    
    ll->linenum = linenum;
    ll->type = type;
    
    return 1;
}

// Main parsing function - Single-pass log parser with state machine
// ALGORITHM: Line-by-line streaming parser, builds ProgramState incrementally
// RETURNS: 1 on success, 0 on fatal error (OOM, allocation failure)
// SIDE EFFECTS: Populates state->suites array, may print warnings to stderr
// POSTCONDITION: All suites parsed, ready for extract_suite_sources()
//
// STATE MACHINE:
//   current_suite: NULL (before first SUIT) or pointing to active suite
//   current_case:  NULL (outside case) or pointing to active test case
//
// MARKERS RECOGNIZED:
//   STIME/ETIME: Optional batch timestamps (metadata only)
//   SUIT /     : Start new test suite
//   RSLT \     : End successful suite
//   ABRT \     : End aborted suite
//   CASE,--    : Start new test case
//   `---       : End test case (result summary)
//
// ERROR HANDLING:
//   - OOM during suite/case growth: return 0 (caller handles cleanup)
//   - MAX_SUITES reached: skip additional suites, continue parsing
//   - MAX_CASES reached: skip additional cases, continue parsing
//   - Missing current_suite: skip lines until next SUIT marker
static int parse_logs(ProgramState *state, FILE *input, const char *filename) {
    char line[MAX_LINE];
    TestSuite *current_suite = NULL;  // Active suite or NULL
    TestCase *current_case = NULL;    // Active case or NULL
    int line_count = 0;               // For fallback line numbering
    
    while (fgets(line, sizeof(line), input)) {
        line_count++;
        
        // Remove trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
        
        // Check for STIME marker (batch start time - optional metadata)
        if (strncmp(line, "STIME ", 6) == 0) {
            safe_strcpy(state->batch_start, line + 6, sizeof(state->batch_start));
            trim(state->batch_start);
            continue;
        }
        
        // Check for ETIME marker (batch end time - optional metadata)
        if (strncmp(line, "ETIME ", 6) == 0) {
            safe_strcpy(state->batch_end, line + 6, sizeof(state->batch_end));
            trim(state->batch_end);
            continue;
        }
        
        // Check for new SUIT marker (starts a new test suite)
        if (strstr(line, "SUIT /")) {
            // Finalize previous suite if any (implicit close)
            current_suite = NULL;
            current_case = NULL;
            
            // Grow suite array if needed (exponential growth strategy)
            if (state->suite_count >= state->suite_capacity) {
                // Check if we've hit the hard limit
                if (state->suite_capacity >= MAX_SUITES) {
                    fprintf(stderr, "t2h: warning: reached MAX_SUITES limit (%d), skipping additional suites\n", MAX_SUITES);
                    continue;  // Skip this suite, continue parsing others
                }
                
                int new_cap = state->suite_capacity * 2;
                if (new_cap > MAX_SUITES) new_cap = MAX_SUITES;
                
                TestSuite *new_suites = realloc(state->suites, 
                                                 new_cap * sizeof(TestSuite));
                if (!new_suites) {
                    // FATAL: Cannot allocate, return failure
                    fprintf(stderr, "t2h: error: out of memory growing suites\n");
                    return 0;
                }
                state->suites = new_suites;
                state->suite_capacity = new_cap;
            }
            
            // Initialize new suite
            current_suite = &state->suites[state->suite_count];
            memset(current_suite, 0, sizeof(TestSuite));
            
            current_suite->case_capacity = 32;
            current_suite->log_capacity = 1000;
            current_suite->cases = NULL;
            current_suite->log_lines = NULL;
            
            current_suite->cases = calloc(current_suite->case_capacity, 
                                          sizeof(TestCase));
            current_suite->log_lines = calloc(current_suite->log_capacity, 
                                              sizeof(LogLine));
            
            if (!current_suite->cases || !current_suite->log_lines) {
                fprintf(stderr, "t2h: error: out of memory allocating suite\n");
                free(current_suite->cases);
                free(current_suite->log_lines);
                current_suite->cases = NULL;
                current_suite->log_lines = NULL;
                return 0;
            }
            
            parse_suit_line(line, current_suite);
            state->suite_count++;
            
            // Add SUIT line to logs (no line number in SUIT lines)
            add_log_line(current_suite, line, 0, LINE_OTHER);
            continue;
        }
        
        // If no current suite, skip line
        if (!current_suite) continue;
        
        // Check for ABRT marker (aborted suite)
        if (strstr(line, "ABRT \\")) {
            parse_abrt_line(line, current_suite);
            // Skip adding ABRT line to log to save space
            
            // Update global totals
            state->total_pass += current_suite->total_pass;
            state->total_fail += current_suite->total_fail;
            state->total_skip += current_suite->total_skip;
            
            current_case = NULL;
            continue;
        }
        
        // Check for RSLT marker (end of suite)
        if (strstr(line, "RSLT \\")) {
            parse_rslt_line(line, current_suite);
            // Skip adding RSLT line to log to save space
            
            // Update global totals
            state->total_pass += current_suite->total_pass;
            state->total_fail += current_suite->total_fail;
            state->total_skip += current_suite->total_skip;
            
            current_case = NULL;
            continue;
        }
        
        // Check for CASE marker (new test case)
        if (strstr(line, "CASE,--")) {
            if (!grow_cases(current_suite)) {
                fprintf(stderr, "t2h: warning: too many test cases, skipping\n");
                current_case = NULL;
            } else {
                current_case = &current_suite->cases[current_suite->case_count];
                memset(current_case, 0, sizeof(TestCase));
                
                parse_case_line(line, current_case);
                current_case->log_start = current_suite->log_line_count;
                current_suite->case_count++;
            }
            
            // Skip adding CASE line to log to save space
            continue;
        }
        
        // Check for case end marker (exclude group end markers like "|`---")
        if (strstr(line, "`---") && !strstr(line, "|`---") && current_case) {
            parse_case_result(line, current_case);
            current_case->log_end = current_suite->log_line_count - 1;
            // Skip adding result line to log to save space
            current_case = NULL;
            continue;
        }
        
        // Classify and add regular log line
        int type = classify_line(line);
        int linenum = extract_line_number(line);
        add_log_line(current_suite, line, linenum > 0 ? linenum : line_count, type);
        
        // Detect SKPT sections in current test case (for border color)
        if (current_case && strstr(line, "SKPT|")) {
            current_case->has_skpt = 1;
        }
        
        // Update current case log end
        if (current_case && current_case->log_end == -1 && current_suite->log_line_count > 0) {
            current_case->log_end = current_suite->log_line_count - 1;
        }
    }
    
    // After parsing all suites, sort test cases by line number and extract source
    // RATIONALE: Source extraction uses single-pass file reading, which requires
    //            test cases to be in ascending line number order. Test execution
    //            order may differ from source order (e.g., when using test filters),
    //            so we sort here to ensure the extraction algorithm's invariant holds.
    for (int i = 0; i < state->suite_count; i++) {
        TestSuite *suite = &state->suites[i];
        
        // Sort test cases by source line number (ascending)
        if (suite->case_count > 0 && suite->cases) {
            qsort(suite->cases, suite->case_count, sizeof(TestCase), 
                  compare_testcase_by_line);
        }
        
        // Extract source code (now guaranteed monotonic line numbers)
        extract_suite_sources(suite, filename);
    }
    
    return 1;
}

// ============================================================================
// Source Code Extraction (04_source_extraction.md - FUNC-006)
// ============================================================================

// Comparison function for sorting test cases by line number
// USAGE: Used by qsort() to sort suite->cases[] before source extraction
// RETURNS: <0 if a before b, >0 if a after b, 0 if equal
// RATIONALE: Source extraction assumes monotonic line numbers for single-pass
//            efficiency. Sorting ensures this invariant holds even if test
//            execution order differs from source order.
static int compare_testcase_by_line(const void *a, const void *b) {
    const TestCase *tc_a = (const TestCase *)a;
    const TestCase *tc_b = (const TestCase *)b;
    
    // Handle cases with line_number = 0 (malformed or missing)
    // Push them to the end so they don't interfere with extraction
    if (tc_a->line_number == 0 && tc_b->line_number == 0) return 0;
    if (tc_a->line_number == 0) return 1;   // a goes after b
    if (tc_b->line_number == 0) return -1;  // b goes after a
    
    // Normal comparison
    return tc_a->line_number - tc_b->line_number;
}

// Try to find source file in multiple locations
static FILE *find_source_file(const char *source_filename, const char *log_path) {
    FILE *fp = NULL;
    char path[MAX_PATH];
    
    // Strategy 1: Try current directory
    fp = fopen(source_filename, "r");
    if (fp) return fp;
    
    // Strategy 2: Try relative to log file directory
    if (log_path && strcmp(log_path, "<stdin>") != 0) {
        // Extract directory from log path
        const char *last_slash = strrchr(log_path, '/');
        if (last_slash) {
            int dir_len = last_slash - log_path;
            if (dir_len > 0 && dir_len < MAX_PATH - 256) {
                memcpy(path, log_path, dir_len);
                path[dir_len] = '/';
                safe_strcpy(path + dir_len + 1, source_filename, 
                           MAX_PATH - dir_len - 1);
                fp = fopen(path, "r");
                if (fp) return fp;
            }
        }
    }
    
    // Strategy 3: Try common test directories
    const char *test_dirs[] = {"test/", "src/", "tests/", "./", NULL};
    for (int i = 0; test_dirs[i]; i++) {
        size_t dir_len = strlen(test_dirs[i]);
        size_t file_len = strlen(source_filename);
        if (dir_len + file_len + 1 < sizeof(path)) {
            memcpy(path, test_dirs[i], dir_len);
            memcpy(path + dir_len, source_filename, file_len + 1);
            fp = fopen(path, "r");
            if (fp) return fp;
        }
    }
    
    return NULL;
}

// Check if a character is in a string literal or character constant
// COMPLEXITY: O(n) where n = pos
// RETURNS: 0=not in string, 1=in double quotes, 2=in single quotes
// USE CASE: Syntax highlighting and comment detection
// NOTE: Does not handle multi-line strings (C doesn't support them)
static int is_in_string_literal(const char *line, int pos) {
    int in_double = 0;
    int in_single = 0;
    
    for (int i = 0; i < pos; i++) {
        if (line[i] == '\\' && (i + 1 < pos)) {
            i++;  // Skip escaped character (e.g., \" or \' or \\)
            continue;
        }
        
        // Toggle string state (strings cannot nest)
        if (line[i] == '"' && !in_single) {
            in_double = !in_double;
        } else if (line[i] == '\'' && !in_double) {
            in_single = !in_single;
        }
    }
    
    if (in_double) return 1;
    if (in_single) return 2;
    return 0;
}

// Check if position is in a comment (optimized to avoid O(n²) complexity)
// COMPLEXITY: O(n) where n = pos (was O(n²) before optimization)
// RETURNS: 0=not in comment, 1=in line comment, 2=in block comment
// OPTIMIZATION: Tracks string state inline to avoid calling is_in_string_literal()
// INVARIANT: Comments inside strings are not treated as comments
// EDGE CASE: Multi-line block comments are tracked via in_block_comment state
static int is_in_comment(const char *line, int pos) {
    int in_block = 0;
    int in_double = 0;
    int in_single = 0;
    
    for (int i = 0; i < pos; i++) {
        // Handle escape sequences
        if (line[i] == '\\' && (i + 1 < pos)) {
            i++;  // Skip escaped character
            continue;
        }
        
        // Track string state (combined to avoid redundant scanning)
        // NOTE: Quotes inside block comments are ignored
        if (line[i] == '"' && !in_single && !in_block) {
            in_double = !in_double;
            continue;
        }
        if (line[i] == '\'' && !in_double && !in_block) {
            in_single = !in_single;
            continue;
        }
        
        // Skip if we're in a string (comments in strings are not real comments)
        if (in_double || in_single) {
            continue;
        }
        
        // Check for line comment (//)
        if (i + 1 < pos && line[i] == '/' && line[i+1] == '/') {
            return 1;  // Everything after // is a comment
        }
        
        // Check for block comment start (/*)
        if (i + 1 < pos && line[i] == '/' && line[i+1] == '*') {
            in_block = 1;
            i++;  // Skip the *
            continue;
        }
        
        // Check for block comment end
        if (in_block && i + 1 < pos && line[i] == '*' && line[i+1] == '/') {
            in_block = 0;
            i++;  // Skip the /
            continue;
        }
    }
    
    return in_block ? 2 : 0;
}

// Helper: Free all uncommitted temp_lines and reset count
// CRITICAL: Prevents memory leaks when abandoning partially-collected source code
// OWNERSHIP: Frees all temp_lines[0..count-1].content, then resets count
// USAGE: Call before advancing case index without transferring ownership
static void free_temp_lines(SourceLine *temp_lines, int *temp_line_count) {
    for (int k = 0; k < *temp_line_count; k++) {
        free(temp_lines[k].content);
        temp_lines[k].content = NULL;  // Prevent double-free
    }
    *temp_line_count = 0;
}

// Check if filename has a C/C++ extension
// RETURNS: 1 if extension is .c, .cpp, or .c++, 0 otherwise
static int is_c_source_file(const char *filename) {
    if (!filename) return 0;
    
    const char *ext = strrchr(filename, '.');
    if (!ext) return 0;
    
    // Check for C/C++ extensions (case-insensitive)
    if (strcmp(ext, ".c") == 0 || strcmp(ext, ".C") == 0) return 1;
    if (strcmp(ext, ".cpp") == 0 || strcmp(ext, ".CPP") == 0) return 1;
    if (strcmp(ext, ".c++") == 0 || strcmp(ext, ".C++") == 0) return 1;
    if (strcmp(ext, ".cc") == 0 || strcmp(ext, ".CC") == 0) return 1;
    if (strcmp(ext, ".cxx") == 0 || strcmp(ext, ".CXX") == 0) return 1;
    
    return 0;
}

// Extract source code for all test cases in a suite (optimized single-pass)
// PREREQUISITE: Test cases MUST be sorted by line_number (ascending) before calling
//               This is enforced by qsort() in parse_logs() after parsing completes
// ALGORITHM: Single-pass file reading with brace-matching state machine
// COMPLEXITY: O(n*m) where n=file lines, m=average line length
// MEMORY: Stack-allocated temp_lines[MAX_SOURCE_LINES] buffer per case
// OWNERSHIP: Transfers temp_lines content to TestCase.source.lines on success
// LIFECYCLE: Called after parse_logs, before generate_html
//
// STATE MACHINE INVARIANTS:
//   - temp_line_count must be 0 or ownership transferred when changing cases
//   - brace_count == 0 && found_opening == 1 means case extraction complete
//   - temp_lines[] content is either transferred or freed, never leaked
//
// EDGE CASES:
//   - Missing source file: all cases marked available=-1
//   - Non C/C++ source file: all cases marked available=-1
//   - Unmatched braces: use EOF heuristic (take what we have)
//   - MAX_SOURCE_LINES exceeded: case truncated at limit
//   - OOM during strdup: free temp_lines, mark case as error
static void extract_suite_sources(TestSuite *suite, const char *log_path) {
    if (!suite || !suite->filename[0]) return;
    
    // Check if source file is C/C++ (based on extension)
    if (!is_c_source_file(suite->filename)) {
        // Not a C/C++ file - mark all test cases as unavailable
        for (int i = 0; i < suite->case_count; i++) {
            suite->cases[i].source.available = -1;
        }
        return;
    }
    
    // Try to open source file once for the entire suite
    FILE *fp = find_source_file(suite->filename, log_path);
    if (!fp) {
        // File not found or not readable - mark all test cases as unavailable
        for (int i = 0; i < suite->case_count; i++) {
            suite->cases[i].source.available = -1;
        }
        return;
    }
    
    // Initialize all test cases
    for (int i = 0; i < suite->case_count; i++) {
        TestCase *tc = &suite->cases[i];
        safe_strcpy(tc->source.filepath, suite->filename, sizeof(tc->source.filepath));
        tc->source.start_line = tc->line_number;
        tc->source.available = -1;  // Will be updated if successful
        tc->source.lines = NULL;
        tc->source.line_count = 0;
    }
    
    // Single pass through the source file
    char line[MAX_LINE];
    int current_line = 0;
    int current_case_idx = 0;
    int in_block_comment = 0;  // Tracks multi-line /* */ comments
    int brace_count = 0;        // Nesting level of {...}
    int found_opening = 0;      // Have we seen at least one '{'?
    
    // Temporary buffer for current test case (stack-allocated for performance)
    // CRITICAL: Content pointers allocated via strdup, must be freed or transferred
    SourceLine temp_lines[MAX_SOURCE_LINES];
    int temp_line_count = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        current_line++;
        
        // Check if we've moved to the next test case
        if (current_case_idx < suite->case_count) {
            TestCase *tc = &suite->cases[current_case_idx];
            
            // Check if we're starting a new test case
            if (current_line == tc->line_number) {
                // Reset state for new test case
                brace_count = 0;
                found_opening = 0;
                temp_line_count = 0;        // CRITICAL: Must be 0 when starting new case
                in_block_comment = 0;
            }
            
            // Check if we're currently extracting this test case
            // NOTE: Stops at MAX_SOURCE_LINES to prevent stack overflow
            if (current_line >= tc->line_number && temp_line_count < MAX_SOURCE_LINES) {
                // Remove trailing newline
                size_t len = strlen(line);
                if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
                
                // Store line in temporary buffer (strdup allocates heap memory)
                temp_lines[temp_line_count].content = strdup(line);
                temp_lines[temp_line_count].linenum = current_line;
                
                if (!temp_lines[temp_line_count].content) {
                    // CRITICAL: OOM - free all temp_lines, mark case unavailable
                    free_temp_lines(temp_lines, &temp_line_count);
                    tc->source.available = -1;
                    current_case_idx++;
                    continue;
                }
                
                temp_line_count++;
                
                // Process line for brace matching (only if we haven't found the end)
                // OPTIMIZATION: Stop brace counting once we find closing brace
                if (!found_opening || brace_count > 0) {
                    size_t line_len = strlen(line);  // Cache to avoid O(n) per iteration
                    for (int i = 0; line[i] != '\0'; i++) {
                        // Handle block comments across lines (state persists via in_block_comment)
                        if (!is_in_string_literal(line, i)) {
                            if (i + 1 < (int)line_len && line[i] == '/' && line[i+1] == '*') {
                                in_block_comment = 1;
                                i++;  // Skip '*'
                                continue;
                            }
                            if (in_block_comment && i + 1 < (int)line_len && line[i] == '*' && line[i+1] == '/') {
                                in_block_comment = 0;
                                i++;  // Skip '/'
                                continue;
                            }
                        }
                        
                        // Skip if in comment or string (don't count braces in comments/strings)
                        if (in_block_comment || is_in_comment(line, i) || is_in_string_literal(line, i)) {
                            continue;
                        }
                        
                        // Count braces (nesting depth tracking)
                        if (line[i] == '{') {
                            brace_count++;
                            found_opening = 1;  // Mark that we've entered function body
                        } else if (line[i] == '}') {
                            brace_count--;
                            
                            // Found matching closing brace - finalize this test case
                            // INVARIANT: brace_count == 0 means balanced braces
                            if (found_opening && brace_count == 0) {
                                tc->source.end_line = current_line;
                                
                                // OWNERSHIP TRANSFER: temp_lines → tc->source.lines
                                tc->source.lines = calloc(temp_line_count, sizeof(SourceLine));
                                if (tc->source.lines) {
                                    // Shallow copy (content pointers transferred)
                                    for (int k = 0; k < temp_line_count; k++) {
                                        tc->source.lines[k] = temp_lines[k];
                                    }
                                    tc->source.line_count = temp_line_count;
                                    tc->source.available = 1;
                                    temp_line_count = 0;  // CRITICAL: Ownership transferred, prevent double-free
                                } else {
                                    // CRITICAL: OOM - must free temp_lines to prevent leak
                                    free_temp_lines(temp_lines, &temp_line_count);
                                    tc->source.available = -1;
                                }
                                
                                // Move to next test case
                                current_case_idx++;
                                break;  // Exit brace counting loop (optimization)
                            }
                        }
                    }
                }
            }
        } else {
            // Processed all test cases, stop reading file
            // CRITICAL: Free any uncommitted temp_lines to prevent leak
            free_temp_lines(temp_lines, &temp_line_count);
            break;
        }
    }
    
    // EOF HANDLER: Handle case where we didn't find closing brace for last test case
    // EDGE CASE: Truncated file, syntax error, or function longer than MAX_SOURCE_LINES
    if (current_case_idx < suite->case_count && temp_line_count > 0) {
        TestCase *tc = &suite->cases[current_case_idx];
        
        // HEURISTIC: Take what we have (partial function better than nothing)
        tc->source.end_line = current_line;
        tc->source.lines = calloc(temp_line_count, sizeof(SourceLine));
        if (tc->source.lines) {
            for (int k = 0; k < temp_line_count; k++) {
                tc->source.lines[k] = temp_lines[k];
            }
            tc->source.line_count = temp_line_count;
            tc->source.available = 1;
            temp_line_count = 0;  // Ownership transferred
        } else {
            // Out of memory - free temp lines
            free_temp_lines(temp_lines, &temp_line_count);
            tc->source.available = -1;
        }
    } else if (temp_line_count > 0) {
        // Safety: free any remaining uncommitted lines (shouldn't happen in normal flow)
        free_temp_lines(temp_lines, &temp_line_count);
    }
    
    fclose(fp);
}

// ============================================================================
// Syntax Highlighting for C Source Code
// ============================================================================

// Check if a word is a C keyword
static int is_c_keyword(const char *word, int len) {
    static const char *keywords[] = {
        "auto", "break", "case", "char", "const", "continue", "default", "do",
        "double", "else", "enum", "extern", "float", "for", "goto", "if",
        "int", "long", "register", "return", "short", "signed", "sizeof", "static",
        "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while",
        NULL
    };
    
    for (int i = 0; keywords[i]; i++) {
        if (strlen(keywords[i]) == (size_t)len && strncmp(word, keywords[i], len) == 0) {
            return 1;
        }
    }
    return 0;
}

// Check if a word is a C type
static int is_c_type(const char *word, int len) {
    static const char *types[] = {
        "size_t", "FILE", "uint8_t", "uint16_t", "uint32_t", "uint64_t",
        "int8_t", "int16_t", "int32_t", "int64_t", "bool", "NULL",
        NULL
    };
    
    for (int i = 0; types[i]; i++) {
        if (strlen(types[i]) == (size_t)len && strncmp(word, types[i], len) == 0) {
            return 1;
        }
    }
    return 0;
}

// Output source code line with syntax highlighting
static void output_source_line_highlighted(FILE *out, const char *line) {
    if (!line) return;
    
    int in_string = 0;
    int in_char = 0;
    int in_line_comment = 0;
    int in_block_comment = 0;
    const char *p = line;
    
    while (*p) {
        // Check for block comment end
        if (in_block_comment && *p == '*' && *(p+1) == '/') {
            fprintf(out, "*/");
            p += 2;
            in_block_comment = 0;
            fprintf(out, "</span>");
            continue;
        }
        
        // If in block comment, output as comment
        if (in_block_comment) {
            if (*p == '<') fprintf(out, "&lt;");
            else if (*p == '>') fprintf(out, "&gt;");
            else if (*p == '&') fprintf(out, "&amp;");
            else fputc(*p, out);
            p++;
            continue;
        }
        
        // Check for line comment
        if (!in_string && !in_char && *p == '/' && *(p+1) == '/') {
            fprintf(out, "<span class=\"src-comment\">");
            in_line_comment = 1;
        }
        
        // If in line comment, output rest as comment
        if (in_line_comment) {
            if (*p == '<') fprintf(out, "&lt;");
            else if (*p == '>') fprintf(out, "&gt;");
            else if (*p == '&') fprintf(out, "&amp;");
            else fputc(*p, out);
            p++;
            if (*p == '\0') fprintf(out, "</span>");
            continue;
        }
        
        // Check for block comment start
        if (!in_string && !in_char && *p == '/' && *(p+1) == '*') {
            fprintf(out, "<span class=\"src-comment\">");
            fprintf(out, "/*");
            p += 2;
            in_block_comment = 1;
            continue;
        }
        
        // Handle escape sequences in strings/chars
        if ((in_string || in_char) && *p == '\\' && *(p+1)) {
            fputc(*p, out);
            p++;
            if (*p == '<') fprintf(out, "&lt;");
            else if (*p == '>') fprintf(out, "&gt;");
            else if (*p == '&') fprintf(out, "&amp;");
            else fputc(*p, out);
            p++;
            continue;
        }
        
        // Check for string start/end
        if (*p == '"' && !in_char) {
            if (!in_string) {
                fprintf(out, "<span class=\"src-string\">");
                in_string = 1;
            } else {
                fputc(*p, out);
                p++;
                fprintf(out, "</span>");
                in_string = 0;
                continue;
            }
        }
        
        // Check for char start/end
        if (*p == '\'' && !in_string) {
            if (!in_char) {
                fprintf(out, "<span class=\"src-string\">");
                in_char = 1;
            } else {
                fputc(*p, out);
                p++;
                fprintf(out, "</span>");
                in_char = 0;
                continue;
            }
        }
        
        // If in string or char, just output
        if (in_string || in_char) {
            if (*p == '<') fprintf(out, "&lt;");
            else if (*p == '>') fprintf(out, "&gt;");
            else if (*p == '&') fprintf(out, "&amp;");
            else fputc(*p, out);
            p++;
            continue;
        }
        
        // Check for preprocessor directive
        if (*p == '#' && (p == line || *(p-1) == ' ' || *(p-1) == '\t')) {
            fprintf(out, "<span class=\"src-preprocessor\">");
            while (*p && *p != '\n') {
                if (*p == '<') fprintf(out, "&lt;");
                else if (*p == '>') fprintf(out, "&gt;");
                else if (*p == '&') fprintf(out, "&amp;");
                else fputc(*p, out);
                p++;
            }
            fprintf(out, "</span>");
            continue;
        }
        
        // Check for number
        if (isdigit((unsigned char)*p)) {
            fprintf(out, "<span class=\"src-number\">");
            while (*p && (isdigit((unsigned char)*p) || *p == '.' || *p == 'x' || 
                          *p == 'X' || (*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F') ||
                          *p == 'u' || *p == 'U' || *p == 'l' || *p == 'L')) {
                fputc(*p, out);
                p++;
            }
            fprintf(out, "</span>");
            continue;
        }
        
        // Check for identifier (keyword or type)
        if (isalpha((unsigned char)*p) || *p == '_') {
            const char *word_start = p;
            while (*p && (isalnum((unsigned char)*p) || *p == '_')) p++;
            int word_len = p - word_start;
            
            if (is_c_keyword(word_start, word_len)) {
                fprintf(out, "<span class=\"src-keyword\">");
                for (int i = 0; i < word_len; i++) fputc(word_start[i], out);
                fprintf(out, "</span>");
            } else if (is_c_type(word_start, word_len)) {
                fprintf(out, "<span class=\"src-type\">");
                for (int i = 0; i < word_len; i++) fputc(word_start[i], out);
                fprintf(out, "</span>");
            } else {
                for (int i = 0; i < word_len; i++) fputc(word_start[i], out);
            }
            continue;
        }
        
        // Regular character
        if (*p == '<') fprintf(out, "&lt;");
        else if (*p == '>') fprintf(out, "&gt;");
        else if (*p == '&') fprintf(out, "&amp;");
        else fputc(*p, out);
        p++;
    }
}

// ============================================================================
// HTML Generation (05_html_generation.md)
// ============================================================================

// HTML/CSS templates (reused from t2h.c with minor adaptations)
static void output_html_header(FILE *out, const char *title, int dark_theme) {
    fprintf(out, "<!DOCTYPE html>\n");
    fprintf(out, "<html lang=\"en\">\n");
    fprintf(out, "<head>\n");
    fprintf(out, "<meta charset=\"UTF-8\">\n");
    fprintf(out, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
    fprintf(out, "<title>%s - Test Report</title>\n", title);
    
    // CSS Variables for Light Theme (Default)
    fprintf(out, "<style>\n");
    fprintf(out, ":root{--bg-body:#f5f5f5;--bg-card:#fff;--bg-card-alt:#fafafa;");
    fprintf(out, "--bg-stat:#f8f9fa;--text-main:#333;--text-muted:#666;--text-link:#667eea;");
    fprintf(out, "--border-color:#ddd;--border-test:#667eea;--shadow-sm:rgba(0,0,0,.1);");
    fprintf(out, "--shadow-md:rgba(0,0,0,.15);--code-bg:#1e1e1e;--code-text:#d4d4d4;");
    fprintf(out, "--code-linenum:#858585;--hover-bg:#f0f0f0;--hover-card:#e9ecef}\n");
    
    // CSS Variables for Dark Theme
    fprintf(out, "[data-theme=\"dark\"]{--bg-body:#0d1117;--bg-card:#161b22;--bg-card-alt:#1c2128;");
    fprintf(out, "--bg-stat:#21262d;--text-main:#e6edf3;--text-muted:#8b949e;--text-link:#58a6ff;");
    fprintf(out, "--border-color:#30363d;--border-test:#58a6ff;--shadow-sm:rgba(0,0,0,.3);");
    fprintf(out, "--shadow-md:rgba(0,0,0,.4);--code-bg:#0d1117;--code-text:#e6edf3;");
    fprintf(out, "--code-linenum:#6e7681;--hover-bg:#21262d;--hover-card:#30363d}\n");
    
    // Base Styles with Variables (minified)
    fprintf(out, "*{margin:0;padding:0;box-sizing:border-box}");
    fprintf(out, "body{font-family:'Segoe UI',Tahoma,Geneva,Verdana,sans-serif;background:var(--bg-body);color:var(--text-main);padding:15px;transition:background-color .3s ease,color .3s ease}");
    fprintf(out, ".container{max-width:1400px;margin:0 auto}");
    fprintf(out, ".header{background:linear-gradient(135deg,#667eea 0%%,#764ba2 100%%);color:white;padding:12px;border-radius:10px;margin-bottom:20px;box-shadow:0 4px 6px var(--shadow-sm)}");
    fprintf(out, ".header h1{font-size:2.5em;margin-bottom:10px}");
    fprintf(out, ".header .subtitle{opacity:.9;font-size:1.1em}");
    fprintf(out, ".card{background:var(--bg-card);padding:10px;border-radius:10px;box-shadow:0 2px 4px var(--shadow-sm);margin-bottom:15px}");
    fprintf(out, ".stats{display:grid;grid-template-columns:repeat(3,1fr);gap:15px;margin:20px 0}");
    fprintf(out, ".stat-item{text-align:center;padding:15px;background:var(--bg-stat);border-radius:8px}");
    fprintf(out, ".stat-number{font-size:2em;font-weight:bold;margin-bottom:5px}");
    fprintf(out, ".stat-label{font-size:.9em;color:var(--text-muted)}");
    fprintf(out, ".color-green{color:#28a745}.color-red{color:#dc3545}.color-yellow{color:#ffc107}");
    fprintf(out, ".test-case{background:var(--bg-card-alt);padding:8px;margin-bottom:10px;border-radius:8px;border-left:4px solid var(--border-test)}");
    fprintf(out, ".test-case:hover{box-shadow:0 4px 8px var(--shadow-sm);background:var(--hover-bg)}");
    fprintf(out, ".test-case.has-failures{border-left-color:#dc3545}");
    fprintf(out, ".test-case.all-passed{border-left-color:#28a745}");
    fprintf(out, ".test-case.all-skipped{border-left-color:#ffc107}");
    fprintf(out, ".test-case-name{font-size:1.1em;font-weight:600;margin-bottom:5px;display:flex;justify-content:space-between}");
    fprintf(out, ".badge{padding:2px 12px;border-radius:12px;font-size:.75em;font-weight:600;margin:0 5px}");
    fprintf(out, ".badge-pass{background:#d4edda;color:#155724}");
    fprintf(out, ".badge-fail{background:#f8d7da;color:#721c24}");
    fprintf(out, ".badge-skip{background:#fff3cd;color:#856404}");
    fprintf(out, ".log-viewer{display:none;margin-top:10px;background:var(--code-bg);color:var(--code-text);padding:12px;border-radius:6px;font-family:'Consolas','Monaco',monospace;font-size:13px;max-height:500px;overflow-y:auto}");
    fprintf(out, ".log-viewer.visible{display:block}");
    fprintf(out, ".log-line{padding:2px 0;white-space:pre-wrap}");
    fprintf(out, ".log-linenum{display:inline-block;width:50px;color:var(--code-linenum);text-align:right;margin-right:10px}");
    fprintf(out, ".log-pass{color:#4ec9b0}");
    fprintf(out, ".log-fail{color:#f48771;font-weight:bold}");
    fprintf(out, ".log-skip{color:#d7ba7d}");
    fprintf(out, ".log-note{color:#6a9955}");
    fprintf(out, ".log-clock{color:#9cdcfe}");
    fprintf(out, ".log-case{color:#dcdcaa;font-weight:bold}");
    fprintf(out, ".src-keyword{color:#569cd6;font-weight:bold}");
    fprintf(out, ".src-type{color:#4ec9b0}");
    fprintf(out, ".src-string{color:#ce9178}");
    fprintf(out, ".src-comment{color:#6a9955;font-style:italic}");
    fprintf(out, ".src-number{color:#b5cea8}");
    fprintf(out, ".src-preprocessor{color:#c586c0}");
    fprintf(out, ".abort-warning{cursor:help;font-size:1.2em; font-weight:400;}");
    fprintf(out, ".suite-section{margin-bottom:8px}");
    fprintf(out, ".suite-header{background:linear-gradient(135deg,#1e3c72 0%%,#2a5298 100%%);color:white;padding:8px;border-radius:10px;margin-bottom:5px;box-shadow:0 3px 5px var(--shadow-md)}");
    fprintf(out, ".suite-header.has-failures{border-left:8px solid #dc3545}");
    fprintf(out, ".suite-header.all-passed{border-left:8px solid #28a745}");
    fprintf(out, ".suite-header h2{color:white;margin-bottom:10px;font-size:1.1em}");
    fprintf(out, ".suite-header p{margin:5px 0;opacity:.95}");
    fprintf(out, ".suite-header code{background:rgba(255,255,255,.2);padding:2px 8px;border-radius:4px}");
    fprintf(out, ".suite-content{display:block;transition:all .3s ease}");
    fprintf(out, ".suite-content.collapsed{display:none}");
    fprintf(out, ".collapse-btn{background:rgba(255,255,255,.15);color:white;border:2px solid rgba(255,255,255,.3);padding:8px 16px;border-radius:5px;cursor:pointer;font-weight:600;font-size:.9em;transition:all .2s;white-space:nowrap;margin-left:10px;width:45px;height:36px;display:inline-flex;align-items:center;justify-content:center}");
    fprintf(out, ".collapse-btn:hover{background:rgba(255,255,255,.25);border-color:rgba(255,255,255,.5);transform:scale(1.05)}");
    fprintf(out, ".top-btn{background:rgba(255,255,255,.15);color:white;border:2px solid rgba(255,255,255,.3);padding:8px 16px;border-radius:5px;cursor:pointer;font-weight:600;font-size:.9em;transition:all .2s;white-space:nowrap;margin-left:10px;width:45px;height:36px;display:inline-flex;align-items:center;justify-content:center}");
    fprintf(out, ".top-btn:hover{background:rgba(255,255,255,.25);border-color:rgba(255,255,255,.5);transform:translateY(-2px)}");
    fprintf(out, ".theme-btn{background:rgba(255,255,255,.15);color:white;border:2px solid rgba(255,255,255,.3);padding:10px 20px;border-radius:5px;cursor:pointer;font-weight:600;font-size:.9em;transition:all .2s;white-space:nowrap;margin-left:10px}");
    fprintf(out, ".theme-btn:hover{background:rgba(255,255,255,.25);border-color:rgba(255,255,255,.5);transform:scale(1.05)}");
    fprintf(out, ".expand-test-btn{background:var(--bg-stat);color:var(--text-main);border:1px solid var(--border-color);padding:4px 10px;border-radius:4px;cursor:pointer;font-weight:500;font-size:.85em;transition:all .2s;white-space:nowrap;margin-left:10px}");
    fprintf(out, ".expand-test-btn:hover{background:var(--hover-card);transform:scale(1.05)}");
    fprintf(out, ".tab-container{display:flex;gap:10px;margin:10px 0;border-bottom:2px solid var(--border-color)}");
    fprintf(out, ".tab-button{background:var(--bg-stat);border:none;padding:10px 20px;cursor:pointer;border-radius:5px 5px 0 0;font-weight:600;transition:all .2s;color:var(--text-main)}");
    fprintf(out, ".tab-button:hover{background:var(--hover-card)}");
    fprintf(out, ".tab-button.active{background:#667eea;color:white}");
    // Utility classes for inline style replacement
    fprintf(out, ".flx-btw{display:flex;justify-content:space-between;align-items:center}");
    fprintf(out, ".flx-1{flex:1}");
    fprintf(out, ".txt-ctr{text-align:center}");
    fprintf(out, ".mb-10{margin-bottom:10px}");
    fprintf(out, ".mt-15{margin-top:15px}");
    fprintf(out, ".hide{display:none}\n");
    fprintf(out, "</style>");
    fprintf(out, "</head>");
    fprintf(out, "<body%s>", dark_theme ? " data-theme=\"dark\"" : "");
    fprintf(out, "<div class=\"container\">");
}

static void output_html_footer(FILE *out) {
    fprintf(out, "</div>");
    // Minified JavaScript functions
    fprintf(out, "<script>");
    fprintf(out, "function scrollToTop(){window.scrollTo({top:0,behavior:'smooth'})}\n");
    fprintf(out, "function toggleSuite(i){const c=document.getElementById('suite-content-'+i),b=document.getElementById('collapse-btn-'+i);if(c.classList.contains('collapsed')){c.classList.remove('collapsed');b.textContent='▼'}else{c.classList.add('collapsed');b.textContent='▶'}}\n");
    fprintf(out, "function toggleLog(id){const l=document.getElementById('log-'+id),s=document.getElementById('src-'+id),t=document.getElementById('tabs-case-'+id),a=document.getElementById('arrow-'+id),ac=document.getElementById('arrow-code-'+id);if(l.classList.contains('visible')){l.classList.remove('visible');if(t)t.style.display='none';if(a)a.innerHTML='&#9654;'}else{if(s&&s.classList.contains('visible')){s.classList.remove('visible');if(ac)ac.innerHTML='&#9654;'}l.classList.add('visible');if(t)t.style.display='flex';if(a)a.innerHTML='&#9660;';if(t){const btn=t.querySelector('.tab-button:nth-child(1)');if(btn)showTab({target:btn,stopPropagation:function(){}},'log-'+id,'src-'+id)}}}\n");
    fprintf(out, "function toggleCode(id){const s=document.getElementById('src-'+id),l=document.getElementById('log-'+id),t=document.getElementById('tabs-case-'+id),a=document.getElementById('arrow-code-'+id),al=document.getElementById('arrow-'+id);if(s.classList.contains('visible')){s.classList.remove('visible');if(t)t.style.display='none';if(a)a.innerHTML='&#9654;'}else{if(l&&l.classList.contains('visible')){l.classList.remove('visible');if(al)al.innerHTML='&#9654;'}s.classList.add('visible');if(t)t.style.display='flex';if(a)a.innerHTML='&#9660;';if(t){const btn=t.querySelector('.tab-button:nth-child(2)');if(btn)showTab({target:btn,stopPropagation:function(){}},'src-'+id,'log-'+id)}}}\n");
    fprintf(out, "function showTab(e,s,h){e.stopPropagation();document.getElementById(s).style.display='block';document.getElementById(h).style.display='none';const b=e.target.parentElement.getElementsByClassName('tab-button');for(let i=0;i<b.length;i++)b[i].classList.remove('active');e.target.classList.add('active')}\n");
    fprintf(out, "function toggleTheme(){const b=document.body;if(b.getAttribute('data-theme')==='dark')b.removeAttribute('data-theme');else b.setAttribute('data-theme','dark')}\n");
    fprintf(out, "</script>");
    fprintf(out, "</body>");
    fprintf(out, "</html>");
}

// Output a test case section with Log/Source tabs
static void output_test_case(FILE *out, TestSuite *suite, TestCase *tc, int case_idx, int show_warning) {
    const char *status_class = "";
    if (tc->fail > 0) {
        status_class = " has-failures";  // RED: one or more FAIL
    } else if (tc->pass > 0) {
        status_class = " all-passed";    // GREEN: one or more PASS, no FAIL
    } else if (tc->skip > 0 || tc->has_skpt) {
        status_class = " all-skipped";   // YELLOW: one or more SKIP or SKPT sections, no PASS or FAIL
    }
    // else: BLUE (default): no PASS/FAIL/SKIP/SKPT
    
    fprintf(out, "<div class=\"test-case%s\">", status_class);
    fprintf(out, "<div class=\"test-case-name\">");
    fprintf(out, "<span>");
    html_escape(out, tc->name);
    // Add warning symbol if this is the last test case in an aborted suite
    // if (show_warning) {
    //     fprintf(out, " <span class=\"abort-warning\" title=\"Aborted\">☠</span>");
    //     //fprintf(out, "☠");
    // }
    fprintf(out, "</span>");
    fprintf(out, "<span>");
    // Show skull in fail badge if aborted, otherwise show X
    if (show_warning) {
        fprintf(out, "<span class=\"badge badge-fail\">%d ☠</span>", tc->fail);
    } else {
        fprintf(out, "<span class=\"badge badge-fail\">%d ✗</span>", tc->fail);
    }
    fprintf(out, "<span class=\"badge badge-pass\">%d ✓</span>", tc->pass);
    fprintf(out, "<span class=\"badge badge-skip\">%d ↷</span>", tc->skip);
    
    // Check if source is available
    int has_source = tc->source.available == 1 && tc->source.line_count > 0;
    
    // Log button
    fprintf(out, "<button class=\"expand-test-btn\" onclick=\"toggleLog('case-%d'); event.stopPropagation();\">", 
            case_idx);
    fprintf(out, "<span id=\"arrow-case-%d\">&#9654;</span> Log</button>", case_idx);
    
    // Code button (after log button)
    if (has_source) {
        fprintf(out, "<button class=\"expand-test-btn\" onclick=\"toggleCode('case-%d'); event.stopPropagation();\">", 
                case_idx);
        fprintf(out, "<span id=\"arrow-code-case-%d\">&#9654;</span> Code</button>", case_idx);
    }
    
    fprintf(out, "</span>");
    fprintf(out, "</div>");
    
    // Tab buttons (if source is available)
    if (has_source) {
        fprintf(out, "<div class=\"tab-container hide\" id=\"tabs-case-%d\">", case_idx);
        fprintf(out, "<button class=\"tab-button active\" onclick=\"showTab(event, 'log-case-%d', 'src-case-%d')\">Log Output</button>", 
                case_idx, case_idx);
        fprintf(out, "<button class=\"tab-button\" onclick=\"showTab(event, 'src-case-%d', 'log-case-%d')\">Source Code</button>", 
                case_idx, case_idx);
        fprintf(out, "</div>");
    }
    
    // Log viewer
    fprintf(out, "<div class=\"log-viewer%s\" id=\"log-case-%d\">", 
            has_source ? "" : "", case_idx);
    
    // Output log lines for this case
    int start = (tc->log_start >= 0) ? tc->log_start : 0;
    int end = (tc->log_end >= 0) ? tc->log_end : suite->log_line_count - 1;
    
    for (int i = start; i <= end && i < suite->log_line_count; i++) {
        LogLine *ll = &suite->log_lines[i];
        const char *css_class = "log-line";
        
        switch (ll->type) {
            case LINE_PASS: css_class = "log-pass"; break;
            case LINE_FAIL: css_class = "log-fail"; break;
            case LINE_SKIP: css_class = "log-skip"; break;
            case LINE_NOTE: css_class = "log-note"; break;
            case LINE_CLOCK: css_class = "log-clock"; break;
            case LINE_CASE: css_class = "log-case"; break;
            default: css_class = "log-line"; break;
        }
        
        // HTML escape the content (log lines already contain line numbers)
        fprintf(out, "<div class=\"%s\">", css_class);
        
        // HTML escaping with leading spaces converted to &nbsp;
        int leading = 1;  // Track if we're still in leading whitespace
        for (const char *p = ll->content; *p; p++) {
            if (leading && *p == ' ') {
                fprintf(out, "&nbsp;");
            } else {
                leading = 0;  // No longer in leading whitespace
                switch (*p) {
                    case '<': fprintf(out, "&lt;"); break;
                    case '>': fprintf(out, "&gt;"); break;
                    case '&': fprintf(out, "&amp;"); break;
                    case ' ': fprintf(out, " "); break;  // Regular spaces after content starts
                    default: fputc(*p, out); break;
                }
            }
        }
        fprintf(out, "</div>\n");
    }
    
    fprintf(out, "</div>");
    
    // Source code viewer (FUNC-006)
    if (has_source) {
        fprintf(out, "<div class=\"log-viewer hide\" id=\"src-case-%d\">", 
                case_idx);
        
        for (int i = 0; i < tc->source.line_count; i++) {
            SourceLine *sl = &tc->source.lines[i];
            fprintf(out, "<div class=\"log-line\">");
            fprintf(out, "<span class=\"log-linenum\">%5d</span> ", sl->linenum);
            
            // Output source code with syntax highlighting
            if (sl->content) {
                output_source_line_highlighted(out, sl->content);
            }
            fprintf(out, "</div>\n");
        }
        
        fprintf(out, "</div>");
    }
    
    fprintf(out, "</div>");
}

// Generate HTML for a single suite
static void output_suite(FILE *out, TestSuite *suite, int suite_idx, int total_suites) {
    fprintf(out, "<div class=\"suite-section\">");
    
    // Determine status class based on test results (same logic as suite index)
    const char *status_class = "";
    if (suite->total_fail > 0) {
        status_class = " has-failures";
    } else if (suite->total_pass > 0) {
        status_class = " all-passed";
    }
    // else: default (blue) - no class needed
    
    // Suite header (clickable to collapse/expand)
    fprintf(out, "<div class=\"suite-header%s\" id=\"suite-%d\">", status_class, suite_idx);
    fprintf(out, "<div class=\"flx-btw\">");
    fprintf(out, "<div class=\"flx-1\">");
    fprintf(out, "<h2>");
    html_escape(out, suite->title);
    fprintf(out, " ");
    // Badges next to title
    // Fail badge with warning symbol inside if suite was aborted
    fprintf(out, "<span class=\"badge badge-fail\">%d ✗", suite->total_fail);
    if (suite->aborted) {
        fprintf(out, " <span class=\"abort-warning\" title=\"Aborted\"> ☠</span>");
    }
    fprintf(out, "</span>");
    fprintf(out, "<span class=\"badge badge-pass\">%d ✓</span>", suite->total_pass);
    fprintf(out, "<span class=\"badge badge-skip\">%d ↷</span>", suite->total_skip);
    fprintf(out, "</h2>");
    fprintf(out, "<p style=\"font-size: 12px;\">File: <code>");
    html_escape(out, suite->filename);
    fprintf(out, "</code>");
    if (suite->start_time[0]) {
        fprintf(out, " | Started: %s", suite->start_time);
    }
    if (suite->end_time[0]) {
        fprintf(out, " | Ended: %s", suite->end_time);
    }
    fprintf(out, "</p>");
    fprintf(out, "</div>");
    
    // Buttons column
    fprintf(out, "<div class=\"txt-ctr\">");
    
    // Show buttons (theme button only for single suite, collapse/top for multi-suite)
    if (total_suites > 1) {
        fprintf(out, "<button class=\"top-btn\" onclick=\"scrollToTop()\">⇧</button>");
        fprintf(out, "<button class=\"collapse-btn\" onclick=\"toggleSuite(%d)\" id=\"collapse-btn-%d\">▶</button>", suite_idx, suite_idx);
    } else {
        // For single suite, show theme button
        fprintf(out, "<button class=\"theme-btn\" onclick=\"toggleTheme()\">🌓</button>");
    }
    fprintf(out, "</div>");
    
    fprintf(out, "</div>");
    fprintf(out, "</div>");
    
    // Suite content (collapsible only for multi-suite, expanded for single suite)
    if (total_suites > 1) {
        fprintf(out, "<div class=\"suite-content collapsed\" id=\"suite-content-%d\">", suite_idx);
    } else {
        fprintf(out, "<div class=\"suite-content\" id=\"suite-content-%d\">", suite_idx);
    }
    
    // Test cases
    fprintf(out, "<div class=\"card\">");
    //fprintf(out, "<h3>Test Cases</h3>");
    for (int i = 0; i < suite->case_count; i++) {
        // Show warning only on last test case if suite was aborted
        int is_last_case = (i == suite->case_count - 1);
        int show_warning = suite->aborted && is_last_case;
        output_test_case(out, suite, &suite->cases[i], suite_idx * 1000 + i, show_warning);
    }
    fprintf(out, "</div>");
    
    fprintf(out, "</div>");  // Close suite-content
    fprintf(out, "</div>");  // Close suite-section
}

// Main HTML generation
static int generate_html(ProgramState *state, FILE *output) {
    if (!state || !output) return 0;
    
    // Determine title
    const char *title = "Test Results";
    if (state->suite_count == 1) {
        title = state->suites[0].title;
    } else if (state->suite_count > 1) {
        title = "Consolidated Test Results";
    }
    
    output_html_header(output, title, state->dark_theme);
    
    // Main header (only for multi-suite reports)
    if (state->suite_count > 1) {
        fprintf(output, "<div class=\"header\">");
        fprintf(output, "<div class=\"flx-btw\">");
        fprintf(output, "<div>");
        fprintf(output, "<h1>%s</h1>", title);
        fprintf(output, "<div class=\"mt-15\">");
        fprintf(output, "<span class=\"badge badge-fail\">%d ✗</span>", state->total_fail);
        fprintf(output, "<span class=\"badge badge-pass\">%d ✓</span>", state->total_pass);
        fprintf(output, "<span class=\"badge badge-skip\">%d ↷</span>", state->total_skip);
        fprintf(output, "</div>");
        fprintf(output, "</div>");
        fprintf(output, "<button class=\"theme-btn\" onclick=\"toggleTheme()\">🌓</button>");
        fprintf(output, "</div>");
        fprintf(output, "</div>");
    }
    
    // Output each suite
    for (int i = 0; i < state->suite_count; i++) {
        output_suite(output, &state->suites[i], i, state->suite_count);
    }
    
    output_html_footer(output);
    
    return 1;
}
