// Wrapper TU compiled with -fno-inline to force out-of-line symbols
#include <omath/linear_algebra/mat.hpp>

void coverage_mat_init_rows_mismatch()
{
    using Mat2 = omath::Mat<2,2>;
    // Intentionally construct with too many rows to hit the throw site
    try
    {
        (void)Mat2{{{1.0f,2.0f}, {3.0f,4.0f}, {5.0f,6.0f}}};
    }
    catch (...) { }
}

void coverage_mat_init_columns_mismatch()
{
    using Mat2 = omath::Mat<2,2>;
    try
    {
        (void)Mat2{{{1.0f,2.0f}, {3.0f}}};
    }
    catch (...) { }
}

// Force explicit instantiation so the constructor (and its throw-sites)
// are emitted into this translation unit and can be attributed by
// coverage tools to the header lines.
template class omath::Mat<2, 2, float, omath::MatStoreType::ROW_MAJOR>;
