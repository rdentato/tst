# --report-error Flag Demonstration

This directory contains a demonstration of the `--report-error` flag.

## Files

- `t_report_error_demo.c` - Test program with intentional failures
- `demo_report_error.sh` - Script showing exit code behavior

## Running the Demo

```bash
./demo_report_error.sh
```

This will show four scenarios:

1. **No flag, no failures**: Exit 0
2. **No flag, with failures**: Exit 0 (default behavior)
3. **With flag, no failures**: Exit 0
4. **With flag, with failures**: Exit 1 ← Key difference!

## Manual Testing

```bash
# Without flag - always returns 0
./t_report_error_demo +demo
echo "Exit: $?"  # Shows 0

# With flag - returns 1 on failure
./t_report_error_demo +demo --report-error
echo "Exit: $?"  # Shows 1
```

## Use in CI/CD

Add `--report-error` to your test commands in CI:

```yaml
# GitHub Actions example
- name: Run tests
  run: ./t_report_error_demo --report-error
```

This ensures your CI pipeline fails when tests fail.
