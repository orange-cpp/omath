//
// Created by Vlad on 6/9/2024.
//
export module omath.projectile_prediction.target;
export import omath.linear_algebra.vector3;

export namespace omath::projectile_prediction
{
    class Target final
    {
    public:
        Vector3<float> m_origin;
        Vector3<float> m_velocity;
        bool m_is_airborne{};
    };
} // namespace omath::projectile_prediction