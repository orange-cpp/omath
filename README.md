<div align = center>

![banner](https://blog.libomath.org/wp-content/uploads/2026/08/omath_logo.png)

![GitHub License](https://img.shields.io/github/license/orange-cpp/omath)
![GitHub contributors](https://img.shields.io/github/contributors/orange-cpp/omath)
![GitHub top language](https://img.shields.io/github/languages/top/orange-cpp/omath)
![GitHub repo size](https://img.shields.io/github/repo-size/orange-cpp/omath)
[![CodeFactor](https://www.codefactor.io/repository/github/orange-cpp/omath/badge)](https://www.codefactor.io/repository/github/orange-cpp/omath)
![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/orange-cpp/omath/cmake-multi-platform.yml)
[![Vcpkg package](https://repology.org/badge/version-for-repo/vcpkg/orange-math.svg)](https://repology.org/project/orange-math/versions)
![Conan Center](https://img.shields.io/conan/v/omath)
![GitHub forks](https://img.shields.io/github/forks/orange-cpp/omath)
[![discord badge](https://dcbadge.limes.pink/api/server/https://discord.gg/eDgdaWbqwZ?style=flat)](https://discord.gg/eDgdaWbqwZ)
[![telegram badge](https://img.shields.io/badge/Telegram-2CA5E0?style=flat-squeare&logo=telegram&logoColor=white)](https://t.me/orangennotes)

omath is a 100% independent, constexpr template blazingly fast math/physics/games/mods/cheats development framework that doesn't have legacy C++ code.

It provides the latest features, is highly customizable, has all for cheat development, DirectX/OpenGL hooking, premade support for different game engines, much more constexpr stuff than in other libraries and more...
<br>
<br>

---

**[<kbd> <br> Install <br> </kbd>][INSTALL]** 
**[<kbd> <br> Examples <br> </kbd>][EXAMPLES]** 
**[<kbd> <br> Documentation <br> </kbd>][DOCUMENTATION]** 
**[<kbd> <br> Contribute <br> </kbd>][CONTRIBUTING]** 
**[<kbd> <br> Donate <br> </kbd>][SPONSOR]** 

---

<br>

</div>


## Quick Example

```cpp
#include <omath/omath.hpp>

using namespace omath;

// 3D vector operations
Vector3<float> a{1, 2, 3};
Vector3<float> b{4, 5, 6};

auto dot = a.dot(b);              // 32.0
auto cross = a.cross(b);          // (-3, 6, -3)
auto distance = a.distance_to(b); // ~5.196
auto normalized = a.normalized(); // Unit vector

// World-to-screen projection (Source Engine example)
using namespace omath::source_engine;
Camera camera(position, angles, viewport, fov, near_plane, far_plane);

if (auto screen = camera.world_to_screen(world_position)) {
    // Draw at screen->x, screen->y
}
```

**[See more examples and tutorials][TUTORIALS]**

## Reverse Engineering Toolkit

`omath::rev_eng` gives every reversed structure typed, self-documenting field access instead of raw `reinterpret_cast` and magic offsets. The same class works against a process you injected into (`InternalReverseEngineeredObject`, plain memory access) or a target read from outside (`ExternalReverseEngineeredObject`, backed by any trait you write around `ReadProcessMemory`/`process_vm_readv`/a driver):

```cpp
#include <omath/linear_algebra/vector3.hpp>
#include <omath/rev_eng/external_rev_object.hpp>

using omath::Vector3;
using omath::rev_eng::ExternalReverseEngineeredObject;

// Any trait with read_memory<T>/write_memory<T> works - ReadProcessMemory, process_vm_readv, a DMA device, ...
struct RpmTrait {
    template<class T>
    static T read_memory(std::uintptr_t address) {
        T value{};
        ReadProcessMemory(g_handle, reinterpret_cast<LPCVOID>(address), &value, sizeof(T), nullptr);
        return value;
    }
};

class Player final : public ExternalReverseEngineeredObject<RpmTrait> {
public:
    using ExternalReverseEngineeredObject::ExternalReverseEngineeredObject;

    [[nodiscard]] Vector3<float> origin() const { return get_by_offset<Vector3<float>>(0x134); }
    [[nodiscard]] int health() const { return get_by_offset<int>(0x140); }
};

Player local_player{local_player_address};
auto pos = local_player.origin();
```

See [external_rev_object.md](docs/rev_eng/external_rev_object.md) and [internal_rev_object.md](docs/rev_eng/internal_rev_object.md) for the full API. On top of that foundation OMath ships ready-made helpers for the harder, engine-specific parts of reverse engineering:

- **Byte pattern scanning** with wildcards across **PE, ELF and Mach-O** - files, loaded modules, or a memory dump, works even against Wine apps.
- **Function hooking** for **DirectX 9/11/12** and **OpenGL** via `omath::hooks::HooksManager`, for drawing an overlay into someone else's render loop.
- **Unreal Engine name resolution** - `get_actor_name`/`get_object_by_index` walk `GNames`/`GObjects` across **UE 2.5 through UE 5** (pointer-array, chunked-array and `FNamePool` layouts) to turn a raw `UObject*` into its class and instance name. See [actor_name.md](docs/engines/unreal_engine/actor_name.md) and [object_array.md](docs/engines/unreal_engine/object_array.md).
- A full, runnable example that ties all three together - process attach, `GObjects` walk, `FName` resolution, and a live GLFW/OpenGL/ImGui overlay with `world_to_screen`/`world_to_radar` - lives in [`examples/example_kf1_dumper`](examples/example_kf1_dumper) and [`examples/example_kf1_overlay`](examples/example_kf1_overlay).

# Features
- **Efficiency**: Optimized for performance, ensuring quick computations using AVX2.
- **Versatility**: Includes a wide array of mathematical functions and algorithms.
- **Ease of Use**: Simplified interface for convenient integration into various projects.
- **Projectile Prediction**: Projectile prediction engine with O(N) algo complexity, that can power you projectile aim-bot.
- **3D Projection**: No need to find view-projection matrix anymore you can make your own projection pipeline.
- **Collision Detection**: Production ready code to handle collision detection by using simple interfaces.
- **No Additional Dependencies**: No additional dependencies need to use OMath except unit test execution
- **Ready for meta-programming**: Omath use templates for common types like Vectors, Matrixes etc, to handle all types!
- **Engine support**: Supports coordinate systems of **Source, Rage, Unity, Unreal, Frostbite, IWEngine, CryEngine and canonical OpenGL**.
- **Cross platform**: Supports Windows, MacOS and Linux.
- **Reverse Engineering**: Typed offset access over internal or external process memory, byte pattern scanning with wildcards in ELF/Mach-O/PE files/modules and loaded processes (works even with Wine apps), and per-engine helpers to resolve names and walk the global object table - see the [Reverse Engineering Toolkit](#reverse-engineering-toolkit) section above.
- **Hooking**: `omath::hooks::HooksManager` hooks DirectX 9/11/12 and OpenGL's present/swap calls for you, so you only write the draw code.
- **Scripting**: Supports to make scripts in Lua out of box.
- **Handy**: Allow to design wall hacks in modern jetpack compose like way.
- **Battle tested**: It's already used by some big players on the market like wraith.su and bluedream.ltd
<div align = center>
 
# Gallery

<br>

[![Youtube Video](docs/images/yt_previews/img.png)](https://youtu.be/lM_NJ1yCunw?si=-Qf5yzDcWbaxAXGQ)

<br>

![APEX Preview]

<br>

![BO2 Preview]

<br>

![CS2 Preview]

<br>

![TF2 Preview]

<br>

![GTA5 Preview]

<br>

![OpenGL Preview]

<br>
<br>

</div>

## Documentation

- **[Getting Started Guide](http://libomath.org/getting_started/)** - Installation and first steps
- **[API Overview](http://libomath.org/api_overview/)** - Complete API reference
- **[Tutorials](http://libomath.org/tutorials/)** - Step-by-step guides
- **[FAQ](http://libomath.org/faq/)** - Common questions and answers
- **[Troubleshooting](http://libomath.org/troubleshooting/)** - Solutions to common issues
- **[Best Practices](http://libomath.org/best_practices/)** - Guidelines for effective usage

## Community & Support

- **Discord**: [Join our community](https://discord.gg/eDgdaWbqwZ)
- **Telegram**: [@orangennotes](https://t.me/orangennotes)
- **Issues**: [Report bugs or request features](https://github.com/orange-cpp/omath/issues)
- **Contributing**: See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines

# Acknowledgments
-  [All contributors](https://github.com/orange-cpp/omath/graphs/contributors)

<!----------------------------------{ Images }--------------------------------->
[APEX Preview]: docs/images/showcase/apex.png
[BO2 Preview]: docs/images/showcase/cod_bo2.png
[CS2 Preview]: docs/images/showcase/cs2.jpeg
[TF2 Preview]: docs/images/showcase/tf2.jpg
[GTA5 Preview]: https://i.imgur.com/W7T8RhZ.png
[OpenGL Preview]: docs/images/showcase/opengl.png
<!----------------------------------{ Buttons }--------------------------------->
[QUICKSTART]: docs/getting_started.md
[INSTALL]: INSTALL.md
[DOCUMENTATION]: http://libomath.org
[TUTORIALS]: docs/tutorials.md
[CONTRIBUTING]: CONTRIBUTING.md
[EXAMPLES]: examples
[SPONSOR]: https://boosty.to/orangecpp/purchase/3568644?ssource=DIRECT&share=subscription_link
