//
// Created by Vlad on 4/18/2025.
//
#include "omath/3d_primitives/box.hpp"



namespace omath::primitives
{

    std::array<Triangle<Vector3<float>>, 8> CreateBox(const Vector3<float>& top, const Vector3<float>& bottom,
                                            const Vector3<float>& dirForward,
                                            const Vector3<float>& dirRight,
                                            const float ratio)
    {
        const auto height = top.DistTo(bottom);
        const auto sideSize = height / ratio;

        std::array<Vector3<float>, 8> points;

        points[0] = bottom + (dirForward + dirRight) * sideSize;
        points[1] = bottom + (dirForward - dirRight) * sideSize;

        points[2] = bottom + (-dirForward + dirRight) * sideSize;
        points[3] = bottom + (-dirForward - dirRight) * sideSize;

        points[4] = top + (dirForward + dirRight) * sideSize;
        points[5] = top + (dirForward - dirRight) * sideSize;

        points[6] = top + (-dirForward + dirRight) * sideSize;
        points[7] = top + (-dirForward - dirRight) * sideSize;


        std::array<Triangle<Vector3<float>>, 8> polygons;

        polygons[0] = {points[0], points[2], points[3]};
        return polygons;
    }
}
