//
// Created by Vlad on 9/1/2025.
//

export module omath.projectile_prediction.projectile;
export import omath.vector3;

export namespace omath::projectile_prediction
{
    class Projectile final
    {
    public:
        Vector3<float> m_origin;
        float m_launch_speed{};
        float m_gravity_scale{};
    };
} // namespace omath::projectile_prediction