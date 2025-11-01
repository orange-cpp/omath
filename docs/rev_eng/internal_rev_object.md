# `omath::rev_eng::InternalReverseEngineeredObject` — raw in-process offset/VTABLE access

> Header: `omath/rev_eng/internal_reverse_engineered_object.hpp`
> Namespace: `omath::rev_eng`
> Purpose: Convenience base for **internal** (same-process) RE wrappers that:
>
> * read/write fields by **byte offset** from `this`
> * call **virtual methods** by **vtable index**

---

## At a glance

```cpp
class InternalReverseEngineeredObject {
protected:
  template<class Type>
  [[nodiscard]] Type&       get_by_offset(std::ptrdiff_t offset);

  template<class Type>
  [[nodiscard]] const Type& get_by_offset(std::ptrdiff_t offset) const;

  template<std::size_t id, class ReturnType>
  ReturnType call_virtual_method(auto... arg_list);
};
```

* `get_by_offset<T>(off)` — returns a **reference** to `T` located at `reinterpret_cast<uintptr_t>(this) + off`.
* `call_virtual_method<id, Ret>(args...)` — fetches the function pointer from `(*reinterpret_cast<void***>(this))[id]` and invokes it as a free function with implicit `this` passed explicitly.

On MSVC builds the function pointer type uses `__thiscall`; on non-MSVC it uses a plain function pointer taking `void*` as the first parameter (the typical Itanium ABI shape).

---

## Example: wrapping a reverse-engineered class

```cpp
struct Player : omath::rev_eng::InternalReverseEngineeredObject {
  // Field offsets (document game/app version!)
  static constexpr std::ptrdiff_t kHealth   = 0x100;
  static constexpr std::ptrdiff_t kPosition = 0x30;

  // Accessors
  float&             health()             { return get_by_offset<float>(kHealth); }
  const float&       health()       const { return get_by_offset<float>(kHealth); }

  Vector3<float>&    position()           { return get_by_offset<Vector3<float>>(kPosition); }
  const Vector3<float>& position() const  { return get_by_offset<Vector3<float>>(kPosition); }

  // Virtuals (vtable indices discovered via RE)
  int  getTeam()            { return call_virtual_method<27, int>(); }
  void setArmor(float val)  { call_virtual_method<42, void>(val); } // signature must match!
};
```

Usage:

```cpp
auto* p = /* pointer to live Player instance within the same process */;
p->health() = 100.f;
int team = p->getTeam();
```

---

## How `call_virtual_method` resolves the signature

```cpp
template<std::size_t id, class ReturnType>
ReturnType call_virtual_method(auto... arg_list) {
#ifdef _MSC_VER
  using Fn = ReturnType(__thiscall*)(void*, decltype(arg_list)...);
#else
  using Fn = ReturnType(*)(void*, decltype(arg_list)...);
#endif
  return (*reinterpret_cast<Fn**>(this))[id](this, arg_list...);
}
```

* The **first parameter** is always `this` (`void*`).
* Remaining parameter types are deduced from the **actual arguments** (`decltype(arg_list)...`).
  Ensure you pass arguments with the correct types (e.g., `int32_t` vs `int`, pointer/ref qualifiers), or define thin wrappers that cast to the exact signature you recovered.

> ⚠ On 32-bit MSVC the `__thiscall` distinction matters; on 64-bit MSVC it’s ignored (all member funcs use the common x64 calling convention).

---

## Safety notes (read before using!)

Working at this level is inherently unsafe; be deliberate:

1. **Correct offsets & alignment**

    * `get_by_offset<T>` assumes `this + offset` is **properly aligned** for `T` and points to an object of type `T`.
    * Wrong offsets or misalignment ⇒ **undefined behavior** (UB), crashes, silent corruption.

2. **Object layout assumptions**

    * The vtable pointer is assumed to be at the **start of the most-derived subobject at `this`**.
    * With **multiple/virtual inheritance**, the desired subobject’s vptr may be at a non-zero offset. If so, adjust `this` to that subobject before calling, e.g.:

      ```cpp
      auto* sub = reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(this) + kSubobjectOffset);
      // … then reinterpret sub instead of this inside a custom helper
      ```

3. **ABI & calling convention**

    * Indices and signatures are **compiler/ABI-specific**. Recheck after updates or different builds (MSVC vs Clang/LLVM-MSVC vs MinGW).

4. **Strict aliasing**

    * Reinterpreting memory as unrelated `T` can violate aliasing rules. Prefer **trivially copyable** PODs and exact original types where possible.

5. **Const-correctness**

    * The `const` overload returns `const T&` but still reinterprets memory; do not write through it. Use the non-const overload to mutate.

6. **Thread safety**

    * No synchronization is provided. Ensure the underlying object isn’t concurrently mutated in incompatible ways.

---

## Tips & patterns

* **Centralize offsets** in `constexpr` with comments (`// game v1.2.3, sig XYZ`).
* **Guard reads**: if you have a canary or RTTI/vtable hash, check it before relying on offsets.
* **Prefer accessors** returning references**:** lets you both read and write with natural syntax.
* **Wrap tricky virtuals**: if a method takes complex/reference params, wrap `call_virtual_method` in a strongly typed member that casts exactly as needed.

---

## Troubleshooting

* **Crash on virtual call** → wrong index or wrong `this` (subobject), or mismatched signature (args/ret or calling conv).
* **Weird field values** → wrong offset, wrong type size/packing, stale layout after an update.
* **Only in 32-bit** → double-check `__thiscall` and parameter passing (register vs stack).

---

*Last updated: 1 Nov 2025*
