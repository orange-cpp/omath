//
// Created by Vlad on 8/14/2026.
//
// Killing Floor 1 (KF_VERSION 1065, Unreal Engine 2.5) GObjects dumper.
//
// Unlike an injected DLL this one stays outside of the game: it opens KillingFloor.exe with OpenProcess and reads it
// through ReadProcessMemory, so omath walks the target's memory through kf1::RpmTrait. KillingFloor.exe is an x86
// process, which is why the layouts in kf1_target.hpp use a 4 byte pointer_size - that makes the dumper work no
// matter whether you build it as x86 or x64.
//
// It prints the local player camera first, then every object in GObjObjects.
//
// Usage: example_kf1_dumper [filter]
// The optional filter keeps only objects whose name or class contains it, e.g. "Pawn".

#include "kf1_common/kf1_target.hpp"
#include <omath/engines/unreal_engine/formulas.hpp>
#include <print>
#include <string_view>
#include <unordered_map>

namespace
{
    void dump_camera(const kf1::Target& target)
    {
        std::println("---- local player camera ----");

        const auto classes = kf1::find_reflection_classes(target);

        if (!classes.complete())
        {
            std::println("no PlayerController/Pawn/Property class in GObjObjects, get past the main menu first\n");
            return;
        }

        const auto player = kf1::find_local_player(target, classes);

        if (!player.valid())
        {
            std::println("no PlayerController instance found, are you in a level?\n");
            return;
        }

        const auto camera = kf1::read_camera(player);
        const auto forward = omath::unreal_engine::forward_vector(camera.angles);

        if (player.controller_count > 1)
            std::println("note        {} PlayerControllers alive", player.controller_count);

        std::println("controller  0x{:08X} {}", player.controller, kf1::object_name(target, player.controller));

        if (player.pawn)
            std::println("pawn        0x{:08X} {}", player.pawn, kf1::object_name(target, player.pawn));
        else
            std::println("pawn        none, falling back to the controller's own location");

        std::println("position    x {:.2f}  y {:.2f}  z {:.2f}", camera.position.x, camera.position.y,
                     camera.position.z);
        std::println("view angles pitch {:.2f}  yaw {:.2f}  roll {:.2f}", camera.angles.pitch.as_degrees(),
                     camera.angles.yaw.as_degrees(), camera.angles.roll.as_degrees());
        std::println("forward     x {:.3f}  y {:.3f}  z {:.3f}", forward.x, forward.y, forward.z);

        // Looked up by name rather than hardcoded, so a different build moving the field changes nothing here
        if (player.fov_offset.has_value())
            std::println("fov         {:.2f} (UProperty::Offset 0x{:X}, resolved by name)", camera.fov,
                         *player.fov_offset);
        else
        {
            std::println("fov         no property named FovAngle, the class chain declares:");
            kf1::list_properties(target, kf1::read_ptr(player.controller, kf1::k_object_class_offset), classes.property,
                                 40);
        }

        std::println("");
    }

    void dump_objects(const kf1::Target& target, const std::string_view filter)
    {
        std::println("---- objects ----");

        // A name costs one read per character, and the few hundred distinct classes repeat over and over
        std::unordered_map<std::uintptr_t, std::string> class_names;
        std::size_t printed{};

        for (std::uint32_t index = 0; index < target.object_count; ++index)
        {
            const auto object = kf1::object_at(target, index);

            if (!object.has_value())
                continue;

            const auto name = kf1::get_actor_name<kf1::RpmTrait>(*object, target.g_names, target.names);

            if (!name.has_value())
                continue;

            const auto object_class = kf1::read_ptr(*object, kf1::k_object_class_offset);
            auto& class_name = class_names[object_class];

            if (class_name.empty())
                class_name = kf1::object_name(target, object_class);

            if (!filter.empty() && !name->contains(filter) && !class_name.contains(filter))
                continue;

            std::println("[{:6}] 0x{:08X} {:<32} {}", index, *object, class_name, *name);
            ++printed;
        }

        std::println("\nprinted {} objects", printed);
    }
} // namespace

int main(const int argc, char* argv[])
{
    const std::string_view filter = argc > 1 ? argv[1] : "";
    const auto target = kf1::attach();

    if (!target.has_value())
        return -1;

    dump_camera(*target);
    dump_objects(*target, filter);

    CloseHandle(kf1::g_process);

    return 0;
}
