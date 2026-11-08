# Changelog - quickhub-core

## [2.0.0] - 2025-10-18

### Major Changes
- **Qt 6 Migration**: Complete migration from Qt 5 to Qt 6
- Updated CMake build system to 3.26+
- C++17 standard compliance

### Fixed
- Fixed `QQmlListProperty` constructor signature (changed from reference to pointer)
- Replaced deprecated `QList::toSet()` with range constructor `QSet<T>(begin, end)`
- Changed `QMap` to `QMultiMap` for proper multi-value support
- Updated `insertMulti()` → `insert()` for QMultiMap
- Replaced removed `qBinaryFind()` with `std::lower_bound()`
- Fixed `SKIP_EMPTY_PARTS` constant → `Qt::SkipEmptyParts`
- Updated `QStandardPaths::DataLocation` → `AppLocalDataLocation`
- Added missing implementations:
  - `AutomationEngine::AutomationEngine()` constructor
  - `AutomationEngine::instanciateRule()` method
  - `ServiceHandlerFactory::ServiceHandlerFactory()` constructor
- Added missing `#include <algorithm>` for std algorithms
- Added missing `#include <QMultiMap>` header

### Changed
- Updated all Qt module references to Qt6:: namespace
- Modernized CMake configuration with:
  - Proper target exports
  - Generator expressions for include directories
  - Optional GUI support (`QH_NO_GUI` option)
- Enhanced include directory structure for better modularity

### Added
- `AutomationEngine.cpp` - Stub implementation for QML automation
- `ServiceHandlerFactory.cpp` - Stub implementation for service handling
- Comprehensive include paths for all subsystems
- Support for conditional GUI compilation

### Technical Details
- Module builds as shared library: `libQHCore.so` (~4.3 MB)
- Installed as plugin in `bin/plugins/`
- Tracks `upgrade-to-qt6` branch
- Compatible with Qt 6.2+

## [1.0.0] - Previous Release

Initial Qt 5 implementation with:
- Core resource management
- WebSocket communication
- Authentication system
- Device management
- Storage backends
