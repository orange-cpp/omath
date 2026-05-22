---
name: code-style
description: omath project C++ code style derived from .idea/codeStyles/Project.xml and .clang-format. Use when writing, editing, or reviewing C++ code in this repo so formatting and naming match the rest of the codebase.
---

# omath Code Style

Authoritative sources: `.clang-format` (formatting) and `.idea/codeStyles/Project.xml` (Rider/CLion naming + formatting). When in doubt, run clang-format — it is the enforced formatter (`clangFormatSettings.ENABLED = true`).

## Formatting

Base style: LLVM with Stroustrup-style braces.

- **Indent**: 4 spaces, no tabs. Tab width 4. Continuation indent 8.
- **Column limit**: 120.
- **Namespace indentation**: `All` — indent contents of every namespace.
- **Access modifier offset**: -4 (access specifiers sit at the class column; members indent one level deeper).
- **Pointer/reference alignment**: Left, with a space *before* `*` / `&` in declarations: `const Vector3& other`, `Type* ptr`.
- **Include blocks**: Merge. Sort using-declarations.
- **Keep blank lines**: max 2 in code and declarations. No blank line at the start of a block.
- **Align trailing comments**: false.
- **Break before binary operators**: non-assignment.

### Braces (Allman / next-line for everything)

Opening brace on its own line after:
class, struct, union, enum, namespace, function, control statement (`if`/`for`/`while`/`switch`), `case` label, lambda body, `catch`, `else`, `while` (of do-while), extern block.

Empty functions, records, and namespaces still split (`SplitEmptyFunction/Record/Namespace: true`).

### Short-form rules (all disabled)

Never collapse onto one line: blocks, functions, lambdas, `if` statements, loops.

### Templates

`template<class T>` goes on its own line, declaration follows on the next line:

```cpp
template<class Type>
requires std::is_arithmetic_v<Type>
class Vector3 : public Vector2<Type>
{
    ...
};
```

No space after `template` keyword. `requires` clause is not extra-indented.

### Spaces

- After control-statement keywords (`if (`, `for (`, `while (`).
- **Not** before `(` in function declarations/definitions/calls.
- After C-style cast: `(int) x`.
- Around range-based-for colon: `for (auto& x : xs)`.
- After commas in template args/params.
- Inside empty parens/braces/templates: no.

## Naming

| Kind | Style | Example |
|---|---|---|
| Namespaces | `snake_case` | `omath::pathfinding`, `omath::primitives` |
| Types (class, struct, enum, union, concept, type alias, typedef, template parameter) | `PascalCase` | `Vector3`, `NavigationMesh`, `Astar`, `ContainedType` |
| Functions (free + member) | `snake_case` | `find_path`, `distance_to_sqr`, `create_box` |
| Fields (class/struct/union members) | `snake_case` | `dir_forward`, `nav_mesh` |
| Variables (global, local, lambda) | `snake_case` | `length_value`, `side_size` |
| Parameters | `snake_case` | `dir_right`, `v_other` |
| Macros | `UPPER_SNAKE_CASE` | `OMATH_FOO_BAR` |
| Enumerators | `UPPER_SNAKE_CASE` | `IMPOSSIBLE_BETWEEN_ANGLE`, `WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS` |

Enum types themselves are PascalCase (`enum class Vector3Error`, `enum class Error`); their members are UPPER_SNAKE_CASE.

## Files

- Headers: `.hpp`, sources: `.cpp`. Both use `snake_case` filenames (e.g. `vector3.hpp`, `proj_pred_engine_avx2.hpp`).
- C headers: `.h`, sources: `.c` (no enforced filename style).
- CUDA: `.cu` / `.cuh`.
- C++ modules: `.ixx`, `.mxx`, `.cppm`, `.ccm`, `.cxxm`, `.c++m`.
- Header guard: `#pragma once` only — no `#ifndef` guards.
- File header comment is optional and follows the form `// Created by <name> on <date>`.

## Idioms used throughout the codebase

- Prefer `[[nodiscard]]`, `noexcept`, and `constexpr` on math / value-type methods.
- `namespace omath` is the root; sub-features live in nested namespaces (`omath::collision`, `omath::engines::source_engine`, etc.).
- Closing namespace brace gets a trailing comment: `} // namespace omath::primitives`.
- Use `std::expected<T, E>` with an `enum class …Error` for fallible operations (see `Vector3Error`, `projection::Error`).

## When editing

Match the surrounding style exactly. If a region disagrees with this guide, prefer the existing local style — don't reformat unrelated code (per the project's CLAUDE.md "Surgical Changes" rule). Run clang-format on touched files before committing.
