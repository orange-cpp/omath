//
// Created by Vlad on 4/18/2025.
//
#include "omath/3d_primitives/box.hpp"

namespace omath::primitives
{
    std::array<Triangle<Vector3<float>>, 12> create_box(const Vector3<float>& top, const Vector3<float>& bottom,
                                                        const Vector3<float>& dir_forward,
                                                        const Vector3<float>& dir_right, const float ratio) noexcept
    {
        const auto height = top.distance_to(bottom);
        const auto side_size = height / ratio;

        // corner layout (0‑3 bottom, 4‑7 top)
        std::array<Vector3<float>, 8> p;
        std::get<0>(p) = bottom + (dir_forward + dir_right) * side_size; // front‑right‑bottom
        std::get<1>(p) = bottom + (dir_forward - dir_right) * side_size; // front‑left‑bottom
        std::get<2>(p) = bottom + (-dir_forward + dir_right) * side_size; // back‑right‑bottom
        std::get<3>(p) = bottom + (-dir_forward - dir_right) * side_size; // back‑left‑bottom
        std::get<4>(p) = top + (dir_forward + dir_right) * side_size; // front‑right‑top
        std::get<5>(p) = top + (dir_forward - dir_right) * side_size; // front‑left‑top
        std::get<6>(p) = top + (-dir_forward + dir_right) * side_size; // back‑right‑top
        std::get<7>(p) = top + (-dir_forward - dir_right) * side_size; // back‑left‑top

        std::array<Triangle<Vector3<float>>, 12> poly;

        // bottom face (+Y up ⇒ wind CW when viewed from above)
        std::get<0>(poly) = {std::get<0>(p), std::get<2>(p), std::get<3>(p)};
        std::get<1>(poly) = {std::get<0>(p), std::get<3>(p), std::get<1>(p)};

        // top face
        std::get<2>(poly) = {std::get<4>(p), std::get<7>(p), std::get<6>(p)};
        std::get<3>(poly) = {std::get<4>(p), std::get<5>(p), std::get<7>(p)};

        // front face
        std::get<4>(poly) = {std::get<0>(p), std::get<5>(p), std::get<1>(p)};
        std::get<5>(poly) = {std::get<0>(p), std::get<4>(p), std::get<5>(p)};

        // right face
        std::get<6>(poly) = {std::get<0>(p), std::get<6>(p), std::get<2>(p)};
        std::get<7>(poly) = {std::get<0>(p), std::get<4>(p), std::get<6>(p)};

        // back face
        std::get<8>(poly) = {std::get<2>(p), std::get<7>(p), std::get<3>(p)};
        std::get<9>(poly) = {std::get<2>(p), std::get<6>(p), std::get<7>(p)};

        // left face
        std::get<10>(poly) = {std::get<1>(p), std::get<7>(p), std::get<5>(p)};
        std::get<11>(poly) = {std::get<1>(p), std::get<3>(p), std::get<7>(p)};

        return poly;
    }
} // namespace omath::primitives
