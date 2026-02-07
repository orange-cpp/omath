# `omath::EncryptedVariable` — Compile-time XOR-encrypted variable

> Header: `omath/containers/encrypted_variable.hpp`
> Namespace: `omath`
> Depends on: `<array>`, `<cstddef>`, `<cstdint>`, `<span>`

`EncryptedVariable` keeps a value XOR-encrypted in memory at rest, using a **compile-time generated random key**. It is designed to hinder static analysis and memory scanners from reading sensitive values (e.g., game constants, keys, thresholds) directly from process memory.

---

## Key concepts

* **Compile-time key generation** — a unique random byte array is produced at compile time via SplitMix64 + FNV-1a seeded from `__FILE__`, `__DATE__`, and `__TIME__`. Each `OMATH_DEF_CRYPT_VAR` expansion receives a distinct key.
* **XOR cipher** — `encrypt()` / `decrypt()` toggle the encrypted state by XOR-ing the raw bytes of the stored value with the key.
* **VarAnchor (RAII guard)** — `drop_anchor()` returns a `VarAnchor` that decrypts on construction and re-encrypts on destruction, ensuring the plaintext window is as short as possible.

---

## API

```cpp
namespace omath {

template<class T, std::size_t key_size, std::array<std::uint8_t, key_size> key>
class EncryptedVariable final {
public:
    using value_type = std::remove_cvref_t<T>;

    constexpr explicit EncryptedVariable(const value_type& data);

    [[nodiscard]] constexpr bool is_encrypted() const;

    constexpr void decrypt();
    constexpr void encrypt();

    [[nodiscard]] constexpr value_type&       value();
    [[nodiscard]] constexpr const value_type& value() const;

    [[nodiscard]] constexpr auto drop_anchor();  // returns VarAnchor

    constexpr ~EncryptedVariable();  // decrypts on destruction
};

template<class EncryptedVarType>
class VarAnchor final {
public:
    constexpr VarAnchor(EncryptedVarType& var);   // decrypts
    constexpr ~VarAnchor();                        // re-encrypts
};

} // namespace omath
```

### Helper macros

```cpp
// Generate a compile-time random byte array of length N
#define OMATH_CT_RAND_ARRAY_BYTE(N)  /* ... */

// Declare a type alias for EncryptedVariable<TYPE> with KEY_SIZE random bytes
#define OMATH_DEF_CRYPT_VAR(TYPE, KEY_SIZE)  /* ... */
```

---

## Usage examples

### Basic encrypt / decrypt

```cpp
#include "omath/containers/encrypted_variable.hpp"

// Define an encrypted float with a 16-byte key
using EncFloat = OMATH_DEF_CRYPT_VAR(float, 16);

EncFloat secret(3.14f);         // encrypted immediately
// secret.value() is XOR-scrambled in memory

secret.decrypt();
float v = secret.value();       // v == 3.14f
secret.encrypt();               // scrambled again
```

### RAII guard with `drop_anchor()`

```cpp
EncFloat secret(42.0f);

{
    auto anchor = secret.drop_anchor();   // decrypts
    float val = secret.value();           // safe to read
    // ... use val ...
}   // anchor destroyed → re-encrypts automatically
```

---

## Notes & edge cases

* **Force-inline**: When `OMATH_ENABLE_FORCE_INLINE` is defined, encrypt/decrypt operations use compiler-specific force-inline attributes to reduce the call-site footprint visible in disassembly.
* **Not cryptographically secure**: XOR with a static key is an obfuscation technique, not encryption. It raises the bar for casual memory scanning but does not resist a determined attacker who can read the binary.
* **Destructor decrypts**: The destructor calls `decrypt()` so the value is in plaintext at the end of its lifetime (e.g., for logging or cleanup).
* **Thread safety**: No internal synchronization. Protect concurrent access with external locks.
* **`constexpr` support**: All operations are `constexpr`-friendly (C++20).

---

## See also

* [Pattern Scan](../utility/pattern_scan.md) — scan memory for byte patterns.
* [Getting Started](../getting_started.md) — quick start with OMath.

---

*Last updated: Feb 2026*
