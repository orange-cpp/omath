// Invariants every engine's PredEngineTrait has to satisfy, and ProjPredEngineLegacy solving through each of them.
//
// Scenarios are laid out along the engine's own axes (k_abs_forward / right / up), so one scenario means the same thing
// in a Z-up right-handed engine and in a Y-up left-handed one. Before this file only the Source trait was exercised
// through the solver, which is how four traits kept launching downwards for a positive pitch without a test noticing.
#include <cmath>
#include <gtest/gtest.h>
#include <omath/engines/cry_engine/formulas.hpp>
#include <omath/engines/cry_engine/traits/camera_trait.hpp>
#include <omath/engines/cry_engine/traits/pred_engine_trait.hpp>
#include <omath/engines/frostbite_engine/formulas.hpp>
#include <omath/engines/frostbite_engine/traits/camera_trait.hpp>
#include <omath/engines/frostbite_engine/traits/pred_engine_trait.hpp>
#include <omath/engines/iw_engine/formulas.hpp>
#include <omath/engines/iw_engine/traits/camera_trait.hpp>
#include <omath/engines/iw_engine/traits/pred_engine_trait.hpp>
#include <omath/engines/opengl_engine/formulas.hpp>
#include <omath/engines/opengl_engine/traits/camera_trait.hpp>
#include <omath/engines/opengl_engine/traits/pred_engine_trait.hpp>
#include <omath/engines/rage_engine/formulas.hpp>
#include <omath/engines/rage_engine/traits/camera_trait.hpp>
#include <omath/engines/rage_engine/traits/pred_engine_trait.hpp>
#include <omath/engines/source_engine/formulas.hpp>
#include <omath/engines/source_engine/traits/camera_trait.hpp>
#include <omath/engines/source_engine/traits/pred_engine_trait.hpp>
#include <omath/engines/unity_engine/formulas.hpp>
#include <omath/engines/unity_engine/traits/camera_trait.hpp>
#include <omath/engines/unity_engine/traits/pred_engine_trait.hpp>
#include <omath/engines/unreal_engine/formulas.hpp>
#include <omath/engines/unreal_engine/traits/camera_trait.hpp>
#include <omath/engines/unreal_engine/traits/pred_engine_trait.hpp>
#include <omath/projectile_prediction/proj_pred_engine_legacy.hpp>
#include <optional>
#include <string>

namespace
{
    using omath::Vector3;
    using omath::projectile_prediction::AimSolution;
    using omath::projectile_prediction::Launcher;
    using omath::projectile_prediction::Projectile;
    using omath::projectile_prediction::ProjPredEngineLegacy;
    using omath::projectile_prediction::Target;

// NOLINTNEXTLINE(*-macro-usage)
#define OMATH_TEST_ENGINE(Name, ns, ScalarType)                                                                        \
    struct Name final                                                                                                  \
    {                                                                                                                  \
        using Scalar = ScalarType;                                                                                     \
        using Trait = omath::ns::PredEngineTrait;                                                                      \
        using CameraTrait = omath::ns::CameraTrait;                                                                    \
        using ViewAngles = omath::ns::ViewAngles;                                                                      \
        static std::string name()                                                                                      \
        {                                                                                                              \
            return #Name;                                                                                              \
        }                                                                                                              \
        static Vector3<Scalar> axis_forward()                                                                          \
        {                                                                                                              \
            return omath::ns::k_abs_forward;                                                                           \
        }                                                                                                              \
        static Vector3<Scalar> axis_right()                                                                            \
        {                                                                                                              \
            return omath::ns::k_abs_right;                                                                             \
        }                                                                                                              \
        static Vector3<Scalar> axis_up()                                                                               \
        {                                                                                                              \
            return omath::ns::k_abs_up;                                                                                \
        }                                                                                                              \
        static Vector3<Scalar> forward_vector(const ViewAngles& angles)                                                \
        {                                                                                                              \
            return omath::ns::forward_vector(angles);                                                                  \
        }                                                                                                              \
        static Vector3<Scalar> right_vector(const ViewAngles& angles)                                                  \
        {                                                                                                              \
            return omath::ns::right_vector(angles);                                                                    \
        }                                                                                                              \
        static Vector3<Scalar> up_vector(const ViewAngles& angles)                                                     \
        {                                                                                                              \
            return omath::ns::up_vector(angles);                                                                       \
        }                                                                                                              \
    };

    OMATH_TEST_ENGINE(Source, source_engine, float)
    OMATH_TEST_ENGINE(Iw, iw_engine, float)
    OMATH_TEST_ENGINE(Rage, rage_engine, float)
    OMATH_TEST_ENGINE(Cry, cry_engine, float)
    OMATH_TEST_ENGINE(Unity, unity_engine, float)
    OMATH_TEST_ENGINE(Frostbite, frostbite_engine, float)
    OMATH_TEST_ENGINE(OpenGl, opengl_engine, float)
    OMATH_TEST_ENGINE(Unreal, unreal_engine, double)

#undef OMATH_TEST_ENGINE

    struct EngineNames final
    {
        template<class Engine>
        static std::string GetName(int) // NOLINT(*-identifier-naming) name required by GoogleTest
        {
            return Engine::name();
        }
    };

    template<class Engine>
    class PredEngineTraitAll : public ::testing::Test
    {
    };

    using Engines = ::testing::Types<Source, Iw, Rage, Cry, Unity, Frostbite, OpenGl, Unreal>;
    TYPED_TEST_SUITE(PredEngineTraitAll, Engines, EngineNames);

    constexpr double k_pitches[] = {-60., -20., 0., 35., 70.};
    constexpr double k_yaws[] = {-150., -45., 0., 30., 120.};

    // A point given as "this far ahead, this far to the right, this high" in the engine's own axes
    template<class Engine>
    Vector3<typename Engine::Scalar> world(const double forward, const double right, const double up)
    {
        using Scalar = typename Engine::Scalar;
        return Engine::axis_forward() * static_cast<Scalar>(forward) + Engine::axis_right() * static_cast<Scalar>(right)
               + Engine::axis_up() * static_cast<Scalar>(up);
    }

    template<class Scalar>
    Scalar angle_difference(const Scalar a, const Scalar b)
    {
        return std::remainder(a - b, Scalar{360});
    }

    template<class Scalar>
    void expect_vectors_near(const Vector3<Scalar>& actual, const Vector3<Scalar>& expected, const double tolerance)
    {
        EXPECT_NEAR(actual.x, expected.x, tolerance);
        EXPECT_NEAR(actual.y, expected.y, tolerance);
        EXPECT_NEAR(actual.z, expected.z, tolerance);
    }

    // ---------------------------------------------------------------- trait invariants

    TYPED_TEST(PredEngineTraitAll, ZeroAnglesBasisIsTheWorldAxes)
    {
        using Scalar = typename TypeParam::Scalar;
        const auto basis = TypeParam::Trait::calc_view_basis(Scalar{0}, Scalar{0});

        expect_vectors_near(basis.forward, TypeParam::axis_forward(), 1e-5);
        expect_vectors_near(basis.right, TypeParam::axis_right(), 1e-5);
        expect_vectors_near(basis.up, TypeParam::axis_up(), 1e-5);
    }

    TYPED_TEST(PredEngineTraitAll, BasisIsOrthonormalAndKeepsHandedness)
    {
        using Scalar = typename TypeParam::Scalar;
        const auto handedness = TypeParam::axis_forward().cross(TypeParam::axis_right()).dot(TypeParam::axis_up());

        for (const auto pitch : k_pitches)
            for (const auto yaw : k_yaws)
            {
                const auto basis =
                        TypeParam::Trait::calc_view_basis(static_cast<Scalar>(pitch), static_cast<Scalar>(yaw));

                EXPECT_NEAR(basis.forward.length(), 1., 1e-5);
                EXPECT_NEAR(basis.right.length(), 1., 1e-5);
                EXPECT_NEAR(basis.up.length(), 1., 1e-5);
                EXPECT_NEAR(basis.forward.dot(basis.right), 0., 1e-5);
                EXPECT_NEAR(basis.forward.dot(basis.up), 0., 1e-5);
                EXPECT_NEAR(basis.right.dot(basis.up), 0., 1e-5);
                EXPECT_NEAR(basis.forward.cross(basis.right).dot(basis.up), handedness, 1e-5)
                        << "pitch " << pitch << " yaw " << yaw;
            }
    }

    // What the solver leans on: it measures angles with calc_direct_*_angle and turns them back into directions with
    // calc_view_basis, so the two must be inverses. A flipped pitch sign in either fails here.
    TYPED_TEST(PredEngineTraitAll, BasisRoundTripsThroughDirectAngles)
    {
        using Scalar = typename TypeParam::Scalar;
        using Trait = typename TypeParam::Trait;

        for (const auto pitch : k_pitches)
            for (const auto yaw : k_yaws)
            {
                const auto forward =
                        Trait::calc_view_basis(static_cast<Scalar>(pitch), static_cast<Scalar>(yaw)).forward;

                EXPECT_NEAR(Trait::calc_direct_pitch_angle(Vector3<Scalar>{}, forward), pitch, 1e-3)
                        << "pitch " << pitch << " yaw " << yaw;
                EXPECT_NEAR(angle_difference(Trait::calc_direct_yaw_angle(Vector3<Scalar>{}, forward),
                                             static_cast<Scalar>(yaw)),
                            0., 1e-3)
                        << "pitch " << pitch << " yaw " << yaw;
            }
    }

    // The muzzle offset is laid out along basis.right and basis.up, so they have to be the engine's own right and up
    // for the view that looks along basis.forward.
    TYPED_TEST(PredEngineTraitAll, BasisMatchesTheEnginesDirectionVectors)
    {
        using Scalar = typename TypeParam::Scalar;

        for (const auto pitch : k_pitches)
            for (const auto yaw : k_yaws)
            {
                const auto basis =
                        TypeParam::Trait::calc_view_basis(static_cast<Scalar>(pitch), static_cast<Scalar>(yaw));
                const auto angles = TypeParam::CameraTrait::calc_look_at_angle(Vector3<Scalar>{}, basis.forward);

                SCOPED_TRACE("pitch " + std::to_string(pitch) + " yaw " + std::to_string(yaw));
                expect_vectors_near(TypeParam::forward_vector(angles), basis.forward, 1e-4);
                expect_vectors_near(TypeParam::right_vector(angles), basis.right, 1e-4);
                expect_vectors_near(TypeParam::up_vector(angles), basis.up, 1e-4);
            }
    }

    // forward * a + up * b in the view frame is the view pitched up by atan(b / a): what Launcher::launch_pitch_offset
    // stands on, and it only holds if basis.up really is the direction a positive pitch turns towards.
    TYPED_TEST(PredEngineTraitAll, RaisingThePitchTurnsForwardTowardsUp)
    {
        using Scalar = typename TypeParam::Scalar;
        using Trait = typename TypeParam::Trait;
        const auto offset = omath::angles::radians_to_degrees(std::atan2(Scalar{200}, Scalar{1200}));

        for (const auto yaw : k_yaws)
        {
            const auto basis = Trait::calc_view_basis(Scalar{20}, static_cast<Scalar>(yaw));
            const auto game_velocity = basis.forward * Scalar{1200} + basis.up * Scalar{200};
            const auto raised = Trait::calc_view_basis(Scalar{20} + offset, static_cast<Scalar>(yaw)).forward
                                * std::hypot(Scalar{1200}, Scalar{200});

            EXPECT_NEAR(game_velocity.distance_to(raised), 0., 2e-2) << "yaw " << yaw;
        }
    }

    TYPED_TEST(PredEngineTraitAll, PositivePitchLaunchesUpwards)
    {
        using Scalar = typename TypeParam::Scalar;
        using Trait = typename TypeParam::Trait;

        const Projectile<Scalar> round{.m_launch_speed = Scalar{100}, .m_gravity_scale = Scalar{1}};
        const auto origin = world<TypeParam>(5., -3., 40.);

        const auto up_shot =
                Trait::predict_projectile_position(origin, round, Scalar{30}, Scalar{0}, Scalar{1}, Scalar{0}) - origin;
        EXPECT_NEAR(up_shot.dot(TypeParam::axis_up()), 50., 1e-3);
        EXPECT_NEAR(up_shot.dot(TypeParam::axis_forward()), 100. * std::cos(omath::angles::degrees_to_radians(30.)),
                    1e-3);
        EXPECT_NEAR(up_shot.dot(TypeParam::axis_right()), 0., 1e-3);

        const auto down_shot =
                Trait::predict_projectile_position(origin, round, Scalar{-30}, Scalar{0}, Scalar{1}, Scalar{0})
                - origin;
        EXPECT_NEAR(down_shot.dot(TypeParam::axis_up()), -50., 1e-3);
    }

    TYPED_TEST(PredEngineTraitAll, ProjectileStartsAtTheLaunchOriginAndFliesStraightWithoutGravity)
    {
        using Scalar = typename TypeParam::Scalar;
        using Trait = typename TypeParam::Trait;

        const Projectile<Scalar> round{.m_launch_speed = Scalar{250}, .m_gravity_scale = Scalar{0}};
        const auto origin = world<TypeParam>(10., 20., 30.);

        expect_vectors_near(
                Trait::predict_projectile_position(origin, round, Scalar{15}, Scalar{40}, Scalar{0}, Scalar{800}),
                origin, 1e-5);

        // No gravity scale: gravity passed in must not matter, and the path is a straight line along the basis
        const auto forward = Trait::calc_view_basis(Scalar{15}, Scalar{40}).forward;
        expect_vectors_near(
                Trait::predict_projectile_position(origin, round, Scalar{15}, Scalar{40}, Scalar{2}, Scalar{800}),
                origin + forward * Scalar{500}, 1e-2);
    }

    TYPED_TEST(PredEngineTraitAll, GravityPullsAlongTheEnginesDownAxis)
    {
        using Scalar = typename TypeParam::Scalar;
        using Trait = typename TypeParam::Trait;

        const Projectile<Scalar> round{.m_launch_speed = Scalar{300}, .m_gravity_scale = Scalar{0.5}};
        const auto origin = world<TypeParam>(0., 0., 100.);

        const auto weightless =
                Trait::predict_projectile_position(origin, round, Scalar{10}, Scalar{25}, Scalar{2}, Scalar{0});
        const auto falling =
                Trait::predict_projectile_position(origin, round, Scalar{10}, Scalar{25}, Scalar{2}, Scalar{800});

        // 0.5 * (800 * 0.5) * 2^2 = 800 straight down, nothing sideways
        expect_vectors_near(weightless - falling, TypeParam::axis_up() * Scalar{800}, 1e-2);
    }

    TYPED_TEST(PredEngineTraitAll, TargetPredictionFallsOnlyWhenAirborne)
    {
        using Scalar = typename TypeParam::Scalar;
        using Trait = typename TypeParam::Trait;

        Target<Scalar> target{.m_origin = world<TypeParam>(100., 50., 300.),
                              .m_velocity = world<TypeParam>(-20., 10., 5.),
                              .m_is_airborne = false};

        const auto grounded = Trait::predict_target_position(target, Scalar{2}, Scalar{800});
        expect_vectors_near(grounded, target.m_origin + target.m_velocity * Scalar{2}, 1e-3);

        target.m_is_airborne = true;
        const auto airborne = Trait::predict_target_position(target, Scalar{2}, Scalar{800});
        expect_vectors_near(grounded - airborne, TypeParam::axis_up() * Scalar{1600}, 1e-2);
    }

    TYPED_TEST(PredEngineTraitAll, SplitsAVectorIntoGroundDistanceAndHeight)
    {
        using Trait = typename TypeParam::Trait;

        const auto delta = world<TypeParam>(3., 4., 12.);
        EXPECT_NEAR(Trait::calc_vector_2d_distance(delta), 5., 1e-5);
        EXPECT_NEAR(Trait::get_vector_height_coordinate(delta), 12., 1e-5);

        const auto below = world<TypeParam>(-6., 8., -7.);
        EXPECT_NEAR(Trait::calc_vector_2d_distance(below), 10., 1e-5);
        EXPECT_NEAR(Trait::get_vector_height_coordinate(below), -7., 1e-5);
    }

    TYPED_TEST(PredEngineTraitAll, DirectAnglesOfTheAxes)
    {
        using Scalar = typename TypeParam::Scalar;
        using Trait = typename TypeParam::Trait;
        const Vector3<Scalar> origin = world<TypeParam>(7., -2., 9.);

        EXPECT_NEAR(Trait::calc_direct_pitch_angle(origin, origin + TypeParam::axis_forward() * Scalar{50}), 0., 1e-3);
        EXPECT_NEAR(Trait::calc_direct_yaw_angle(origin, origin + TypeParam::axis_forward() * Scalar{50}), 0., 1e-3);
        EXPECT_NEAR(Trait::calc_direct_pitch_angle(origin, origin + world<TypeParam>(10., 0., 10.)), 45., 1e-3);
        EXPECT_NEAR(Trait::calc_direct_pitch_angle(origin, origin + world<TypeParam>(10., 0., -10.)), -45., 1e-3);

        // Turning to the right and to the left are opposite yaws of the same size, whichever sign the engine gives them
        const auto to_the_right = Trait::calc_direct_yaw_angle(origin, origin + world<TypeParam>(10., 10., 0.));
        const auto to_the_left = Trait::calc_direct_yaw_angle(origin, origin + world<TypeParam>(10., -10., 0.));
        EXPECT_NEAR(std::abs(to_the_right), 45., 1e-3);
        EXPECT_NEAR(to_the_right + to_the_left, 0., 1e-3);
    }

    // ---------------------------------------------------------------- the legacy engine through every trait

    template<class Engine>
    struct Scenario final
    {
        using Scalar = typename Engine::Scalar;

        Projectile<Scalar> projectile;
        Launcher<Scalar> launcher;
        Target<Scalar> target;
        Scalar gravity = Scalar{800};
        Scalar hit_tolerance = Scalar{10};
    };

    // Solves, then fires the trait's own projectile from where the solution puts the muzzle and checks it arrives.
    // Returns the solution so a test can assert more about it.
    template<class Engine>
    std::optional<AimSolution<typename Engine::Scalar>> solve_and_check(const Scenario<Engine>& scenario)
    {
        using Scalar = typename Engine::Scalar;
        using Trait = typename Engine::Trait;

        const ProjPredEngineLegacy<Trait, Scalar> engine(scenario.gravity, Scalar{1} / Scalar{1000}, Scalar{10},
                                                         Scalar{5});
        const auto aim = engine.maybe_calculate_aim(scenario.projectile, scenario.launcher, scenario.target);
        if (!aim.has_value())
        {
            ADD_FAILURE() << "engine found no solution";
            return std::nullopt;
        }

        const auto basis = Trait::calc_view_basis(aim->angles.pitch, aim->angles.yaw);
        const auto launch_origin = scenario.launcher.launch_origin(basis);
        const auto projectile_position = Trait::predict_projectile_position(
                launch_origin, scenario.projectile, aim->angles.pitch + scenario.launcher.launch_pitch_offset,
                aim->angles.yaw, aim->time_of_flight, scenario.gravity);
        const auto target_position =
                Trait::predict_target_position(scenario.target, aim->time_of_flight, scenario.gravity);

        EXPECT_LE(projectile_position.distance_to(target_position), scenario.hit_tolerance)
                << "fired from the solved muzzle with the solved angles the round has to arrive";
        expect_vectors_near(aim->predicted_target_position, target_position, 1e-3);

        // The aim point sits on the eye's view ray, as far out as the target
        const auto& eye = scenario.launcher.eye_origin;
        EXPECT_NEAR(Trait::calc_direct_pitch_angle(eye, aim->aim_point), aim->angles.pitch, 0.01);
        EXPECT_NEAR(angle_difference(Trait::calc_direct_yaw_angle(eye, aim->aim_point), aim->angles.yaw), 0., 0.01);
        EXPECT_NEAR(eye.distance_to(aim->aim_point), eye.distance_to(target_position), 0.05);

        return aim;
    }

    TYPED_TEST(PredEngineTraitAll, EngineHitsAnElevatedTargetWithAnUpwardPitch)
    {
        using Scalar = typename TypeParam::Scalar;
        const Scenario<TypeParam> scenario{
                .projectile = {.m_launch_speed = Scalar{1500}, .m_gravity_scale = Scalar{1}},
                .launcher = {.eye_origin = world<TypeParam>(0., 0., 64.)},
                .target = {.m_origin = world<TypeParam>(800., 200., 250.), .m_velocity = {}, .m_is_airborne = false},
        };

        const auto aim = solve_and_check(scenario);
        ASSERT_TRUE(aim.has_value());
        EXPECT_GT(aim->angles.pitch, 0.);
        EXPECT_GT(aim->time_of_flight, 0.);
    }

    TYPED_TEST(PredEngineTraitAll, EngineHitsATargetBelowWithADownwardPitch)
    {
        using Scalar = typename TypeParam::Scalar;
        const Scenario<TypeParam> scenario{
                .projectile = {.m_launch_speed = Scalar{2000}, .m_gravity_scale = Scalar{0.5}},
                .launcher = {.eye_origin = world<TypeParam>(0., 0., 600.)},
                .target = {.m_origin = world<TypeParam>(700., -250., 0.), .m_velocity = {}, .m_is_airborne = false},
        };

        const auto aim = solve_and_check(scenario);
        ASSERT_TRUE(aim.has_value());
        EXPECT_LT(aim->angles.pitch, 0.);
    }

    TYPED_TEST(PredEngineTraitAll, EngineLeadsAMovingTargetFromAnOffsetMuzzle)
    {
        using Scalar = typename TypeParam::Scalar;
        const Scenario<TypeParam> scenario{
                .projectile = {.m_launch_speed = Scalar{1100}, .m_gravity_scale = Scalar{0.4}},
                .launcher = {.eye_origin = world<TypeParam>(0., 0., 64.),
                             .muzzle_offset = {.forward = Scalar{16}, .right = Scalar{8}, .up = Scalar{-6}}},
                .target = {.m_origin = world<TypeParam>(600., -150., 0.),
                           .m_velocity = world<TypeParam>(-40., 90., 0.),
                           .m_is_airborne = false},
        };

        const auto aim = solve_and_check(scenario);
        ASSERT_TRUE(aim.has_value());

        // It aims where the target will be, not where it is
        EXPECT_GT(aim->predicted_target_position.distance_to(scenario.target.m_origin), 30.);
    }

    TYPED_TEST(PredEngineTraitAll, EngineHitsAFallingTarget)
    {
        using Scalar = typename TypeParam::Scalar;
        const Scenario<TypeParam> scenario{
                .projectile = {.m_launch_speed = Scalar{1800}, .m_gravity_scale = Scalar{0.5}},
                .launcher = {.eye_origin = world<TypeParam>(0., 0., 64.)},
                .target = {.m_origin = world<TypeParam>(400., 120., 500.),
                           .m_velocity = world<TypeParam>(15., -10., -30.),
                           .m_is_airborne = true},
        };

        const auto aim = solve_and_check(scenario);
        ASSERT_TRUE(aim.has_value());
        EXPECT_LT(aim->predicted_target_position.dot(TypeParam::axis_up()),
                  scenario.target.m_origin.dot(TypeParam::axis_up()));
    }

    // A rocket has no arc to hide behind: from the rotated muzzle the solved direction has to run through the target
    TYPED_TEST(PredEngineTraitAll, RocketFromAnOffsetMuzzleFliesThroughTheTarget)
    {
        using Scalar = typename TypeParam::Scalar;
        using Trait = typename TypeParam::Trait;
        const Scenario<TypeParam> scenario{
                .projectile = {.m_launch_speed = Scalar{1100}, .m_gravity_scale = Scalar{0}},
                .launcher = {.eye_origin = world<TypeParam>(0., 0., 64.),
                             .muzzle_offset = {.forward = Scalar{23.5}, .right = Scalar{12}, .up = Scalar{-3}}},
                .target = {.m_origin = world<TypeParam>(900., 300., 150.), .m_velocity = {}, .m_is_airborne = false},
        };

        const auto aim = solve_and_check(scenario);
        ASSERT_TRUE(aim.has_value());

        const auto basis = Trait::calc_view_basis(aim->angles.pitch, aim->angles.yaw);
        const auto to_target = scenario.target.m_origin - scenario.launcher.launch_origin(basis);
        const auto off_the_ray = to_target - basis.forward * to_target.dot(basis.forward);
        // One refinement pass leaves a second-order error; placing the muzzle only once would be off by a third
        // of a unit here
        EXPECT_LE(off_the_ray.length(), 0.05);

        // and the eye does not look straight at the target: the muzzle is off to the side
        EXPECT_GT(std::abs(angle_difference(aim->angles.yaw, Trait::calc_direct_yaw_angle(scenario.launcher.eye_origin,
                                                                                          scenario.target.m_origin))),
                  0.3);
    }

    // The muzzle turns with the view while the round leaves along the view raised by the offset. Placing the muzzle
    // from the launch pitch instead would put it several units away from where the game spawns the round.
    TYPED_TEST(PredEngineTraitAll, TiltedRocketFromAnOffsetMuzzleFliesThroughTheTarget)
    {
        using Scalar = typename TypeParam::Scalar;
        using Trait = typename TypeParam::Trait;
        const Scenario<TypeParam> scenario{
                .projectile = {.m_launch_speed = Scalar{1100}, .m_gravity_scale = Scalar{0}},
                .launcher = {.eye_origin = world<TypeParam>(0., 0., 64.),
                             .muzzle_offset = {.forward = Scalar{23.5}, .right = Scalar{12}, .up = Scalar{-3}},
                             .launch_pitch_offset = Scalar{10}},
                .target = {.m_origin = world<TypeParam>(900., 300., 150.), .m_velocity = {}, .m_is_airborne = false},
        };

        const auto aim = solve_and_check(scenario);
        ASSERT_TRUE(aim.has_value());

        const auto view = Trait::calc_view_basis(aim->angles.pitch, aim->angles.yaw);
        const auto launch_direction =
                Trait::calc_view_basis(aim->angles.pitch + scenario.launcher.launch_pitch_offset, aim->angles.yaw)
                        .forward;
        const auto to_target = scenario.target.m_origin - scenario.launcher.launch_origin(view);
        const auto off_the_ray = to_target - launch_direction * to_target.dot(launch_direction);
        EXPECT_LE(off_the_ray.length(), 0.05);

        // The eye looks well below the line the round takes
        EXPECT_NEAR(aim->angles.pitch + 10.,
                    Trait::calc_direct_pitch_angle(scenario.launcher.launch_origin(view), scenario.target.m_origin),
                    1e-3);
    }

    // TF2-style pipe: velocity = forward * 1200 + up * 200 in the view frame, given to the engine as a pitch offset
    TYPED_TEST(PredEngineTraitAll, PipeStyleLaunchFiredTheWayTheGameDoesHits)
    {
        using Scalar = typename TypeParam::Scalar;
        using Trait = typename TypeParam::Trait;
        constexpr Scalar forward_speed{1200};
        constexpr Scalar up_speed{200};

        const Scenario<TypeParam> scenario{
                .projectile = {.m_launch_speed = std::hypot(forward_speed, up_speed), .m_gravity_scale = Scalar{0.5}},
                .launcher = {.eye_origin = world<TypeParam>(0., 0., 64.),
                             .muzzle_offset = {.forward = Scalar{16}, .right = Scalar{8}, .up = Scalar{-6}},
                             .launch_pitch_offset =
                                     omath::angles::radians_to_degrees(std::atan2(up_speed, forward_speed))},
                .target = {.m_origin = world<TypeParam>(600., 150., 0.),
                           .m_velocity = world<TypeParam>(-40., 30., 0.),
                           .m_is_airborne = false},
        };

        const auto aim = solve_and_check(scenario);
        ASSERT_TRUE(aim.has_value());

        const auto basis = Trait::calc_view_basis(aim->angles.pitch, aim->angles.yaw);
        const auto time = aim->time_of_flight;
        const auto drop = Scalar{0.5} * scenario.gravity * scenario.projectile.m_gravity_scale * time * time;
        const auto position = scenario.launcher.launch_origin(basis)
                              + (basis.forward * forward_speed + basis.up * up_speed) * time
                              - TypeParam::axis_up() * drop;

        EXPECT_LE(position.distance_to(Trait::predict_target_position(scenario.target, time, scenario.gravity)), 6.);
    }

    TYPED_TEST(PredEngineTraitAll, EngineReportsNoSolutionForAnUnreachableTarget)
    {
        using Scalar = typename TypeParam::Scalar;
        const ProjPredEngineLegacy<typename TypeParam::Trait, Scalar> engine(Scalar{800}, Scalar{1} / Scalar{100},
                                                                             Scalar{5}, Scalar{5});

        const Projectile<Scalar> slow{.m_launch_speed = Scalar{10}, .m_gravity_scale = Scalar{1}};
        const Launcher<Scalar> launcher{.eye_origin = world<TypeParam>(0., 0., 64.)};
        const Target<Scalar> far_away{
                .m_origin = world<TypeParam>(50000., 0., 0.), .m_velocity = {}, .m_is_airborne = false};

        EXPECT_FALSE(engine.maybe_calculate_aim(slow, launcher, far_away).has_value());
    }
} // namespace
