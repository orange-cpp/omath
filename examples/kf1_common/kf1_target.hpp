//
// Created by Vlad on 8/14/2026.
//
// Shared reader for Killing Floor 1 (KF_VERSION 1065, Unreal Engine 2.5), used by example_kf1_dumper and
// example_kf1_overlay. Everything here is pure ReadProcessMemory, nothing is injected into the game.
//
// The offsets are for Core.dll of build 1065, derived by disassembling its own accessors. A different Core.dll means
// re-deriving them, see docs/engines/unreal_engine/actor_name.md.

#pragma once
#include <Windows.h>
#include <array>
#include <cstdint>
#include <omath/engines/unreal_engine/actor_name.hpp>
#include <omath/engines/unreal_engine/constants.hpp>
#include <omath/engines/unreal_engine/object_array.hpp>
#include <optional>
#include <print>
#include <string>
#include <string_view>
#include <tlhelp32.h>
#include <vector>

namespace kf1
{
    using omath::rev_eng::shift_address;
    using omath::unreal_engine::get_actor_name;
    using omath::unreal_engine::get_object_by_index;
    using omath::unreal_engine::get_object_count;
    using omath::unreal_engine::NameLayout;
    using omath::unreal_engine::ObjectArrayLayout;
    using omath::unreal_engine::PitchAngle;
    using omath::unreal_engine::RollAngle;
    using omath::unreal_engine::ViewAngles;
    using omath::unreal_engine::YawAngle;

    // Core.dll, preferred base 0x10100000 but the loader may relocate it, so the base is resolved at runtime
    constexpr std::uintptr_t k_g_obj_objects_rva = 0x16A70C; // UObject::GObjObjects -> TArray<UObject*>
    constexpr std::uintptr_t k_g_names_rva = 0x166674; // FName::Names -> TArray<FNameEntry*>

    // Engine.dll holds the audio subsystem here, and the subsystem keeps the actor it listens from - the local pawn -
    // at +0x38, with the viewport right in front of it at +0x34. That is a static path to the local player, no sweep
    // over GObjObjects needed. It only holds while the game runs ALAudio, so the result is validated before use.
    constexpr std::uintptr_t k_audio_subsystem_rva = 0x4C6934;
    constexpr std::ptrdiff_t k_audio_listener_offset = 0x38;

    // UObject and the reflection chain it hangs off, all verified against Core.dll's own accessors
    constexpr std::ptrdiff_t k_object_class_offset = 0x28; // UObject::Class -> UClass*
    constexpr std::ptrdiff_t k_struct_super_offset = 0x2C; // UStruct::SuperField -> UStruct*
    constexpr std::ptrdiff_t k_struct_children_offset = 0x40; // UStruct::Children -> UField*
    constexpr std::ptrdiff_t k_field_next_offset = 0x30; // UField::Next -> UField*
    constexpr std::ptrdiff_t k_property_offset_offset = 0x4C; // UProperty::Offset -> int32

    // AActor, verified against AActor::GetViewRotation and ULevel::MoveActor
    constexpr std::ptrdiff_t k_actor_location_offset = 0x14C; // FVector { float X, Y, Z; }
    constexpr std::ptrdiff_t k_actor_rotation_offset = 0x158; // FRotator { int32 Pitch, Yaw, Roll; }
    constexpr std::ptrdiff_t k_actor_controller_offset = 0x360; // AController*, null when not possessed

    inline HANDLE g_process{};

    // A failed read yields a zeroed value, which silently looks like a real one. The flag lets a caller tell the two
    // apart: clear it, read a group of fields, then ask whether anything went missing before trusting the result.
    inline bool g_read_failed{};

    inline void clear_read_error()
    {
        g_read_failed = false;
    }

    [[nodiscard]]
    inline bool read_failed()
    {
        return g_read_failed;
    }

    struct RpmTrait final
    {
        template<class Type>
        [[nodiscard("You must use read value")]]
        static Type read_memory(const std::uintptr_t address) noexcept
        {
            Type value{};
            SIZE_T read{};

            if (!ReadProcessMemory(g_process, reinterpret_cast<LPCVOID>(address), &value, sizeof(Type), &read)
                || read != sizeof(Type))
                g_read_failed = true;

            return value;
        }
    };

    template<class Type>
    [[nodiscard]]
    Type read(const std::uintptr_t address)
    {
        return RpmTrait::read_memory<Type>(address);
    }

    [[nodiscard]]
    inline std::uintptr_t read_ptr(const std::uintptr_t base, const std::ptrdiff_t offset = 0)
    {
        return omath::rev_eng::read_pointer<RpmTrait>(shift_address(base, offset), sizeof(std::uint32_t));
    }

    [[nodiscard]]
    inline DWORD find_process_id(const wchar_t* process_name)
    {
        const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (snapshot == INVALID_HANDLE_VALUE)
            return 0;

        PROCESSENTRY32W entry{.dwSize = sizeof(PROCESSENTRY32W)};
        DWORD process_id{};

        for (auto found = Process32FirstW(snapshot, &entry); found; found = Process32NextW(snapshot, &entry))
        {
            if (_wcsicmp(entry.szExeFile, process_name) == 0)
            {
                process_id = entry.th32ProcessID;
                break;
            }
        }

        CloseHandle(snapshot);

        return process_id;
    }

    // TH32CS_SNAPMODULE32 is what makes the 32 bit modules of the game visible to an x64 tool
    [[nodiscard]]
    inline std::uintptr_t find_module_base(const DWORD process_id, const wchar_t* module_name)
    {
        const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);

        if (snapshot == INVALID_HANDLE_VALUE)
            return 0;

        MODULEENTRY32W entry{.dwSize = sizeof(MODULEENTRY32W)};
        std::uintptr_t module_base{};

        for (auto found = Module32FirstW(snapshot, &entry); found; found = Module32NextW(snapshot, &entry))
        {
            if (_wcsicmp(entry.szModule, module_name) == 0)
            {
                module_base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
                break;
            }
        }

        CloseHandle(snapshot);

        return module_base;
    }

    // The game's top level window, needed by the overlay to know where to sit
    [[nodiscard]]
    inline HWND find_main_window(const DWORD process_id)
    {
        struct Search
        {
            DWORD process_id;
            HWND window;
        } search{process_id, nullptr};

        EnumWindows(
                [](const HWND window, const LPARAM param) -> BOOL
                {
                    auto* state = reinterpret_cast<Search*>(param);
                    DWORD owner{};
                    GetWindowThreadProcessId(window, &owner);

                    if (owner != state->process_id || !IsWindowVisible(window) || GetWindow(window, GW_OWNER))
                        return TRUE;

                    state->window = window;

                    return FALSE;
                },
                reinterpret_cast<LPARAM>(&search));

        return search.window;
    }

    // The ue2_5() presets already carry the right 4 byte pointer width for this x86 target and match KF1 1065
    // (UObject::Name at 0x24, UTF-16 FNameEntry text at 0xC) exactly, no overrides needed
    [[nodiscard]]
    inline ObjectArrayLayout object_array_layout()
    {
        return ObjectArrayLayout::ue2_5();
    }

    [[nodiscard]]
    inline NameLayout name_layout()
    {
        return NameLayout::ue2_5();
    }

    struct Target final
    {
        DWORD process_id{};
        std::uintptr_t engine_base{};
        std::uintptr_t g_obj_objects{};
        std::uintptr_t g_names{};
        ObjectArrayLayout object_array{};
        NameLayout names{};
        std::uint32_t object_count{};
    };

    // Opens the game and resolves both globals, or explains on stdout why it could not
    [[nodiscard]]
    inline std::optional<Target> attach()
    {
        const auto process_id = find_process_id(L"KillingFloor.exe");

        if (!process_id)
        {
            std::println("[ERROR] KillingFloor.exe is not running");
            return std::nullopt;
        }

        g_process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);

        if (!g_process)
        {
            std::println("[ERROR] OpenProcess failed with {}, try running as administrator", GetLastError());
            return std::nullopt;
        }

        const auto core_base = find_module_base(process_id, L"Core.dll");

        if (!core_base)
        {
            std::println("[ERROR] Core.dll is not loaded in process {}", process_id);
            return std::nullopt;
        }

        Target target{.process_id = process_id,
                      .engine_base = find_module_base(process_id, L"Engine.dll"),
                      .g_obj_objects = core_base + k_g_obj_objects_rva,
                      .g_names = core_base + k_g_names_rva,
                      .object_array = object_array_layout(),
                      .names = name_layout()};

        const auto count = get_object_count<RpmTrait>(target.g_obj_objects, target.object_array);

        if (!count.has_value())
        {
            std::println("[ERROR] failed to read GObjObjects");
            return std::nullopt;
        }

        target.object_count = *count;

        std::println("Core.dll at 0x{:X}, GObjObjects at 0x{:X} ({} slots), GNames at 0x{:X}", core_base,
                     target.g_obj_objects, target.object_count, target.g_names);

        return target;
    }

    [[nodiscard]]
    inline std::optional<std::uintptr_t> object_at(const Target& target, const std::uint32_t index)
    {
        const auto object = get_object_by_index<RpmTrait>(target.g_obj_objects, index, target.object_array);

        // Slots of destroyed objects are empty
        if (!object.has_value())
            return std::nullopt;

        return *object;
    }

    // One RPM call for the whole TArray<UObject*> data block. UE 2.5 stores bare pointers there, so a sweep over the
    // snapshot costs one read per object instead of the three get_object_by_index needs - 40 ms instead of 126 ms for
    // the ~91k slots of a running KF1. Use get_object_by_index when you walk a handful of indices.
    [[nodiscard]]
    inline std::vector<std::uint32_t> read_object_table(const Target& target)
    {
        const auto data = read_ptr(shift_address(target.g_obj_objects, target.object_array.objects_offset));
        const auto count = get_object_count<RpmTrait>(target.g_obj_objects, target.object_array).value_or(0);

        std::vector<std::uint32_t> table(count);

        if (!data || table.empty()
            || !ReadProcessMemory(g_process, reinterpret_cast<LPCVOID>(data), table.data(),
                                  table.size() * sizeof(std::uint32_t), nullptr))
            table.clear();

        return table;
    }

    [[nodiscard]]
    inline std::string object_name(const Target& target, const std::uintptr_t object)
    {
        return get_actor_name<RpmTrait>(object, target.g_names, target.names).value_or("?");
    }

    // The walk UObject::IsA performs, the hop guard keeps corrupt data from spinning forever
    [[nodiscard]]
    inline bool is_kind_of(std::uintptr_t object_class, const std::uintptr_t wanted_class)
    {
        for (auto hops = 0; object_class && hops < 64; ++hops)
        {
            if (object_class == wanted_class)
                return true;

            object_class = read_ptr(object_class, k_struct_super_offset);
        }

        return false;
    }

    struct ReflectionClasses final
    {
        std::uintptr_t actor{};
        std::uintptr_t player_controller{};
        std::uintptr_t pawn{};
        std::uintptr_t property{};

        [[nodiscard]]
        bool complete() const
        {
            return actor && player_controller && pawn && property;
        }
    };

    // Resolves UClass objects by name instead of trusting an Engine.dll address, since that DLL does get relocated.
    // An object named "PlayerController" whose own class is named "Class" is the class itself, not an instance.
    // This is the one expensive scan, every object name has to be read to run it, so do it once and keep the result.
    [[nodiscard]]
    inline ReflectionClasses find_reflection_classes(const Target& target)
    {
        ReflectionClasses classes;

        for (std::uint32_t index = 0; index < target.object_count; ++index)
        {
            const auto object = object_at(target, index);

            if (!object.has_value())
                continue;

            const auto name = get_actor_name<RpmTrait>(*object, target.g_names, target.names);

            if (!name.has_value())
                continue;

            if (*name != "Actor" && *name != "PlayerController" && *name != "Pawn" && *name != "Property")
                continue;

            if (object_name(target, read_ptr(*object, k_object_class_offset)) != "Class")
                continue;

            if (*name == "Actor")
                classes.actor = *object;
            else if (*name == "PlayerController")
                classes.player_controller = *object;
            else if (*name == "Pawn")
                classes.pawn = *object;
            else
                classes.property = *object;

            if (classes.complete())
                break;
        }

        return classes;
    }

    // Walks the class chain, and on every level the Children/Next sibling list, exactly like UStruct::Link does
    [[nodiscard]]
    inline std::optional<std::int32_t> find_property_offset(const Target& target, std::uintptr_t owner_class,
                                                            const std::string_view property_name,
                                                            const std::uintptr_t property_class)
    {
        for (auto class_hops = 0; owner_class && class_hops < 32; ++class_hops)
        {
            auto field = read_ptr(owner_class, k_struct_children_offset);

            for (auto field_hops = 0; field && field_hops < 4096; ++field_hops)
            {
                if (object_name(target, field) == property_name
                    && is_kind_of(read_ptr(field, k_object_class_offset), property_class))
                    return read<std::int32_t>(shift_address(field, k_property_offset_offset));

                field = read_ptr(field, k_field_next_offset);
            }

            owner_class = read_ptr(owner_class, k_struct_super_offset);
        }

        return std::nullopt;
    }

    // Diagnostic for when a property lookup comes up empty: shows what the class chain actually declares
    inline void list_properties(const Target& target, std::uintptr_t owner_class, const std::uintptr_t property_class,
                                const int limit)
    {
        auto shown = 0;

        for (auto class_hops = 0; owner_class && class_hops < 32 && shown < limit; ++class_hops)
        {
            auto field = read_ptr(owner_class, k_struct_children_offset);

            for (auto field_hops = 0; field && field_hops < 4096 && shown < limit; ++field_hops)
            {
                if (is_kind_of(read_ptr(field, k_object_class_offset), property_class))
                {
                    std::println("              {}::{}", object_name(target, owner_class), object_name(target, field));
                    ++shown;
                }

                field = read_ptr(field, k_field_next_offset);
            }

            owner_class = read_ptr(owner_class, k_struct_super_offset);
        }
    }

    [[nodiscard]]
    inline omath::Vector3<double> read_location(const std::uintptr_t actor)
    {
        const auto location = read<std::array<float, 3>>(shift_address(actor, k_actor_location_offset));

        return {location[0], location[1], location[2]};
    }

    // Unreal rotators count 65536 units per full turn, wrap them into [-180, 180) before handing them to omath
    [[nodiscard]]
    inline double rotator_units_to_degrees(const std::int32_t units)
    {
        const auto wrapped = static_cast<std::int32_t>(static_cast<std::uint32_t>(units) & 0xFFFF);

        return (wrapped >= 32768 ? wrapped - 65536 : wrapped) * 360.0 / 65536.0;
    }

    [[nodiscard]]
    inline ViewAngles read_view_angles(const std::uintptr_t actor,
                                       const std::ptrdiff_t offset = k_actor_rotation_offset)
    {
        const auto rotator = read<std::array<std::int32_t, 3>>(shift_address(actor, offset));

        return {PitchAngle::from_degrees(rotator_units_to_degrees(rotator[0])),
                YawAngle::from_degrees(rotator_units_to_degrees(rotator[1])),
                RollAngle::from_degrees(rotator_units_to_degrees(rotator[2]))};
    }

    // A possessed pawn points its Controller back at the controller. The class check matters: plenty of unrelated
    // objects keep a pointer at 0x360 too, a HUD for example.
    [[nodiscard]]
    inline std::uintptr_t find_possessed_pawn(const Target& target, const std::uintptr_t pawn_class,
                                              const std::uintptr_t controller)
    {
        for (std::uint32_t index = 0; index < target.object_count; ++index)
        {
            const auto object = object_at(target, index);

            if (object.has_value() && is_kind_of(read_ptr(*object, k_object_class_offset), pawn_class)
                && read_ptr(*object, k_actor_controller_offset) == controller)
                return *object;
        }

        return 0;
    }

    struct LocalPlayer final
    {
        std::uintptr_t controller{};
        std::uintptr_t pawn{};
        // How many PlayerControllers the scan walked past, 0 when the static shortcut resolved the player instead
        std::size_t controller_count{};

        // Resolved by name once, on the controller and on the pawn respectively
        std::optional<std::int32_t> fov_offset;
        std::optional<std::int32_t> eye_height_offset;

        // PlayerController::CalcViewRotation, the rotation the engine renders the view with. Actor::Rotation would do
        // as well were it not for KF parking a foreign rotation in it every few frames - measured at 33 of 400
        // samples while moving, against 0 for CalcViewRotation.
        std::optional<std::int32_t> view_rotation_offset;

        [[nodiscard]]
        bool valid() const
        {
            return controller != 0;
        }
    };

    inline void resolve_player_properties(const Target& target, const ReflectionClasses& classes, LocalPlayer& player)
    {
        const auto controller_class = read_ptr(player.controller, k_object_class_offset);

        player.fov_offset = find_property_offset(target, controller_class, "FovAngle", classes.property);
        player.view_rotation_offset =
                find_property_offset(target, controller_class, "CalcViewRotation", classes.property);

        if (player.pawn)
            player.eye_height_offset = find_property_offset(target, read_ptr(player.pawn, k_object_class_offset),
                                                            "EyeHeight", classes.property);
    }

    // Full scan for the local player. KF1 keeps a cinematic PlayerController alive for the menu backdrop, and a listen
    // server has one per player, so the interesting one is whichever actually possesses a Pawn.
    [[nodiscard]]
    inline LocalPlayer find_local_player_by_scan(const Target& target, const ReflectionClasses& classes)
    {
        std::vector<std::uintptr_t> controllers;

        for (std::uint32_t index = 0; index < target.object_count; ++index)
        {
            const auto object = object_at(target, index);

            if (object.has_value() && is_kind_of(read_ptr(*object, k_object_class_offset), classes.player_controller))
                controllers.push_back(*object);
        }

        if (controllers.empty())
            return {};

        LocalPlayer player{.controller = controllers.front(), .controller_count = controllers.size()};

        for (const auto controller : controllers)
        {
            const auto possessed = find_possessed_pawn(target, classes.pawn, controller);

            if (!possessed)
                continue;

            player.controller = controller;
            player.pawn = possessed;
            break;
        }

        resolve_player_properties(target, classes, player);

        return player;
    }

    // The audio subsystem's listener is the local pawn, and the pawn points back at its controller, so the whole
    // local player resolves in three reads instead of a sweep over every object. Both ends are class checked, and
    // anything unexpected - no pawn yet, a different audio subsystem, spectating - falls back to the scan.
    [[nodiscard]]
    inline LocalPlayer find_local_player(const Target& target, const ReflectionClasses& classes)
    {
        const auto audio_subsystem = read_ptr(target.engine_base + k_audio_subsystem_rva);
        const auto pawn = audio_subsystem ? read_ptr(audio_subsystem, k_audio_listener_offset) : 0;
        const auto controller = pawn ? read_ptr(pawn, k_actor_controller_offset) : 0;

        if (!controller || !is_kind_of(read_ptr(pawn, k_object_class_offset), classes.pawn)
            || !is_kind_of(read_ptr(controller, k_object_class_offset), classes.player_controller))
            return find_local_player_by_scan(target, classes);

        LocalPlayer player{.controller = controller, .pawn = pawn};
        resolve_player_properties(target, classes, player);

        return player;
    }

    // Cheap enough to run every frame, unlike find_local_player
    [[nodiscard]]
    inline bool still_valid(const ReflectionClasses& classes, const LocalPlayer& player)
    {
        if (!player.valid())
            return false;

        if (!is_kind_of(read_ptr(player.controller, k_object_class_offset), classes.player_controller))
            return false;

        return !player.pawn || read_ptr(player.pawn, k_actor_controller_offset) == player.controller;
    }

    struct CameraState final
    {
        omath::Vector3<double> position;
        ViewAngles angles;
        double fov{90.};
    };

    // Where the engine puts the player's view: the pawn's location lifted by its eye height, angles from the
    // controller. Without a pawn the controller's own location is the best guess.
    [[nodiscard]]
    inline CameraState read_camera(const LocalPlayer& player)
    {
        CameraState state{.position = read_location(player.pawn ? player.pawn : player.controller),
                          .angles = read_view_angles(player.controller,
                                                     player.view_rotation_offset.value_or(k_actor_rotation_offset))};

        if (player.pawn && player.eye_height_offset.has_value())
            state.position.z += read<float>(shift_address(player.pawn, *player.eye_height_offset));

        if (player.fov_offset.has_value())
            state.fov = read<float>(shift_address(player.controller, *player.fov_offset));

        return state;
    }
} // namespace kf1
