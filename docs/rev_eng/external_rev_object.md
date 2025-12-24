# `omath::rev_eng::ExternalReverseEngineeredObject` — typed offsets over external memory

> Header: `omath/rev_eng/external_reverse_engineered_object.hpp`
> Namespace: `omath::rev_eng`
> Pattern: **CRTP-style wrapper** around a user-provided *ExternalMemoryManagementTrait* that actually reads/writes another process or device’s memory.

A tiny base class for reverse-engineered objects that live **outside** your address space. You pass an absolute base address and a trait with `read_memory` / `write_memory`. Your derived types then expose strongly-typed getters/setters that delegate into the trait using **byte offsets**.

---

## Quick look

```cpp
template<class ExternalMemoryManagementTrait>
class ExternalReverseEngineeredObject {
public:
  explicit ExternalReverseEngineeredObject(std::uintptr_t addr)
    : m_object_address(addr) {}

protected:
  template<class Type>
  [[nodiscard]] Type get_by_offset(std::ptrdiff_t offset) const {
    return ExternalMemoryManagementTrait::read_memory(m_object_address + offset);
  }

  template<class Type>
  void set_by_offset(std::ptrdiff_t offset, const Type& value) const {
    ExternalMemoryManagementTrait::write_memory(m_object_address + offset, value);
  }

private:
  std::uintptr_t m_object_address{};
};
```

---

## Trait requirements

Your `ExternalMemoryManagementTrait` must provide:

```cpp
// Reads sizeof(T) bytes starting at absolute address and returns T.
template<class T>
static T read_memory(std::uintptr_t absolute_address);

// Writes sizeof(T) bytes to absolute address.
template<class T>
static void write_memory(std::uintptr_t absolute_address, const T& value);
```

> Tip: If your implementation prefers returning `bool`/`expected<>` for writes, either:
>
> * make `write_memory` `void` and throw/log internally, or
> * adjust `set_by_offset` in your fork to surface the status.

### Common implementations

* **Windows**: wrap `ReadProcessMemory` / `WriteProcessMemory` with a stored `HANDLE` (often captured via a singleton or embedded in the trait).
* **Linux**: `/proc/<pid>/mem`, `process_vm_readv/writev`, `ptrace`.
* **Device/FPGA**: custom MMIO/driver APIs.

---

## How to use (derive and map fields)

Create a concrete type for your target structure and map known offsets:

```cpp
struct WinRPMTrait {
  template<class T>
  static T read_memory(std::uintptr_t addr) {
    T out{};
    SIZE_T n{};
    if (!ReadProcessMemory(g_handle, reinterpret_cast<LPCVOID>(addr), &out, sizeof(T), &n) || n != sizeof(T))
      throw std::runtime_error("RPM failed");
    return out;
  }
  template<class T>
  static void write_memory(std::uintptr_t addr, const T& val) {
    SIZE_T n{};
    if (!WriteProcessMemory(g_handle, reinterpret_cast<LPVOID>(addr), &val, sizeof(T), &n) || n != sizeof(T))
      throw std::runtime_error("WPM failed");
  }
};

class Player final : public omath::rev_eng::ExternalReverseEngineeredObject<WinRPMTrait> {
  using Base = omath::rev_eng::ExternalReverseEngineeredObject<WinRPMTrait>;
public:
  using Base::Base; // inherit ctor (takes base address)

  // Offsets taken from your RE notes (in bytes)
  Vector3<float> position() const { return get_by_offset<Vector3<float>>(0x30); }
  void set_position(const Vector3<float>& p) const { set_by_offset(0x30, p); }

  float health() const { return get_by_offset<float>(0x100); }
  void  set_health(float h) const { set_by_offset(0x100, h); }
};
```

Then:

```cpp
Player p{ /* base address you discovered */ 0x7FF6'1234'0000ull };
auto pos = p.position();
p.set_health(100.f);
```

---

## Design notes & constraints

* **Offsets are byte offsets** from the object’s **base address** passed to the constructor.
* **Type safety is on you**: `Type` must match the external layout at that offset (endian, packing, alignment).
* **No lifetime tracking**: if the target object relocates/frees, you must update/recreate the wrapper with the new base address.
* **Thread safety**: the class itself is stateless; thread safety depends on your trait implementation.
* **Endianness**: assumes the host and target endianness agree, or your trait handles conversion.
* **Error handling**: this header doesn’t prescribe it; adopt exceptions/expected/logging inside the trait.

---

## Best practices

* Centralize offsets in one place (constexprs or a small struct) and **comment source/version** (e.g., *game v1.2.3*).
* Wrap fragile multi-field writes in a trait-level **transaction** if your platform supports it.
* Validate pointers/guards (e.g., vtable signature, canary) before trusting offsets.
* Prefer **plain old data** (`struct` without virtuals) for `Type` to ensure trivial byte copies.

---

## Minimal trait sketch (POSIX, `process_vm_readv`)

```cpp
struct LinuxPvmTrait {
  static pid_t pid;

  template<class T> static T read_memory(std::uintptr_t addr) {
    T out{};
    iovec local{ &out, sizeof(out) }, remote{ reinterpret_cast<void*>(addr), sizeof(out) };
    if (process_vm_readv(pid, &local, 1, &remote, 1, 0) != ssize_t(sizeof(out)))
      throw std::runtime_error("pvm_readv failed");
    return out;
  }

  template<class T> static void write_memory(std::uintptr_t addr, const T& val) {
    iovec local{ const_cast<T*>(&val), sizeof(val) }, remote{ reinterpret_cast<void*>(addr), sizeof(val) };
    if (process_vm_writev(pid, &local, 1, &remote, 1, 0) != ssize_t(sizeof(val)))
      throw std::runtime_error("pvm_writev failed");
  }
};
```

---

## Troubleshooting

* **Garbled values** → wrong offset/Type, or target’s structure changed between versions.
* **Access denied** → missing privileges (admin/root), wrong process handle, or page protections.
* **Crashes in trait** → add bounds/sanity checks; many APIs fail on unmapped pages.
* **Writes “stick” only briefly** → the target may constantly overwrite (server authority / anti-cheat / replication).

---

*Last updated: 24 Dec 2025*
