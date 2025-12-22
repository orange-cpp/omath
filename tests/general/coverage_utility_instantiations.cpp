// Force execution and emission of utility header functions (compiled with -fno-inline)
#include <omath/utility/pattern_scan.hpp>
#include <omath/utility/pe_pattern_scan.hpp>
#include <omath/utility/color.hpp>

using namespace omath;

void force_utility_instantiations()
{
    // PatternScanner: span overload
    std::vector<std::byte> buf = {std::byte(0x48), std::byte(0x89), std::byte(0xE5)};
    std::span<std::byte> buf_span(buf);
    volatile auto it = PatternScanner::scan_for_pattern(buf_span, "48 89 E5");

    // Color helpers
    Color c = Color::from_rgba(10, 20, 30, 255);
    volatile auto hsv = c.to_hsv();
    c.set_hue(0.2f);
    c.set_saturation(0.4f);
    c.set_value(0.5f);

    // PePatternScanner: call loaded-module variant with null to exercise null check
    volatile auto pe_null = PePatternScanner::scan_for_pattern_in_loaded_module(nullptr, "55 8B EC");

    (void)it; (void)hsv; (void)pe_null;
}

extern "C" void call_force_utility_instantiations()
{
    force_utility_instantiations();
}
