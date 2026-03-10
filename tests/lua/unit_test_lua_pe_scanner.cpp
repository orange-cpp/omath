//
// Created by orange on 10.03.2026.
//
#include <gtest/gtest.h>
#include <lua.hpp>
#include <omath/lua/lua.hpp>
#include <cstdint>
#include <cstring>
#include <vector>

namespace
{
    std::vector<std::uint8_t> make_fake_pe_module(std::uint32_t base_of_code, std::uint32_t size_code,
                                                  const std::vector<std::uint8_t>& code_bytes)
    {
        constexpr std::uint32_t e_lfanew         = 0x80;
        constexpr std::uint32_t nt_sig           = 0x4550;
        constexpr std::uint16_t opt_magic        = 0x020B; // PE32+
        constexpr std::uint16_t num_sections     = 1;
        constexpr std::uint16_t opt_hdr_size     = 0xF0;
        constexpr std::uint32_t section_table_off = e_lfanew + 4 + 20 + opt_hdr_size;
        constexpr std::uint32_t section_hdr_size = 40;
        constexpr std::uint32_t text_chars       = 0x60000020;

        const std::uint32_t headers_end = section_table_off + section_hdr_size;
        const std::uint32_t code_end    = base_of_code + size_code;
        const std::uint32_t total_size  = std::max(headers_end, code_end) + 0x100;
        std::vector<std::uint8_t> buf(total_size, 0);

        auto w16 = [&](std::size_t off, std::uint16_t v) { std::memcpy(buf.data() + off, &v, 2); };
        auto w32 = [&](std::size_t off, std::uint32_t v) { std::memcpy(buf.data() + off, &v, 4); };
        auto w64 = [&](std::size_t off, std::uint64_t v) { std::memcpy(buf.data() + off, &v, 8); };

        w16(0x00, 0x5A4D);
        w32(0x3C, e_lfanew);
        w32(e_lfanew, nt_sig);

        const std::size_t fh  = e_lfanew + 4;
        w16(fh + 2,  num_sections);
        w16(fh + 16, opt_hdr_size);

        const std::size_t opt = fh + 20;
        w16(opt + 0,   opt_magic);
        w32(opt + 4,   size_code);
        w32(opt + 20,  base_of_code);
        w64(opt + 24,  0);
        w32(opt + 32,  0x1000);
        w32(opt + 36,  0x200);
        w32(opt + 56,  code_end);
        w32(opt + 60,  headers_end);
        w32(opt + 108, 0);

        const std::size_t sh = section_table_off;
        std::memcpy(buf.data() + sh, ".text", 5);
        w32(sh + 8,  size_code);
        w32(sh + 12, base_of_code);
        w32(sh + 16, size_code);
        w32(sh + 20, base_of_code);
        w32(sh + 36, text_chars);

        if (base_of_code + code_bytes.size() <= buf.size())
            std::memcpy(buf.data() + base_of_code, code_bytes.data(), code_bytes.size());

        return buf;
    }
} // namespace

class LuaPeScanner : public ::testing::Test
{
protected:
    lua_State*                  L            = nullptr;
    std::vector<std::uint8_t>   m_fake_module;

    void SetUp() override
    {
        const std::vector<std::uint8_t> code = {0x90, 0x01, 0x02, 0x03, 0x04};
        m_fake_module = make_fake_pe_module(0x200, static_cast<std::uint32_t>(code.size()), code);

        L = luaL_newstate();
        luaL_openlibs(L);
        omath::lua::LuaInterpreter::register_lib(L);

        lua_pushinteger(L, static_cast<lua_Integer>(
                reinterpret_cast<std::uintptr_t>(m_fake_module.data())));
        lua_setglobal(L, "FAKE_MODULE_BASE");

        if (luaL_dofile(L, LUA_SCRIPTS_DIR "/pe_scanner_tests.lua") != LUA_OK)
            FAIL() << lua_tostring(L, -1);
    }

    void TearDown() override { lua_close(L); }

    void check(const char* func_name)
    {
        lua_getglobal(L, func_name);
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        {
            FAIL() << lua_tostring(L, -1);
            lua_pop(L, 1);
        }
    }
};

TEST_F(LuaPeScanner, PatternScanner_FindsExactPattern)       { check("PatternScanner_FindsExactPattern"); }
TEST_F(LuaPeScanner, PatternScanner_FindsPatternAtOffset)    { check("PatternScanner_FindsPatternAtNonZeroOffset"); }
TEST_F(LuaPeScanner, PatternScanner_WildcardMatches)         { check("PatternScanner_WildcardMatches"); }
TEST_F(LuaPeScanner, PatternScanner_ReturnsNilWhenNotFound)  { check("PatternScanner_ReturnsNilWhenNotFound"); }
TEST_F(LuaPeScanner, PatternScanner_ReturnsNilForEmptyBuffer){ check("PatternScanner_ReturnsNilForEmptyBuffer"); }
TEST_F(LuaPeScanner, PeScanner_FindsExactPattern)            { check("PeScanner_FindsExactPattern"); }
TEST_F(LuaPeScanner, PeScanner_WildcardMatches)              { check("PeScanner_WildcardMatches"); }
TEST_F(LuaPeScanner, PeScanner_ReturnsNilWhenNotFound)       { check("PeScanner_ReturnsNilWhenNotFound"); }
TEST_F(LuaPeScanner, PeScanner_CustomSectionFallsBackToNil)  { check("PeScanner_CustomSectionFallsBackToNil"); }
TEST_F(LuaPeScanner, SectionScanResult_TypeIsRegistered)     { check("SectionScanResult_TypeIsRegistered"); }
