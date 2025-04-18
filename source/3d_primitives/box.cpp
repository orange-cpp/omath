//
// Created by Vlad on 4/18/2025.
//
#include "omath/3d_primitives/box.hpp"


namespace omath::primitives
{
    std::array<Triangle<Vector3<float>>, 12> CreateBox(const Vector3<float>& top, const Vector3<float>& bottom,
                                                       const Vector3<float>& dirForward,
                                                       const Vector3<float>& dirRight,
                                                       const float ratio)
    {
        const auto height = top.DistTo(bottom);
        const auto sideSize = height / ratio;

        // corner layout (0‑3 bottom, 4‑7 top)
        std::array<Vector3<float>, 8> p;
        p[0] = bottom + (dirForward + dirRight) * sideSize; // front‑right‑bottom
        p[1] = bottom + (dirForward - dirRight) * sideSize; // front‑left‑bottom
        p[2] = bottom + (-dirForward + dirRight) * sideSize; // back‑right‑bottom
        p[3] = bottom + (-dirForward - dirRight) * sideSize; // back‑left‑bottom
        p[4] = top + (dirForward + dirRight) * sideSize; // front‑right‑top
        p[5] = top + (dirForward - dirRight) * sideSize; // front‑left‑top
        p[6] = top + (-dirForward + dirRight) * sideSize; // back‑right‑top
        p[7] = top + (-dirForward - dirRight) * sideSize; // back‑left‑top

        std::array<Triangle<Vector3<float>>, 12> poly;

        // bottom face (+Y up ⇒ wind CW when viewed from above)
        poly[0] = {p[0], p[2], p[3]};
        poly[1] = {p[0], p[3], p[1]};

        // top face
        poly[2] = {p[4], p[7], p[6]};
        poly[3] = {p[4], p[5], p[7]};

        // front face
        poly[4] = {p[0], p[5], p[1]};
        poly[5] = {p[0], p[4], p[5]};

        // right face
        poly[6] = {p[0], p[6], p[2]};
        poly[7] = {p[0], p[4], p[6]};

        // back face
        poly[8] = {p[2], p[7], p[3]};
        poly[9] = {p[2], p[6], p[7]};

        // left face
        poly[10] = {p[1], p[7], p[5]};
        poly[11] = {p[1], p[3], p[7]};

        return poly;
    }
}
