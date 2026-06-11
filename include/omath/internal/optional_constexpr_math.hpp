//
// Created by orange on 6/11/2026.
//
#pragma once


#ifdef OMATH_USE_GCEM
#include <gcem.hpp>
#define OMATH_CONSTEXPR constexpr
#else
#define OMATH_CONSTEXPR
#endif