//
// Created by Vlad on 3/22/2025.
//

export module omath.unity_engine.formulas;
export import omath.unity_engine.constants;

export namespace omath::unity_engine
{
    [[nodiscard]]
    Vector3<float> forward_vector(const ViewAngles& angles) noexcept;

    [[nodiscard]]
    Vector3<float> right_vector(const ViewAngles& angles) noexcept;

    [[nodiscard]]
    Vector3<float> up_vector(const ViewAngles& angles) noexcept;

    [[nodiscard]] Mat4X4 calc_view_matrix(const ViewAngles& angles, const Vector3<float>& cam_origin) noexcept;

    [[nodiscard]]
    Mat4X4 rotation_matrix(const ViewAngles& angles) noexcept;

    [[nodiscard]]
    Mat4X4 calc_perspective_projection_matrix(float field_of_view, float aspect_ratio, float near, float far) noexcept;
} // namespace omath::unity_engine
