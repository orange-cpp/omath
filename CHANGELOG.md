# Changelog

All notable changes to this project from v2.6.1 to v3.6.2 will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [v3.6.2] - 2025-09-15

### Fixed
- Repair omath.hpp by removing unexisting header include (#69)

## [v3.6.1] - 2025-09-14

### Added
- 2D screen to world conversion functionality
- Overload for screen_to_world that accepts a 2D screen position

### Changed
- Renamed screen_to_dnc to screen_to_ndc for clarity

### Fixed
- Enabled formatting support for Angle objects using std::format

### Documentation
- Improved README readability and button aesthetics
- Updated README with gallery, install and usage information
- Repositioned YouTube preview link
- Removed unnecessary div tag from documentation

## [v3.6.0] - 2025-09-08

### Added
- Screen to world space conversion functionality
- Projection test for world-to-screen consistency
- Support for handling cases where inverse view projection matrix is singular
- Support for cases when world position is out of screen bounds

### Changed
- Exposed Camera class to unit tests
- Simplified raycast early exit condition

### Fixed
- Fixed float type conversion in world_to_screen test

## [v3.5.3] - 2025-09-08

### Added
- Angle class with normalization support
- Different normalization and clamping strategies for angles
- Trigonometric functions and arithmetic operators for Angle class
- Unit tests for Angle class functionality

### Removed
- **BREAKING**: Deprecated Matrix class and associated source files and unit tests

### Changed
- Disabled unity builds to address compilation issues

## [v3.5.2] - 2025-09-03

### Added
- Initial project logo design
- Binary file omath.psd for logo design

### Changed
- Updated README to use local image links instead of Imgur links
- Replaced project banner with dedicated logo

### Documentation
- Acknowledged logo design contribution in credits

## [v3.5.1] - 2025-08-31

### Changed
- **BREAKING**: Moved linear algebra headers to new subdirectory for better project structure
- Updated include paths to reflect directory change
- Updated include paths in omath.hpp to be more explicit
- Removed unnecessary include of vector3.hpp from vector2.hpp

### Added
- Added vcpkg to tracked repositories

## [v3.5.0] - 2025-08-29

### Added
- Left-handed and right-handed orthographic projection matrices
- Plane primitive generation function that generates plane from two vertices, direction vector, and size

### Changed
- Updated project version to 3.5.0
- Added compile definition to expose project version via OMATH_VERSION macro
- Simplified plane creation logic for better performance

### Fixed
- Fixed plane triangle generation vertex order to address potential winding order issues

## [v3.4.0] - 2025-08-26

### Added
- **NEW**: std::format support for math types
- std::formatter specializations for math objects

### Changed
- Updated project version from 3.3.0 to 3.4.0

## [v3.3.1] - 2025-08-25

### Fixed
- Various minor fixes and improvements

## [v3.3.0] - 2025-08-25

### Added
- **NEW**: Unreal Engine math library integration

## [v3.2.3] - 2025-08-12

### Fixed
- Various stability improvements

## [v3.2.2] - 2025-08-12

### Fixed
- Additional stability improvements

## [v3.2.1] - 2025-08-12

### Fixed
- Minor bug fixes

## [v3.2.0] - 2025-08-12

### Added
- **NEW**: Comparison operators for Vector types (==, !=, <, >, <=, >=)

## [v3.1.0] - 2025-08-10

### Changed
- **BREAKING**: Refactored camera and prediction functionality using traits
- Improved type safety and performance through trait-based design

## [v3.0.5] - 2025-08-06

### Added
- Engine traits for projectile prediction
- Enhanced projectile prediction capabilities

## [v3.0.4.2] - 2025-08-04

### Fixed
- Various bug fixes and stability improvements

## [v3.0.4.1] - 2025-08-03

### Fixed
- Minor bug fixes

## [v3.0.4] - 2025-08-03

### Changed
- **BREAKING**: Refactored projectile prediction engine for better performance and maintainability

## [v3.0.3.1] - 2025-07-15

### Added
- Option to enable legacy classes for backward compatibility

### Documentation
- Updated README.md and INSTALL.md with community contributions

## [v3.0.3] - 2025-07-07

### Fixed
- Check for ImGUI dependency in omathConfig.cmake.in

## [v3.0.2.1] - 2025-06-17

### Documentation
- Added Writerside documentation integration

## [v3.0.2] - 2025-05-13

### Fixed
- Various bug fixes and improvements

## [v3.0.1] - 2025-05-04

### Added
- **NEW**: noexcept specifications for better exception safety

## [v3.0.0] - 2025-05-03

### Added
- **NEW**: CI/CD pipeline integration
- cmake-multi-platform.yml workflow
- New code style implementation

### Changed
- **BREAKING**: Major version bump with new architecture
- **BREAKING**: New code style applied throughout the project

---

## Summary of Major Changes (v2.6.1 → v3.6.2)

### 🚀 New Features
- **Angle Class**: Complete angle handling with normalization strategies
- **Screen/World Conversion**: Bidirectional screen-world coordinate conversion
- **std::format Support**: Modern C++ formatting for all math types
- **Unreal Engine Integration**: Math library compatibility
- **Vector Comparisons**: Full comparison operator support
- **Orthographic Projections**: Left and right-handed matrices
- **Primitive Generation**: Plane primitive creation utilities
- **Projectile Prediction**: Enhanced physics prediction capabilities

### 💥 Breaking Changes
- Matrix class removed (use Mat class instead)
- Linear algebra headers moved to subdirectory
- Camera and prediction APIs refactored with traits
- Projectile prediction engine restructured

### 🔧 Developer Experience
- CI/CD pipeline added
- Logo and branding introduced
- Comprehensive documentation updates
- noexcept specifications added
- Unity builds disabled for compilation compatibility

### 📈 Version Progression
This changelog covers the evolution from v2.6.1 through the major v3.0.0 release to the current v3.6.2, representing significant architectural improvements and feature additions to the OMath library.

[v3.6.2]: https://github.com/orange-cpp/omath/compare/v3.6.1...v3.6.2
[v3.6.1]: https://github.com/orange-cpp/omath/compare/v3.6.0...v3.6.1
[v3.6.0]: https://github.com/orange-cpp/omath/compare/v3.5.3...v3.6.0
[v3.5.3]: https://github.com/orange-cpp/omath/compare/v3.5.2...v3.5.3
[v3.5.2]: https://github.com/orange-cpp/omath/compare/v3.5.1...v3.5.2
[v3.5.1]: https://github.com/orange-cpp/omath/compare/v3.5.0...v3.5.1
[v3.5.0]: https://github.com/orange-cpp/omath/compare/v3.4.0...v3.5.0
[v3.4.0]: https://github.com/orange-cpp/omath/compare/v3.3.1...v3.4.0
[v3.3.1]: https://github.com/orange-cpp/omath/compare/v3.3.0...v3.3.1
[v3.3.0]: https://github.com/orange-cpp/omath/compare/v3.2.3...v3.3.0
[v3.2.3]: https://github.com/orange-cpp/omath/compare/v3.2.2...v3.2.3
[v3.2.2]: https://github.com/orange-cpp/omath/compare/v3.2.1...v3.2.2
[v3.2.1]: https://github.com/orange-cpp/omath/compare/v3.2.0...v3.2.1
[v3.2.0]: https://github.com/orange-cpp/omath/compare/v3.1.0...v3.2.0
[v3.1.0]: https://github.com/orange-cpp/omath/compare/v3.0.5...v3.1.0
[v3.0.5]: https://github.com/orange-cpp/omath/compare/v3.0.4.2...v3.0.5
[v3.0.4.2]: https://github.com/orange-cpp/omath/compare/v3.0.4.1...v3.0.4.2
[v3.0.4.1]: https://github.com/orange-cpp/omath/compare/v3.0.4...v3.0.4.1
[v3.0.4]: https://github.com/orange-cpp/omath/compare/v3.0.3.1...v3.0.4
[v3.0.3.1]: https://github.com/orange-cpp/omath/compare/v3.0.3...v3.0.3.1
[v3.0.3]: https://github.com/orange-cpp/omath/compare/v3.0.2.1...v3.0.3
[v3.0.2.1]: https://github.com/orange-cpp/omath/compare/v3.0.2...v3.0.2.1
[v3.0.2]: https://github.com/orange-cpp/omath/compare/v3.0.1...v3.0.2
[v3.0.1]: https://github.com/orange-cpp/omath/compare/v3.0.0...v3.0.1
[v3.0.0]: https://github.com/orange-cpp/omath/compare/v2.6.1...v3.0.0