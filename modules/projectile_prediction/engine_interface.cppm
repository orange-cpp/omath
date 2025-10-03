//
// Created by Vlad on 10/3/2025.
//
module;
#include <optional>

export module omath.projectile_prediction.engine_interface;
export import omath.linear_algebra.vector3;
export import omath.projectile_prediction.projectile;
export import omath.projectile_prediction.target;

export namespace omath::projectile_prediction
{
    class ProjPredEngineInterface
    {
    public:
        [[nodiscard]]
        virtual std::optional<Vector3<float>> maybe_calculate_aim_point(const Projectile& projectile,
                                                                        const Target& target) const = 0;
        virtual ~ProjPredEngineInterface() = default;
    };
}