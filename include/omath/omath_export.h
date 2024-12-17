#pragma once

/* Export prefix for functions */
#ifdef _MSC_VER
	/* MSVC */
#	define OMATH_API_EXPORT __declspec(dllexport)
#else
	/* GCC/Clang */
#	define OMATH_API_EXPORT __attribute__((visibility("default")))
#endif

/* Import prefix for functions */
#ifdef _MSC_VER
#	define OMATH_API_IMPORT __declspec(dllimport)
#else
#	define OMATH_API_IMPORT extern
#endif

/* Resolve import/export */
#ifdef OMATH_EXPORT
#	define OMATH_API OMATH_API_EXPORT
#else
#	define OMATH_API OMATH_API_IMPORT
#endif