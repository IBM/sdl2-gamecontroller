# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.12]
### Breaking Changes
- Added ESM support
- CJS users must now use `const gamecontroller = require('sdl2-gamecontroller').default`

## [1.0.10] - Previous Release
### Added
- Controller battery events
### Fixed
- Build issue with brew on Linux

## [1.0.9] - Previous Release
### Added
- Support for Apple M1

## [1.0.8] - Previous Release
### Changed
- Changed error handling to issue warnings for mismatched params instead of throwing errors
### Fixed
- Issue where undefined parameters in `rumble` and other functions caused errors

## [1.0.7] - Previous Release
### Added
- Support for ASUS ROG Chakram mouse (Requires SDL2 2.0.22)
- Support for setting the polling interval

## [1.0.6] - Previous Release
### Added
- Full TypeScript support with strict event typing
- Event type definitions for all controller events
- Proper type definitions for all controller methods
- Type safety for event emitter methods (on, once, emit)

### Changed
- Improved type definitions for controller events:
    - Button events (up/down states)
    - Axis motion events
    - Sensor events
    - Device events
    - Battery events
    - Touchpad events
- Enhanced TypeScript interfaces for better type inference
- Restructured event handling with proper type safety