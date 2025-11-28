# TST Tag Behavior Quick Reference

## Tag Declaration
```c
tstcase("Test name", +tag1, +tag2, -tag3) { ... }
```

## Default Behavior (No Command-Line Filters)
- ✅ Untagged tests: RUN
- ❌ Any tagged test (+tag or -tag): SKIP

## Command-Line Filters

| Filter | Activates |
|--------|-----------|
| `+tag` | Tests with `+tag` |
| `-tag` | Tests with `-tag` |
| `+*` | All tests with any `+tag` (NOT `-tag`) |

## Examples

### Test Suite
```c
tstcase("A") { }                    // No tags
tstcase("B", +slow) { }             // +slow
tstcase("C", -ci) { }               // -ci  
tstcase("D", +slow, +db) { }        // +slow, +db
tstcase("E", +db, -ci) { }          // +db, -ci
```

### What Runs?

| Command | A | B | C | D | E |
|---------|---|---|---|---|---|
| `./test` | ✅ | ❌ | ❌ | ❌ | ❌ |
| `./test +slow` | ✅ | ✅ | ❌ | ✅ | ❌ |
| `./test +*` | ✅ | ✅ | ❌ | ✅ | ✅ |
| `./test -ci` | ✅ | ❌ | ✅ | ❌ | ✅ |
| `./test +slow -ci` | ✅ | ✅ | ✅ | ✅ | ✅ |

## Key Principle
Tags require **activation** via command-line filters. Without filters, only untagged tests run.
