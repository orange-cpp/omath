#include <gtest/gtest.h>
#include <omath/hashing.hpp>

TEST(unit_test_hashing, fnv1a)
{
    constexpr auto hash = omath::hashing::fnv1a("hello");

    static_assert(hash == 0xA430D84680AABD0BULL);
    EXPECT_EQ(hash, 0xA430D84680AABD0BULL);
}

TEST(unit_test_hashing, crc32)
{
    constexpr auto crc = omath::hashing::crc32("123456789");

    static_assert(crc == 0xCBF43926U);
    EXPECT_EQ(crc, 0xCBF43926U);
}

TEST(unit_test_hashing, base64_encode)
{
    static_assert(omath::hashing::base64_encode("foo") == "Zm9v");
    EXPECT_EQ(omath::hashing::base64_encode("foobar"), "Zm9vYmFy");
}

TEST(unit_test_hashing, base64_decode)
{
    static_assert(omath::hashing::base64_decode("Zm9v").has_value());
    static_assert(omath::hashing::base64_decode("Zm9v").value() == "foo");
    EXPECT_EQ(omath::hashing::base64_decode("Zm9vYmFy").value(), "foobar");
    EXPECT_EQ(omath::hashing::base64_decode("Zm9v=").error(), omath::hashing::Base64Error::INVALID_LENGTH);
    EXPECT_EQ(omath::hashing::base64_decode("Zm?v").error(), omath::hashing::Base64Error::INVALID_CHARACTER);
    EXPECT_EQ(omath::hashing::base64_decode("Zm9=").error(), omath::hashing::Base64Error::INVALID_PADDING);
}
