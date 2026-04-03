# Contributing

## Prerequisites

- C++ compiler with C++23 support (Clang 18+, GCC 14+, MSVC 19.38+)
- CMake 3.25+
- Git
- Familiarity with the codebase (see `INSTALL.md` for setup)

For questions, create a draft PR or reach out via [Telegram](https://t.me/orange_cpp).

## Workflow

1. [Fork](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/working-with-forks/fork-a-repo) the repository.
2. Create a feature branch from `main`.
3. Make your changes, ensuring tests pass.
4. Open a [pull request](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/proposing-changes-to-your-work-with-pull-requests/creating-a-pull-request-from-a-fork) against `main`.

## Code Style

Follow the project `.clang-format`. Run `clang-format` before committing.

## Building

Use one of the CMake presets defined in `CMakePresets.json`:

```bash
cmake --preset <preset-name> -DOMATH_BUILD_TESTS=ON
cmake --build --preset <preset-name>
```

Run `cmake --list-presets` to see available configurations.

## Tests

All new functionality must include unit tests. Run the test binary after building to verify nothing is broken.
