#include <gtest/gtest.h>
#include <omath/linear_algebra/vector2.hpp>
#include <omath/linear_algebra/vector3.hpp>
#include <omath/linear_algebra/vector4.hpp>
#include <omath/linear_algebra/mat.hpp>
#include <functional>
#include <format>

using namespace omath;

TEST(LinearAlgebraExtra, FormatterAndHashVector2)
{
    Vector2<float> v{1.0f, 2.0f};
    const std::string s = std::format("{}", v);
    EXPECT_EQ(s, "[1, 2]");

    const std::size_t h1 = std::hash<Vector2<float>>{}(v);
    const std::size_t h2 = std::hash<Vector2<float>>{}(Vector2<float>{1.0f, 2.0f});
    const std::size_t h3 = std::hash<Vector2<float>>{}(Vector2<float>{2.0f, 3.0f});

    EXPECT_EQ(h1, h2);
    EXPECT_NE(h1, h3);
}

TEST(LinearAlgebraExtra, FormatterAndHashVector3)
{
    Vector3<float> v{1.0f, 2.0f, 3.0f};
    const std::string s = std::format("{}", v);
    EXPECT_EQ(s, "[1, 2, 3]");

    const std::size_t h1 = std::hash<Vector3<float>>{}(v);
    const std::size_t h2 = std::hash<Vector3<float>>{}(Vector3<float>{1.0f, 2.0f, 3.0f});
    EXPECT_EQ(h1, h2);

    // point_to_same_direction
    EXPECT_TRUE((Vector3<float>{1,0,0}.point_to_same_direction(Vector3<float>{2,0,0})));
    EXPECT_FALSE((Vector3<float>{1,0,0}.point_to_same_direction(Vector3<float>{-1,0,0})));
}

TEST(LinearAlgebraExtra, FormatterAndHashVector4)
{
    Vector4<float> v{1.0f, 2.0f, 3.0f, 4.0f};
    const std::string s = std::format("{}", v);
    EXPECT_EQ(s, "[1, 2, 3, 4]");

    const std::size_t h1 = std::hash<Vector4<float>>{}(v);
    const std::size_t h2 = std::hash<Vector4<float>>{}(Vector4<float>{1.0f, 2.0f, 3.0f, 4.0f});
    EXPECT_EQ(h1, h2);
}

TEST(LinearAlgebraExtra, MatRawArrayAndOperators)
{
    Mat<2,2> m{{1.0f, 2.0f},{3.0f,4.0f}};
    const auto raw = m.raw_array();
    EXPECT_EQ(raw.size(), 4);
    EXPECT_FLOAT_EQ(raw[0], 1.0f);
    EXPECT_FLOAT_EQ(raw[3], 4.0f);

    // operator[] index access
    EXPECT_FLOAT_EQ(m.at(0,0), 1.0f);
    EXPECT_FLOAT_EQ(m.at(1,1), 4.0f);
}


