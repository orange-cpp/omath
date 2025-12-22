#include "coverage_wrappers.hpp"

// Backwards-compatible wrappers: old symbol names that redirected to
// the existing forwarder implementations. This keeps tests or external
// code that expect the older `call_*_noinline_forwarders` symbols working.

extern "C" {
    void call_vector3_noinline_forwarders()
    {
        coverage_wrappers::call_vector3_forwarders();
    }

    void call_triangle_noinline_forwarders()
    {
        coverage_wrappers::call_triangle_forwarders();
    }

    void call_vector4_noinline_forwarders()
    {
        coverage_wrappers::call_vector4_forwarders();
    }
}
