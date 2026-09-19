// Launcher, MuzzleOffset and the parts of ProjPredEngineInterface that do not need a real solver behind them
#include <gtest/gtest.h>
#include <omath/projectile_prediction/launcher.hpp>
#include <omath/projectile_prediction/proj_pred_engine.hpp>
#include <omath/projectile_prediction/proj_pred_engine_legacy.hpp>
#include <optional>
#include <type_traits>
#include <utility>

namespace
{
    using omath::Vector3;
    using omath::projectile_prediction::AimAngles;
    using omath::projectile_prediction::AimSolution;
    using omath::projectile_prediction::Launcher;
    using omath::projectile_prediction::MuzzleOffset;
    using omath::projectile_prediction::Projectile;
    using omath::projectile_prediction::ProjPredEngineInterface;
    using omath::projectile_prediction::ProjPredEngineLegacy;
    using omath::projectile_prediction::Target;
    using omath::projectile_prediction::ViewBasis;

    constexpr ViewBasis<float> k_world_axes{.forward = {1, 0, 0}, .right = {0, 1, 0}, .up = {0, 0, 1}};

    TEST(Launcher, DefaultsCarryNoOffsets)
    {
        constexpr Launcher<float> launcher{.eye_origin = {1, 2, 3}};

        static_assert(launcher.muzzle_offset.is_zero());
        static_assert(launcher.world_offset == Vector3<float>{0, 0, 0});
        static_assert(launcher.launch_pitch_offset == 0.f);
        static_assert(launcher.launch_origin(k_world_axes) == Vector3<float>{1, 2, 3});
        SUCCEED();
    }

    TEST(Launcher, LaunchOriginAddsEveryPart)
    {
        constexpr Launcher<float> launcher{.eye_origin = {1, 2, 3},
                                           .muzzle_offset = {.forward = 10, .right = 20, .up = 30},
                                           .world_offset = {100, 200, 300}};

        static_assert(launcher.launch_origin(k_world_axes) == Vector3<float>{111, 222, 333});
        SUCCEED();
    }

    TEST(Launcher, MuzzleOffsetFollowsTheBasisAndWorldOffsetDoesNot)
    {
        // "16 ahead, 8 to the right, 6 down" is a different world position once the view has turned; the world part
        // stays where it was.
        constexpr ViewBasis<float> turned{.forward = {0, 1, 0}, .right = {1, 0, 0}, .up = {0, 0, 1}};
        constexpr Launcher<float> launcher{.eye_origin = {0, 0, 64},
                                           .muzzle_offset = {.forward = 16, .right = 8, .up = -6},
                                           .world_offset = {0, 0, -10}};

        static_assert(launcher.launch_origin(k_world_axes) == Vector3<float>{16, 8, 48});
        static_assert(launcher.launch_origin(turned) == Vector3<float>{8, 16, 48});
        SUCCEED();
    }

    TEST(Launcher, PitchOffsetDoesNotMoveTheMuzzle)
    {
        // It tilts the round, not the weapon: the launch origin only ever depends on the basis handed in
        constexpr Launcher<float> plain{.eye_origin = {0, 0, 64},
                                        .muzzle_offset = {.forward = 16, .right = 8, .up = -6}};
        constexpr Launcher<float> tilted{.eye_origin = {0, 0, 64},
                                         .muzzle_offset = {.forward = 16, .right = 8, .up = -6},
                                         .launch_pitch_offset = 9.46f};

        static_assert(plain.launch_origin(k_world_axes) == tilted.launch_origin(k_world_axes));
        SUCCEED();
    }

    TEST(Launcher, MuzzleOffsetIsZeroOnlyWhenEveryComponentIs)
    {
        static_assert(MuzzleOffset<float>{}.is_zero());
        static_assert(MuzzleOffset<float>{.forward = -0.f}.is_zero());
        static_assert(!MuzzleOffset<float>{.forward = 1e-6f}.is_zero());
        static_assert(!MuzzleOffset<float>{.right = -8.f}.is_zero());
        static_assert(!MuzzleOffset<float>{.up = 0.5f}.is_zero());
        static_assert(!MuzzleOffset<double>{.up = 1e-300}.is_zero());
        SUCCEED();
    }

    TEST(Launcher, WorksInDoublePrecision)
    {
        constexpr ViewBasis<double> axes{.forward = {1, 0, 0}, .right = {0, 1, 0}, .up = {0, 0, 1}};
        constexpr Launcher<double> launcher{.eye_origin = {0.5, 0.25, 0.125},
                                            .muzzle_offset = {.forward = 1, .right = 2, .up = 4}};

        static_assert(launcher.launch_origin(axes) == Vector3<double>{1.5, 2.25, 4.125});
        SUCCEED();
    }

    TEST(Launcher, InputAndOutputTypesAreAggregates)
    {
        // Designated initialisers are the intended way to fill these in
        static_assert(std::is_aggregate_v<Launcher<float>>);
        static_assert(std::is_aggregate_v<MuzzleOffset<float>>);
        static_assert(std::is_aggregate_v<ViewBasis<float>>);
        static_assert(std::is_aggregate_v<AimAngles<float>>);
        static_assert(std::is_aggregate_v<AimSolution<float>>);
        static_assert(std::is_aggregate_v<Launcher<double>>);
        SUCCEED();
    }

    TEST(Launcher, ProjectileNeedsOnlyItsPhysicalProperties)
    {
        // New code describes the round and leaves the legacy origin fields alone. They have to default to zero for
        // that: without a default initialiser on m_origin this exact spelling warns under clang's -Wextra.
        constexpr Projectile<float> round{.m_launch_speed = 1100.f, .m_gravity_scale = 0.5f};

        static_assert(round.m_origin == Vector3<float>{0, 0, 0});
        static_assert(round.m_launch_offset == Vector3<float>{0, 0, 0});
        static_assert(std::is_aggregate_v<Projectile<float>>);
        SUCCEED();
    }

    // ---------------------------------------------------------------- ProjPredEngineInterface

    // Remembers what it was asked and answers with fixed numbers
    class RecordingEngine final : public ProjPredEngineInterface<float>
    {
    public:
        explicit RecordingEngine(const bool has_solution): m_has_solution(has_solution)
        {
        }

        [[nodiscard]]
        std::optional<AimSolution<float>> maybe_calculate_aim(const Projectile<float>&, const Launcher<float>& launcher,
                                                              const Target<float>&) const override
        {
            m_last_launcher = launcher;
            ++m_calls;

            if (!m_has_solution)
                return std::nullopt;

            return AimSolution<float>{.angles = {11.f, 22.f},
                                      .aim_point = {3, 4, 5},
                                      .predicted_target_position = {6, 7, 8},
                                      .time_of_flight = 0.5f};
        }

        mutable std::optional<Launcher<float>> m_last_launcher;
        mutable int m_calls = 0;

    private:
        bool m_has_solution;
    };

    constexpr Projectile<float> k_legacy_projectile{
            .m_origin = {1, 2, 3}, .m_launch_offset = {4, 5, 6}, .m_launch_speed = 1000.f, .m_gravity_scale = 1.f};
    constexpr Target<float> k_target{.m_origin = {100, 0, 0}, .m_velocity = {}, .m_is_airborne = false};

    TEST(ProjPredEngineInterface, LauncherFromProjectileMapsTheLegacyFields)
    {
        constexpr auto launcher = ProjPredEngineInterface<float>::launcher_from_projectile(k_legacy_projectile);

        static_assert(launcher.eye_origin == Vector3<float>{1, 2, 3});
        static_assert(launcher.world_offset == Vector3<float>{4, 5, 6});
        static_assert(launcher.muzzle_offset.is_zero());
        static_assert(launcher.launch_pitch_offset == 0.f);
        SUCCEED();
    }

    TEST(ProjPredEngineInterface, WrappersSolveThroughALauncherBuiltFromTheLegacyFields)
    {
        const RecordingEngine engine(true);

        const auto point = engine.maybe_calculate_aim_point(k_legacy_projectile, k_target);
        ASSERT_TRUE(point.has_value());
        EXPECT_EQ(point.value(), (Vector3<float>{3, 4, 5}));

        ASSERT_TRUE(engine.m_last_launcher.has_value());
        EXPECT_EQ(engine.m_last_launcher->eye_origin, (Vector3<float>{1, 2, 3}));
        EXPECT_EQ(engine.m_last_launcher->world_offset, (Vector3<float>{4, 5, 6}));
        EXPECT_TRUE(engine.m_last_launcher->muzzle_offset.is_zero());
        EXPECT_FLOAT_EQ(engine.m_last_launcher->launch_pitch_offset, 0.f);

        const auto angles = engine.maybe_calculate_aim_angles(k_legacy_projectile, k_target);
        ASSERT_TRUE(angles.has_value());
        EXPECT_FLOAT_EQ(angles->pitch, 11.f);
        EXPECT_FLOAT_EQ(angles->yaw, 22.f);

        // Each wrapper is a full solve: callers that want both should ask for the solution once
        EXPECT_EQ(engine.m_calls, 2);
    }

    TEST(ProjPredEngineInterface, WrappersPassNoSolutionThrough)
    {
        const RecordingEngine engine(false);

        EXPECT_FALSE(engine.maybe_calculate_aim_point(k_legacy_projectile, k_target).has_value());
        EXPECT_FALSE(engine.maybe_calculate_aim_angles(k_legacy_projectile, k_target).has_value());
        EXPECT_EQ(engine.m_calls, 2);
    }

    TEST(ProjPredEngineInterface, CannotBeSlicedThroughABaseReference)
    {
        using Base = ProjPredEngineInterface<float>;

        static_assert(std::has_virtual_destructor_v<Base>);
        static_assert(!std::is_copy_assignable_v<Base>);
        static_assert(!std::is_move_assignable_v<Base>);
        static_assert(!std::is_copy_constructible_v<Base>);

        // Concrete engines stay ordinary values
        static_assert(std::is_copy_constructible_v<ProjPredEngineLegacy<>>);
        static_assert(std::is_copy_assignable_v<ProjPredEngineLegacy<>>);
        static_assert(std::is_nothrow_move_constructible_v<ProjPredEngineLegacy<>>);
        static_assert(std::is_copy_constructible_v<RecordingEngine>);
        SUCCEED();
    }

    TEST(ProjPredEngineInterface, LegacyEngineAnswersTheSameThroughTheInterface)
    {
        const ProjPredEngineLegacy<> engine(800.f, 1.f / 1000.f, 10.f, 5.f);
        const ProjPredEngineInterface<float>& through_base = engine;

        constexpr Projectile<float> round{.m_launch_speed = 1500.f, .m_gravity_scale = 1.f};
        constexpr Launcher<float> launcher{.eye_origin = {0, 0, 64},
                                           .muzzle_offset = {.forward = 16, .right = 8, .up = -6}};
        constexpr Target<float> target{.m_origin = {700, 120, 90}, .m_velocity = {-60, 45, 0}, .m_is_airborne = false};

        static_assert(noexcept(engine.maybe_calculate_aim(round, launcher, target)));

        const auto direct = engine.maybe_calculate_aim(round, launcher, target);
        const auto dispatched = through_base.maybe_calculate_aim(round, launcher, target);
        ASSERT_TRUE(direct.has_value());
        ASSERT_TRUE(dispatched.has_value());

        EXPECT_FLOAT_EQ(direct->angles.pitch, dispatched->angles.pitch);
        EXPECT_FLOAT_EQ(direct->angles.yaw, dispatched->angles.yaw);
        EXPECT_FLOAT_EQ(direct->time_of_flight, dispatched->time_of_flight);
        EXPECT_EQ(direct->aim_point, dispatched->aim_point);

        // A copy is its own engine with the same settings
        const auto copy = engine; // NOLINT(*-unnecessary-copy-initialization)
        const auto from_copy = copy.maybe_calculate_aim(round, launcher, target);
        ASSERT_TRUE(from_copy.has_value());
        EXPECT_FLOAT_EQ(from_copy->angles.pitch, direct->angles.pitch);
    }
} // namespace
