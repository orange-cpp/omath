//
// omath.hpp - Main header file that includes all omath library components
// Created for the omath library
//

#pragma once

// Basic math utilities
#include "omath/angles.hpp"
#include "omath/angle.hpp"

// Vector classes (in dependency order)
#include "omath/linear_algebra/vector2.hpp"
#include "omath/linear_algebra/vector4.hpp"
#include "omath/linear_algebra/vector3.hpp"

// Matrix classes
#include "linear_algebra/matrix.hpp"
#include "omath/linear_algebra/mat.hpp"

// Color functionality
#include "omath/color.hpp"

// Geometric primitives
#include "omath/triangle.hpp"
#include "omath/view_angles.hpp"

// 3D primitives
#include "omath/3d_primitives/box.hpp"

// Collision detection
#include "omath/collision/line_tracer.hpp"

// Pathfinding algorithms
#include "omath/pathfinding/a_star.hpp"
#include "omath/pathfinding/navigation_mesh.hpp"

// Projectile prediction
#include "omath/projectile_prediction/projectile.hpp"
#include "omath/projectile_prediction/target.hpp"
#include "omath/projectile_prediction/proj_pred_engine.hpp"
#include "omath/projectile_prediction/proj_pred_engine_legacy.hpp"
#include "omath/projectile_prediction/proj_pred_engine_avx2.hpp"

// Projection functionality
#include "omath/projection/error_codes.hpp"
#include "omath/projection/camera.hpp"

// Engine-specific implementations

// IW Engine
#include "omath/engines/iw_engine/constants.hpp"
#include "omath/engines/iw_engine/formulas.hpp"
#include "omath/engines/iw_engine/camera.hpp"
#include "omath/engines/iw_engine/traits/camera_trait.hpp"
#include "omath/engines/iw_engine/traits/pred_engine_trait.hpp"

// OpenGL Engine
#include "omath/engines/opengl_engine/constants.hpp"
#include "omath/engines/opengl_engine/formulas.hpp"
#include "omath/engines/opengl_engine/camera.hpp"
#include "omath/engines/opengl_engine/traits/camera_trait.hpp"
#include "omath/engines/opengl_engine/traits/pred_engine_trait.hpp"

// Source Engine
#include "omath/engines/source_engine/constants.hpp"
#include "omath/engines/source_engine/formulas.hpp"
#include "omath/engines/source_engine/camera.hpp"
#include "omath/engines/source_engine/traits/camera_trait.hpp"
#include "omath/engines/source_engine/traits/pred_engine_trait.hpp"

// Unity Engine
#include "omath/engines/unity_engine/constants.hpp"
#include "omath/engines/unity_engine/formulas.hpp"
#include "omath/engines/unity_engine/camera.hpp"
#include "omath/engines/unity_engine/traits/camera_trait.hpp"
#include "omath/engines/unity_engine/traits/pred_engine_trait.hpp"

// Unreal Engine
#include "omath/engines/unreal_engine/constants.hpp"
#include "omath/engines/unreal_engine/formulas.hpp"
#include "omath/engines/unreal_engine/camera.hpp"
#include "omath/engines/unreal_engine/traits/camera_trait.hpp"
#include "omath/engines/unreal_engine/traits/pred_engine_trait.hpp"