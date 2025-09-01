//
// Created by Vlad on 9/1/2025.
//
module;
#include <optional>

export module omath.projectile_prediction.proj_pred_engine;

import omath.projectile_prediction.target;
import omath.projectile_prediction.projectile;

namespace omath::projectile_prediction
{
    class ProjPredEngineInterface
    {
    public:
        [[nodiscard]]
        virtual std::optional<Vector3<float>> maybe_calculate_aim_point(const Projectile& projectile,
                                                                        const Target& target) const = 0;
        virtual ~ProjPredEngineInterface() = default;
    };
} // namespace omath::projectile_prediction