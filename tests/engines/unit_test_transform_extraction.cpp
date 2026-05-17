#include <gtest/gtest.h>
#include <omath/engines/cry_engine/formulas.hpp>
#include <omath/engines/frostbite_engine/formulas.hpp>
#include <omath/engines/iw_engine/formulas.hpp>
#include <omath/engines/opengl_engine/formulas.hpp>
#include <omath/engines/source_engine/formulas.hpp>
#include <omath/engines/unity_engine/formulas.hpp>
#include <omath/engines/unreal_engine/formulas.hpp>

template<class Type>
static void expect_vector_near(const omath::Vector3<Type>& actual, const omath::Vector3<Type>& expected,
                               const double epsilon)
{
    EXPECT_NEAR(static_cast<double>(actual.x), static_cast<double>(expected.x), epsilon);
    EXPECT_NEAR(static_cast<double>(actual.y), static_cast<double>(expected.y), epsilon);
    EXPECT_NEAR(static_cast<double>(actual.z), static_cast<double>(expected.z), epsilon);
}

template<class Mat4X4, class ViewAnglesType, class RotationMatrixFn, class ExtractOriginFn, class ExtractScaleFn,
         class ExtractRotationAnglesFn>
static void verify_transform_extractors(const omath::Vector3<typename Mat4X4::ContainedType>& origin,
                                        const omath::Vector3<typename Mat4X4::ContainedType>& scale,
                                        const ViewAnglesType& angles, RotationMatrixFn rotation_matrix,
                                        ExtractOriginFn extract_origin, ExtractScaleFn extract_scale,
                                        ExtractRotationAnglesFn extract_rotation_angles)
{
    using Type = typename Mat4X4::ContainedType;
    constexpr auto store_ordering = Mat4X4::get_store_ordering();
    const auto transform = omath::mat_translation<Type, store_ordering>(origin) * rotation_matrix(angles)
                           * omath::mat_scale<Type, store_ordering>(scale);

    expect_vector_near(extract_origin(transform), origin, 1e-4);
    expect_vector_near(extract_scale(transform), scale, 1e-4);
    expect_vector_near(extract_rotation_angles(transform).as_vector3(), angles.as_vector3(), 1e-3);
}

TEST(TransformExtraction, SourceEngine)
{
    namespace e = omath::source_engine;
    const e::ViewAngles angles{
            e::PitchAngle::from_degrees(20.f),
            e::YawAngle::from_degrees(-35.f),
            e::RollAngle::from_degrees(15.f),
    };

    verify_transform_extractors<e::Mat4X4>({12.f, -3.f, 8.f}, {2.f, 3.f, 4.f}, angles, e::rotation_matrix,
                                          e::extract_origin, e::extract_scale, e::extract_rotation_angles);
}

TEST(TransformExtraction, IwEngine)
{
    namespace e = omath::iw_engine;
    const e::ViewAngles angles{
            e::PitchAngle::from_degrees(-18.f),
            e::YawAngle::from_degrees(42.f),
            e::RollAngle::from_degrees(-11.f),
    };

    verify_transform_extractors<e::Mat4X4>({-7.f, 5.f, 13.f}, {1.5f, 2.5f, 3.5f}, angles, e::rotation_matrix,
                                          e::extract_origin, e::extract_scale, e::extract_rotation_angles);
}

TEST(TransformExtraction, UnrealEngine)
{
    namespace e = omath::unreal_engine;
    const e::ViewAngles angles{
            e::PitchAngle::from_degrees(30.0),
            e::YawAngle::from_degrees(45.0),
            e::RollAngle::from_degrees(-20.0),
    };

    verify_transform_extractors<e::Mat4X4>({100.0, -50.0, 25.0}, {2.0, 4.0, 6.0}, angles, e::rotation_matrix,
                                          e::extract_origin, e::extract_scale, e::extract_rotation_angles);
}

TEST(TransformExtraction, UnityEngine)
{
    namespace e = omath::unity_engine;
    const e::ViewAngles angles{
            e::PitchAngle::from_degrees(15.f),
            e::YawAngle::from_degrees(-25.f),
            e::RollAngle::from_degrees(35.f),
    };

    verify_transform_extractors<e::Mat4X4>({4.f, 9.f, -2.f}, {0.5f, 3.f, 7.f}, angles, e::rotation_matrix,
                                          e::extract_origin, e::extract_scale, e::extract_rotation_angles);
}

TEST(TransformExtraction, FrostbiteEngine)
{
    namespace e = omath::frostbite_engine;
    const e::ViewAngles angles{
            e::PitchAngle::from_degrees(-12.f),
            e::YawAngle::from_degrees(33.f),
            e::RollAngle::from_degrees(-27.f),
    };

    verify_transform_extractors<e::Mat4X4>({6.f, -8.f, 10.f}, {1.25f, 2.75f, 4.25f}, angles, e::rotation_matrix,
                                          e::extract_origin, e::extract_scale, e::extract_rotation_angles);
}

TEST(TransformExtraction, CryEngine)
{
    namespace e = omath::cry_engine;
    const e::ViewAngles angles{
            e::PitchAngle::from_degrees(-18.f),
            e::YawAngle::from_degrees(40.f),
            e::RollAngle::from_degrees(22.f),
    };

    verify_transform_extractors<e::Mat4X4>({3.f, 14.f, -6.f}, {2.f, 5.f, 8.f}, angles, e::rotation_matrix,
                                          e::extract_origin, e::extract_scale, e::extract_rotation_angles);
}

TEST(TransformExtraction, OpenGlEngine)
{
    namespace e = omath::opengl_engine;
    const e::ViewAngles angles{
            e::PitchAngle::from_degrees(24.f),
            e::YawAngle::from_degrees(-31.f),
            e::RollAngle::from_degrees(17.f),
    };

    verify_transform_extractors<e::Mat4X4>({-9.f, 2.f, 11.f}, {3.f, 6.f, 9.f}, angles, e::rotation_matrix,
                                          e::extract_origin, e::extract_scale, e::extract_rotation_angles);
}
