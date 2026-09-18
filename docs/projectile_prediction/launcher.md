# `omath::projectile_prediction::Launcher` — who fires, and from where

> Header: `omath/projectile_prediction/launcher.hpp`
> Namespace: `omath::projectile_prediction`
> Types: `Launcher<T>`, `MuzzleOffset<T>`, `ViewBasis<T>`
> Used by: `ProjPredEngineInterface::maybe_calculate_aim`, every engine `PredEngineTrait`

`Launcher` separates *who shoots* from *what is shot*. `Projectile` keeps the physical properties of the round; `Launcher` says where the eye is and where the projectile leaves relative to it.

---

## API

```cpp
namespace omath::projectile_prediction {

// Muzzle position relative to the eye, in the shooter's own view frame
template<class T = float>
struct MuzzleOffset {
  T forward{};
  T right{};
  T up{};
  constexpr bool is_zero() const noexcept;
};

// Forward, right and up of a view frame, in the engine's world axes
template<class T = float>
struct ViewBasis {
  Vector3<T> forward;
  Vector3<T> right;
  Vector3<T> up;
};

template<class T = float>
struct Launcher {
  Vector3<T>      eye_origin;        // where the view angles apply (camera / eye)
  MuzzleOffset<T> muzzle_offset{};   // rotates with the view angles
  Vector3<T>      world_offset{};    // fixed world-space part, if the engine has one

  // eye + world_offset + forward * a + right * b + up * c for the given basis
  constexpr Vector3<T> launch_origin(const ViewBasis<T>& basis) const noexcept;
};

} // namespace omath::projectile_prediction
```

---

## Why the muzzle offset is view-relative

Engines spawn projectiles at `eye + forward * a + right * b + up * c`, so the world-space launch position depends on the very angles the solver is looking for. A fixed world-space offset can only be right when the shooter already faces the target. The engines therefore place the muzzle from the direct angles to the target first, solve, and then re-solve once from where the muzzle ends up with the solved angles.

`world_offset` exists for engines whose spawn offset really is fixed in world space, and for the compatibility wrappers that map the old `Projectile::m_launch_offset` onto it. Both parts add up; leave either at zero.

---

## Example

```cpp
using namespace omath::projectile_prediction;

constexpr Projectile<float> rocket{.m_launch_speed = 1100.f, .m_gravity_scale = 0.f};
constexpr Launcher<float> launcher{
    .eye_origin    = {0.f, 0.f, 64.f},
    .muzzle_offset = {.forward = 16.f, .right = 8.f, .up = -6.f},   // Source-style
};

const ProjPredEngineLegacy<> solver(800.f, 1.f / 1000.f, 10.f, 5.f);
if (const auto aim = solver.maybe_calculate_aim(rocket, launcher, target))
{
    // aim->angles is what to set on the camera; aim->aim_point is on that ray
}
```

Every engine trait exposes `calc_view_basis(pitch, yaw)` so the same `Launcher` works for any engine's axis convention.

---

*Last updated: 18 Sep 2026*
