# TST Documentation

This directory contains comprehensive documentation for the TST (Test) framework - a lightweight, single-header C testing framework.

## Documentation Structure

The documentation is organized into several categories to serve different audiences and purposes:

### Core Documentation

- **[log_format.bnf](log_format.bnf)** - BNF grammar specification for TST log output format
- **[nesting-prevention.md](nesting-prevention.md)** - Technical notes on test nesting prevention mechanisms
- **[prog_manual.md](prog_manual.md)** - Programming manual for test authors
- **[ref_manual.md](ref_manual.md)** - Complete reference manual for all TST macros and features
- **[html_report.png](html_report.png)** - Screenshot showing example HTML report output


### Maintainer Documentation (`maintainer/`)

Information for project maintainers and contributors:

- **[t2h_architecture.md](maintainer/t2h_architecture.md)** - Architecture of the t2h (test-to-HTML) utility
- **[tst_h_internals.md](maintainer/tst_h_internals.md)** - Internal implementation details of tst.h
- **[tst_sh_internals.md](maintainer/tst_sh_internals.md)** - Internal details of tst.sh script
- **[maintenance_playbook.md](maintainer/maintenance_playbook.md)** - Procedures for maintaining the codebase
- **[debugging_and_review.md](maintainer/debugging_and_review.md)** - Debugging strategies and code review guidelines
- **[error_and_performance.md](maintainer/error_and_performance.md)** - Error handling patterns and performance considerations

## Getting Started

If you're new to TST, start with these documents in order:

1. **[../tutorial/README.md](../tutorial/README.md)** - Hands-on tutorial (located in tutorial directory)
2. **[prog_manual.md](prog_manual.md)** - Learn how to write tests
3. **[ref_manual.md](ref_manual.md)** - Complete macro reference

## For Different Audiences

### Test Authors
- Start: [../tutorial/README.md](../tutorial/README.md)
- Reference: [prog_manual.md](prog_manual.md), [ref_manual.md](ref_manual.md)
- Quick reference: [../tutorial/QUICK_REFERENCE.md](../tutorial/QUICK_REFERENCE.md)

### Framework Developers
- Implementation: [maintainer/tst_h_internals.md](maintainer/tst_h_internals.md)
- Maintenance: [maintainer/maintenance_playbook.md](maintainer/maintenance_playbook.md)

### Tool Developers (t2h)
- Architecture: [maintainer/t2h_architecture.md](maintainer/t2h_architecture.md)

### Integration Developers
- Log format: [log_format.bnf](log_format.bnf)

## Contributing to Documentation

When adding or modifying documentation:

1. Place general user documentation in the `docs/` root
2. Place maintainer-specific docs in `maintainer/`
3. Update this README when adding new categories or major documents
4. Use Markdown format (.md) for all text documentation
5. Include cross-references to related documents

## Documentation Standards

- Use clear, concise language
- Include code examples where applicable
- Cross-reference related documents
- Keep design docs synchronized with implementation
- Update documentation when changing code behavior
