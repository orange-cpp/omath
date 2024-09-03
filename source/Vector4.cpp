//
// Vector4.cpp
//

#include "omath/Vector4.h"

#include <algorithm>
#include <cmath>

namespace omath
{

    float Vector4::Length() const
    {
        return std::sqrt(LengthSqr());
    }
}
