//
// Created by Vlad on 8/10/2025.
//

export module omath.unity_engine.camera_trait;
export import omath.unity_engine.constants;
export import omath.projection.camera;

export namespace omath::unity_engine
{
    class CameraTrait final
    {
    public:
        [[nodiscard]]
        static ViewAngles calc_look_at_angle(const Vector3<float>& cam_origin, const Vector3<float>& look_at) noexcept;

        [[nodiscard]]
        static Mat4X4 calc_view_matrix(const ViewAngles& angles, const Vector3<float>& cam_origin) noexcept;
        [[nodiscard]]
        static Mat4X4 calc_projection_matrix(const projection::FieldOfView& fov, const projection::ViewPort& view_port,
                                             float near, float far) noexcept;
    };

} // namespace omath::unity_engine