# Contributing to ares_cdh
## Code Style
- 4-space indentation
- `snake_case` for variables and functions
- `PascalCase` for types and classes
- `UPPER_SNAKE_CASE` for constants
- `#pragma once` for include guards
- Doxygen `///` comments on all public interfaces

## Type System Rules
Use only types from types.hpp (U8, U16, U32, Byte). Never use int, long or unsigned directly - their sizes are platform-dependent. Always use static_cast for narrowing conversions.

## Memory Rules
No dynamic allocation. Do not use new, delete, malloc or free anywhere in the library. Fixed-size arrays and stack allocation only. The CRC table uses static constexpr - compile-time constants belong in read-only memory, not RAM.

## Error Handling Rules
Do not use exceptions anywhere in the library. Use `Result<T>` for all fallible operations - operations that can fail at runtime return `Result<T>` where `T` is the success value. Use `Result<void>` when there is no success value. Precondition violations (programming errors) call `std::abort()` - not exceptions, not silent failure. The distinction: runtime conditions get `Result`, programming bugs get `abort`.

## Testing Requirements
Every validation step requires at least one test covering the rejection case with the specific expected ParseError, and at least one test covering the success path. Redundant tests that cover identical code paths should be consolidated - test suites should have no duplicate coverage.

## Commit Message Format
Commit messages follow the Conventional Commits format. A commit message consists of a type prefix, a short imperative description, and optionally a body explaining the reasoning. Earlier commits predate this convention.

# Standards Compliance
All packet format decisions must be verified against CCSDS 133.0-B-2 (Space Packet Protocol, June 2020). If your change interprets the standard differently than the existing implementation, cite the relevant section number in your PR description.