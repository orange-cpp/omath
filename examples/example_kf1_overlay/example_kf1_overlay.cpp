//
// Created by Vlad on 8/14/2026.
//
// Click through overlay for Killing Floor 1 (KF_VERSION 1065, Unreal Engine 2.5).
//
// A transparent, always on top GLFW window is pinned to the game's client area and ImGui draws into it. The camera is
// read out of the game with ReadProcessMemory (see kf1_common/kf1_target.hpp), handed to omath's Unreal camera, and
// both the world origin {0, 0, 0} and every selected actor are projected with world_to_screen.
//
// INSERT opens the picker, where the actor classes present in the level can be ticked on and off, END closes the
// overlay. The overlay never takes focus, not even with the picker open, because UE 2.5 minimizes a fullscreen game
// as soon as it loses focus - so the picker is driven with the mouse alone and the game stays up. Both hotkeys are
// ignored unless the game is in front or the picker is open, which keeps an END pressed in some other application
// from closing the overlay.
//
// Nothing is injected and nothing is written to the game, this is a read only observer.

#include "kf1_common/kf1_target.hpp"
#include <GLFW/glfw3.h>
#include <chrono>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <map>
#include <omath/algorithm/radar.hpp>
#include <omath/engines/unreal_engine/camera.hpp>
#include <print>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#define GLFW_EXPOSE_NATIVE_WIN32
#include "omath/hud/hud_renderer_interface.hpp"
#include "omath/hud/screen_overlay.hpp"
#include <GLFW/glfw3native.h>

using omath::projection::FieldOfView;
using omath::projection::ViewPort;
using omath::unreal_engine::Camera;

namespace
{
    constexpr auto k_near_plane = 1.;
    constexpr auto k_far_plane = 32768.;

    // A full sweep of the object table costs about 40 ms, so it is spread over frames instead of stalling one. This
    // budget keeps a sweep at roughly 4 ms per frame and refreshes the actor list about five times a second.
    constexpr std::size_t k_scan_budget = 8192;
    constexpr std::size_t k_max_tracked = 512;

    constexpr auto k_radar_radius = 110.f;
    constexpr auto k_radar_margin = 24.f;

    struct WindowRect final
    {
        int x{};
        int y{};
        int width{};
        int height{};

        [[nodiscard]]
        bool operator==(const WindowRect&) const = default;
    };

    struct ClassInfo final
    {
        std::string name;
        bool is_actor{};
    };

    struct TrackedActor final
    {
        std::uintptr_t address{};
        std::string name;
        std::string class_name;
    };

    // Walks the object table a slice at a time, collecting the actors whose class was ticked in the picker and
    // counting every actor class it meets, which is what fills the picker list
    class ActorTracker final
    {
    public:
        void advance(const kf1::Target& target, const kf1::ReflectionClasses& classes,
                     const std::unordered_set<std::string>& selected)
        {
            if (m_cursor >= m_table.size())
                start_sweep(target);

            const auto end = std::min(m_cursor + k_scan_budget, m_table.size());

            for (; m_cursor < end; ++m_cursor)
            {
                const std::uintptr_t object = m_table[m_cursor];

                if (!object)
                    continue;

                const auto object_class = kf1::read_ptr(object, kf1::k_object_class_offset);

                if (!object_class)
                    continue;

                const auto& info = class_info(target, classes, object_class);

                if (!info.is_actor)
                    continue;

                ++m_counts[info.name];

                // The object name is read once per sweep here, reading it per frame would cost far more
                if (m_in_progress.size() < k_max_tracked && selected.contains(info.name))
                    m_in_progress.push_back({object, kf1::object_name(target, object), info.name});
            }
        }

        [[nodiscard]]
        const std::vector<TrackedActor>& tracked() const
        {
            return m_tracked;
        }

        [[nodiscard]]
        const std::map<std::string, std::uint32_t>& actor_classes() const
        {
            return m_published_counts;
        }

    private:
        void start_sweep(const kf1::Target& target)
        {
            m_tracked = std::move(m_in_progress);
            m_in_progress.clear();
            m_published_counts = std::move(m_counts);
            m_counts.clear();
            m_table = kf1::read_object_table(target);
            m_cursor = 0;
        }

        // Class objects never move, so a class is named and classified once and then only looked up
        [[nodiscard]]
        const ClassInfo& class_info(const kf1::Target& target, const kf1::ReflectionClasses& classes,
                                    const std::uintptr_t object_class)
        {
            const auto known = m_classes.find(object_class);

            if (known != m_classes.end())
                return known->second;

            return m_classes
                    .emplace(object_class, ClassInfo{kf1::object_name(target, object_class),
                                                     kf1::is_kind_of(object_class, classes.actor)})
                    .first->second;
        }

        std::vector<std::uint32_t> m_table;
        std::size_t m_cursor{};
        std::unordered_map<std::uintptr_t, ClassInfo> m_classes;
        std::map<std::string, std::uint32_t> m_counts;
        std::map<std::string, std::uint32_t> m_published_counts;
        std::vector<TrackedActor> m_in_progress;
        std::vector<TrackedActor> m_tracked;
    };

    // Where the game draws, in screen coordinates - that is exactly the area the overlay has to cover for a
    // projection made with the game's own viewport to line up
    [[nodiscard]]
    WindowRect client_rect_on_screen(const HWND window)
    {
        RECT client{};
        POINT top_left{};

        if (!GetClientRect(window, &client) || !ClientToScreen(window, &top_left))
            return {};

        return {top_left.x, top_left.y, client.right - client.left, client.bottom - client.top};
    }

    [[nodiscard]]
    GLFWwindow* create_overlay_window(const WindowRect& rect)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
        glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        auto* window = glfwCreateWindow(rect.width, rect.height, "omath kf1 overlay", nullptr, nullptr);

        if (!window)
            return nullptr;

        glfwSetWindowPos(window, rect.x, rect.y);
        glfwShowWindow(window);

        return window;
    }

    // Click through while watching, click catching while the picker is open. WS_EX_NOACTIVATE stays on either way and
    // the overlay never calls SetForegroundWindow: UE 2.5 minimizes a fullscreen game the moment it loses focus, which
    // would take the game and the overlay with it off the screen. A window that never activates still receives mouse
    // messages, so the picker stays usable while the game keeps believing it is in front.
    void set_click_through(GLFWwindow* window, const bool click_through)
    {
        glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, click_through ? GLFW_TRUE : GLFW_FALSE);

        const auto native = glfwGetWin32Window(window);
        auto style = GetWindowLongPtrW(native, GWL_EXSTYLE) | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW;

        if (click_through)
            style |= WS_EX_TRANSPARENT;
        else
            style &= ~static_cast<LONG_PTR>(WS_EX_TRANSPARENT);

        SetWindowLongPtrW(native, GWL_EXSTYLE, style);
    }

    [[nodiscard]]
    bool key_pressed(const int key, bool& was_down)
    {
        const auto down = (GetAsyncKeyState(key) & 0x8000) != 0;
        const auto pressed = down && !was_down;
        was_down = down;

        return pressed;
    }

    void draw_origin_marker(const ImVec2& position, const double distance)
    {
        auto* canvas = ImGui::GetBackgroundDrawList();

        canvas->AddCircleFilled(position, 4.f, IM_COL32(255, 170, 0, 255));
        canvas->AddCircle(position, 14.f, IM_COL32(255, 170, 0, 200), 0, 2.f);
        canvas->AddLine({position.x - 24.f, position.y}, {position.x - 16.f, position.y}, IM_COL32(255, 170, 0, 200),
                        2.f);
        canvas->AddLine({position.x + 16.f, position.y}, {position.x + 24.f, position.y}, IM_COL32(255, 170, 0, 200),
                        2.f);
        canvas->AddText({position.x + 20.f, position.y - 8.f}, IM_COL32(255, 255, 255, 255),
                        std::format("world origin {{0, 0, 0}}\n{:.0f} units away", distance).c_str());
    }

    void draw_actor_marker(const ImVec2& position, const TrackedActor& actor, const double distance)
    {
        auto* canvas = ImGui::GetBackgroundDrawList();
        const auto label = std::format("{}\n{}  {:.0f}u", actor.name, actor.class_name, distance);
        const auto size = ImGui::CalcTextSize(label.c_str());

        canvas->AddCircleFilled(position, 3.f, IM_COL32(80, 220, 120, 255));
        canvas->AddRectFilled({position.x - size.x * .5f - 4.f, position.y + 6.f},
                              {position.x + size.x * .5f + 4.f, position.y + 10.f + size.y}, IM_COL32(0, 0, 0, 130),
                              3.f);
        canvas->AddText({position.x - size.x * .5f, position.y + 8.f}, IM_COL32(80, 220, 120, 255), label.c_str());
    }

    // The radar sits in the top right corner of the game's client area, north on it is wherever the player looks
    [[nodiscard]]
    ImVec2 radar_center(const WindowRect& rect)
    {
        return {static_cast<float>(rect.width) - k_radar_margin - k_radar_radius, k_radar_margin + k_radar_radius};
    }

    void draw_radar_background(const WindowRect& rect, const float range)
    {
        auto* canvas = ImGui::GetBackgroundDrawList();
        const auto center = radar_center(rect);

        canvas->AddCircleFilled(center, k_radar_radius, IM_COL32(0, 0, 0, 120));
        canvas->AddCircle(center, k_radar_radius, IM_COL32(210, 210, 210, 90), 0, 1.5f);
        canvas->AddCircle(center, k_radar_radius * .5f, IM_COL32(210, 210, 210, 40), 0, 1.f);
        canvas->AddLine({center.x - k_radar_radius, center.y}, {center.x + k_radar_radius, center.y},
                        IM_COL32(210, 210, 210, 40));
        canvas->AddLine({center.x, center.y - k_radar_radius}, {center.x, center.y + k_radar_radius},
                        IM_COL32(210, 210, 210, 40));

        // The player always sits in the middle looking up the screen
        canvas->AddTriangleFilled({center.x, center.y - 7.f}, {center.x - 5.f, center.y + 5.f},
                                  {center.x + 5.f, center.y + 5.f}, IM_COL32(255, 255, 255, 220));
        canvas->AddText({center.x - k_radar_radius, center.y + k_radar_radius + 4.f}, IM_COL32(210, 210, 210, 140),
                        std::format("radar {:.0f}u", range).c_str());
    }

    // omath turns a world position into an offset from the radar centre: forward is -y, right is +x, which is already
    // what ImGui wants. Anything past the radar range is pinned to the rim so its direction still shows.
    void draw_radar_blip(const WindowRect& rect, const Camera& camera, const omath::Vector3<double>& position,
                         const float range, const ImU32 color)
    {
        auto offset = omath::algorithm::world_to_radar(camera, position, static_cast<double>(k_radar_radius / range));
        const auto length = offset.length();
        const auto pinned = length > k_radar_radius;

        if (pinned && length > 0.f)
            offset *= k_radar_radius / length;

        const auto center = radar_center(rect);

        ImGui::GetBackgroundDrawList()->AddCircleFilled({center.x + offset.x, center.y + offset.y}, pinned ? 2.f : 3.f,
                                                        color);
    }

    void draw_status(const std::string& text)
    {
        auto* canvas = ImGui::GetBackgroundDrawList();

        canvas->AddRectFilled({12.f, 12.f}, {392.f, 152.f}, IM_COL32(0, 0, 0, 140), 6.f);
        canvas->AddText({24.f, 24.f}, IM_COL32(220, 220, 220, 255), text.c_str());
    }

    void draw_picker(const ActorTracker& tracker, std::unordered_set<std::string>& selected, bool& radar_enabled,
                     float& radar_range)
    {
        ImGui::SetNextWindowSize({340.f, 500.f}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos({20.f, 170.f}, ImGuiCond_FirstUseEver);
        ImGui::Begin("actor picker");

        ImGui::TextUnformatted("Tick the classes to draw. The list holds every");
        ImGui::TextUnformatted("actor class currently alive in the level.");
        ImGui::TextUnformatted("Mouse only: the overlay never takes focus, so a");
        ImGui::TextUnformatted("fullscreen game will not minimize itself.");
        ImGui::Separator();
        ImGui::Checkbox("radar", &radar_enabled);
        ImGui::SetNextItemWidth(150.f);
        ImGui::SliderFloat("range", &radar_range, 500.f, 8000.f, "%.0f units");
        ImGui::Separator();

        if (ImGui::Button("clear selection"))
            selected.clear();

        ImGui::SameLine();
        ImGui::Text("%d selected", static_cast<int>(selected.size()));
        ImGui::Separator();
        ImGui::BeginChild("class list");

        for (const auto& [name, count] : tracker.actor_classes())
        {
            auto ticked = selected.contains(name);

            // Everything after ### is the widget id, so a count ticking up between frames cannot swallow a click
            if (ImGui::Checkbox(std::format("{} ({})###{}", name, count, name).c_str(), &ticked))
            {
                if (ticked)
                    selected.insert(name);
                else
                    selected.erase(name);
            }
        }

        ImGui::EndChild();
        ImGui::End();
    }
} // namespace

int main()
{
    const auto target = kf1::attach();

    if (!target.has_value())
        return -1;

    const auto game_window = kf1::find_main_window(target->process_id);

    if (!game_window)
    {
        std::println("[ERROR] KillingFloor.exe has no visible window");
        return -1;
    }

    std::println("resolving reflection classes, this walks every object name once...");

    const auto classes = kf1::find_reflection_classes(*target);

    if (!classes.complete())
    {
        std::println("[ERROR] no Actor/PlayerController/Pawn/Property class, get past the main menu and start the "
                     "overlay again");
        return -1;
    }

    auto player = kf1::find_local_player(*target, classes);

    if (!glfwInit())
    {
        std::println("[ERROR] glfwInit failed");
        return -1;
    }

    auto overlay_rect = client_rect_on_screen(game_window);

    // A minimized game has no client area to match, start full screen sized and let the follow logic below fix it up
    if (overlay_rect.width <= 0 || overlay_rect.height <= 0)
        overlay_rect = {0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};

    auto* window = create_overlay_window(overlay_rect);

    if (!window)
    {
        std::println("[ERROR] failed to create the overlay window");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    set_click_through(window, true);

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // No imgui.ini next to the binary, the picker rebuilds its layout every run anyway
    ImGui::GetIO().IniFilename = nullptr;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    std::println("overlay is up, INSERT opens the actor picker (mouse driven, the game keeps focus), END closes the "
                 "overlay while the game is in front or the picker is open");

    ActorTracker tracker;
    std::unordered_set<std::string> selected;
    auto picker_open = false;
    auto radar_enabled = true;
    auto radar_range = 3000.f;
    auto insert_was_down = false;
    auto end_was_down = false;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // GetAsyncKeyState is global, so the edges are always consumed, but they only count while the game is in front
        // or the picker is up - otherwise END typed in another application would close the overlay
        const auto end_pressed = key_pressed(VK_END, end_was_down);
        const auto insert_pressed = key_pressed(VK_INSERT, insert_was_down);
        const auto ours = GetForegroundWindow() == game_window || picker_open;

        if (ours && end_pressed)
            break;

        if (ours && insert_pressed)
        {
            picker_open = !picker_open;
            set_click_through(window, !picker_open);
        }

        // Follow the game window, it can be moved, resized or switched between windowed and fullscreen
        if (const auto rect = client_rect_on_screen(game_window); rect != overlay_rect && rect.width > 0)
        {
            overlay_rect = rect;
            glfwSetWindowPos(window, rect.x, rect.y);
            glfwSetWindowSize(window, rect.width, rect.height);
        }

        // A minimized game has nothing to draw over, but the picker has to stay reachable: should the game minimize
        // itself anyway, hiding the overlay would take the picker away with no way to get it back
        if (IsIconic(game_window) && !picker_open)
        {
            glfwHideWindow(window);
            // Nothing gets swapped while hidden, so idle deliberately instead of spinning on vsync
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        glfwShowWindow(window);

        // Cheap validation every frame, the expensive rescan only happens once the player object is gone, which is
        // what a level change or a respawn looks like from out here
        if (!kf1::still_valid(classes, player))
            player = kf1::find_local_player(*target, classes);

        tracker.advance(*target, classes, selected);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (player.valid())
        {
            const auto state = kf1::read_camera(player);
            const auto view_port =
                    ViewPort{static_cast<float>(overlay_rect.width), static_cast<float>(overlay_rect.height)};

            // Everything the projection needs comes straight out of the game: origin, angles, and the horizontal FOV
            // that UE keeps in PlayerController::FovAngle
            const Camera camera{state.position, state.angles,
                                view_port,      FieldOfView::from_degrees(static_cast<float>(state.fov)),
                                k_near_plane,   k_far_plane};

            if (radar_enabled)
                draw_radar_background(overlay_rect, radar_range);

            constexpr omath::Vector3 world_origin{0., 0., 0.};
            const auto origin_screen = camera.world_to_screen(world_origin);

            if (origin_screen.has_value())
                draw_origin_marker({static_cast<float>(origin_screen->x), static_cast<float>(origin_screen->y)},
                                   state.position.distance_to(world_origin));

            if (radar_enabled)
                draw_radar_blip(overlay_rect, camera, world_origin, radar_range, IM_COL32(255, 170, 0, 255));

            auto drawn = 0;

            // One position read per actor feeds both the screen marker and the radar blip
            for (const auto& actor : tracker.tracked())
            {
                const auto position = kf1::read_location(actor.address);

                if (radar_enabled)
                    draw_radar_blip(overlay_rect, camera, position, radar_range, IM_COL32(80, 220, 120, 255));

                const auto screen = camera.world_to_screen(position);

                if (!screen.has_value())
                    continue;

                draw_actor_marker({static_cast<float>(screen->x), static_cast<float>(screen->y)}, actor,
                                  state.position.distance_to(position));
                ++drawn;
            }

            draw_status(
                    std::format("kf1 overlay {}x{}   INSERT picker, END quit\n"
                                "camera  {:.0f} {:.0f} {:.0f}\n"
                                "angles  pitch {:.1f}  yaw {:.1f}  roll {:.1f}\n"
                                "fov     {:.1f}\n"
                                "origin  {}\n"
                                "actors  {}{} tracked, {} on screen, {} classes selected",
                                overlay_rect.width, overlay_rect.height, state.position.x, state.position.y,
                                state.position.z, state.angles.pitch.as_degrees(), state.angles.yaw.as_degrees(),
                                state.angles.roll.as_degrees(), state.fov,
                                origin_screen.has_value()
                                        ? std::format("{:.0f}, {:.0f} on screen", origin_screen->x, origin_screen->y)
                                        : "off screen",
                                tracker.tracked().size(), tracker.tracked().size() >= k_max_tracked ? " (capped)" : "",
                                drawn, selected.size()));
        }
        else
            draw_status("kf1 overlay\nno local player, waiting for a level");

        if (picker_open)
            draw_picker(tracker, selected, radar_enabled, radar_range);

        ImGui::Render();

        int framebuffer_width{};
        int framebuffer_height{};
        glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
        glViewport(0, 0, framebuffer_width, framebuffer_height);
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    CloseHandle(kf1::g_process);

    return 0;
}
