# quickhub-core

The core plugin framework for QuickHub - a modular Qt-based server framework.

## Overview

QuickHub-core provides the fundamental infrastructure for building modular server applications with Qt. It includes resource management, authentication, device management, WebSocket communication, storage backends, and automation capabilities.

## Features

### Core Infrastructure
- Plugin-based architecture
- Resource management system (Lists, Objects, Images)
- RESTful resource handling
- WebSocket-based communication
- Session management

### Authentication & Authorization
- User authentication system
- Identity management
- Permission-based access control
- Token-based authentication

### Device Management
- Device registration and discovery
- Device handle management
- Device properties and functions
- Permission-based device access

### Storage Backends
- Filesystem storage
- MongoDB integration
- Temporary/in-memory storage
- Image resource storage

### Automation
- QML-based automation engine
- Automation rule management
- Device QML adapters

### Services
- Service registration and management
- Service request handling
- Service factory pattern

## Requirements

- Qt 6.2 or later
- CMake 3.26 or later
- C++17 compatible compiler
- Optional: MongoDB C++ driver for database storage

## Building

This module is part of the 2log.io project and is built as a submodule:

```bash
# From the main 2log.io directory
mkdir build && cd build
cmake ..
cmake --build . --target QHCore
```

## Qt 6 Migration

As of version 2.0, this module has been migrated to Qt 6. Major changes include:
- Updated to Qt6:: module namespace
- Fixed `QQmlListProperty` constructor (reference → pointer)
- Replaced `QList::toSet()` with range constructor
- Changed `QMap` to `QMultiMap` for multi-value support
- Replaced `qBinaryFind()` with `std::lower_bound()`
- Updated `SKIP_EMPTY_PARTS` constant to `Qt::SkipEmptyParts`
- Fixed `QStandardPaths::DataLocation` → `AppLocalDataLocation`
- Added stub implementations for `AutomationEngine` and `ServiceHandlerFactory`
- CMake modernization

## Configuration Options

- `QH_NO_GUI`: Disable GUI components (default: OFF)

## Dependencies

### Qt Modules
- Qt6::Core
- Qt6::WebSockets
- Qt6::Qml
- Qt6::Concurrent
- Qt6::Gui (optional, if QH_NO_GUI=OFF)

### Internal Dependencies
- QHPluginSystem

## Outputs

- **libQHCore.so** (~4.3 MB) - Core framework library (plugin)

## Architecture

QuickHub uses a resource-based architecture where:
- **Resources** represent data entities (lists, objects, images)
- **Handlers** manage WebSocket communication for resources
- **Factories** create resource and handler instances
- **Storage** provides persistence backends

## License

This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de
