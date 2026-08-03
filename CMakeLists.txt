#   2log.io
#   Copyright (C) 2025 - 2log.io | mail@2log.io,  mail@friedemann-metzger.de,
#   drook207@gmail.com
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU Affero General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Affero General Public License for more details.
#
#   You should have received a copy of the GNU Affero General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

cmake_minimum_required(VERSION 3.26)

project(quickhub-core VERSION 0.1.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core WebSockets Concurrent Gui Qml)

qt_standard_project_setup()

# Define whether to include GUI components
option(QH_NO_GUI "Disable GUI components" OFF)

set(CORE_SOURCES
    src/QHCorePlugin.cpp
    src/QHCorePlugin.h
    src/qhcore_global.h
    src/Connection/Connection.cpp
    src/Connection/Connection.h
    src/Connection/IConnectable.h
    src/Connection/ISocket.h
    src/Connection/VirtualConnection.cpp
    src/Connection/VirtualConnection.h
    src/Server/Authentication/AuthentificationService.cpp
    src/Server/Authentication/AuthentificationService.h
    src/Server/Authentication/DefaultAuthenticator.cpp
    src/Server/Authentication/DefaultAuthenticator.h
    src/Server/Authentication/IAuthenticator.h
    src/Server/Authentication/IIdentitiy.cpp
    src/Server/Authentication/IIdentitiy.h
    src/Server/Authentication/IUser.cpp
    src/Server/Authentication/IUser.h
    src/Server/Authentication/User.cpp
    src/Server/Authentication/User.h
    src/Server/Authentication/ApiToken/ApiTokenIdentity.cpp
    src/Server/Authentication/ApiToken/ApiTokenIdentity.h
    src/Server/Authentication/ApiToken/ApiTokenManager.cpp
    src/Server/Authentication/ApiToken/ApiTokenManager.h
    src/Server/Authentication/ApiToken/ApiTokenService.cpp
    src/Server/Authentication/ApiToken/ApiTokenService.h
    src/Server/Authentication/ApiToken/ApiTokenListResource.cpp
    src/Server/Authentication/ApiToken/ApiTokenListResource.h
    src/Server/Authentication/ApiToken/ApiTokenListResourceFactory.cpp
    src/Server/Authentication/ApiToken/ApiTokenListResourceFactory.h
    src/Server/Automation/AutomationEngine.cpp
    src/Server/Automation/AutomationEngine.h
    src/Server/Automation/QmlComponents/AutomationRule.cpp
    src/Server/Automation/QmlComponents/AutomationRule.h
    src/Server/Automation/QmlComponents/DeviceQmlAdapter.cpp
    src/Server/Automation/QmlComponents/DeviceQmlAdapter.h
    src/Server/Defines/ErrDef.h
    src/Server/Devices/DeviceHandle.cpp
    src/Server/Devices/DeviceHandle.h
    src/Server/Devices/DeviceManager.cpp
    src/Server/Devices/DeviceManager.h
    src/Server/Devices/DevicePermissionManager.cpp
    src/Server/Devices/DevicePermissionManager.h
    src/Server/Devices/DeviceProperty.cpp
    src/Server/Devices/DeviceProperty.h
    src/Server/Devices/DeviceService.cpp
    src/Server/Devices/DeviceService.h
    src/Server/Devices/DeviceUpdateLogic.cpp
    src/Server/Devices/DeviceUpdateLogic.h
    src/Server/Devices/IDevice.cpp
    src/Server/Devices/IDevice.h
    src/Server/Devices/IDevicePermissionController.cpp
    src/Server/Devices/IDevicePermissionController.h
    src/Server/Logging/Logger.cpp
    src/Server/Logging/Logger.h
    src/Server/Resources/ListResource/IListResourceStorage.h
    src/Server/Resources/ListResource/IListResourceStorageFactory.h
    src/Server/Resources/ListResource/ListAccessProxy.cpp
    src/Server/Resources/ListResource/ListAccessProxy.h
    src/Server/Resources/ListResource/ListResource.cpp
    src/Server/Resources/ListResource/ListResource.h
    src/Server/Resources/ListResource/ListResourceFactory.cpp
    src/Server/Resources/ListResource/ListResourceFactory.h
    src/Server/Resources/ListResource/SharedQObjectListResource.h
    src/Server/Resources/ListResource/SharedQObjectListResource.cpp
    src/Server/Resources/ListResource/QObjectListResource.cpp
    src/Server/Resources/ListResource/QObjectListResource.h
    src/Server/Resources/ObjectResource/IObjectResourceStorage.h
    src/Server/Resources/ObjectResource/IObjectResourceStorageFactory.h
    src/Server/Resources/ObjectResource/ObjectAccessProxy.cpp
    src/Server/Resources/ObjectResource/ObjectAccessProxy.h
    src/Server/Resources/ObjectResource/ObjectResource.cpp
    src/Server/Resources/ObjectResource/ObjectResource.h
    src/Server/Resources/ObjectResource/ObjectResourceFactory.cpp
    src/Server/Resources/ObjectResource/ObjectResourceFactory.h
    src/Server/Resources/ObjectResource/QObjectResource.cpp
    src/Server/Resources/ObjectResource/QObjectResource.h
    src/Server/Resources/ResourceManager/IResource.cpp
    src/Server/Resources/ResourceManager/IResource.h
    src/Server/Resources/ResourceManager/IResourceFactory.h
    src/Server/Resources/ResourceManager/ResourceManager.cpp
    src/Server/Resources/ResourceManager/ResourceManager.h
    src/Server/Services/IService.h
    src/Server/Services/ServiceManager.cpp
    src/Server/Services/ServiceManager.h
    src/Server/Settings/SettingsManager.cpp
    src/Server/Settings/SettingsManager.h
    src/Server/Settings/SettingsResource.cpp
    src/Server/Settings/SettingsResource.h
    src/SocketApi/DataHandler/Lists/IList.cpp
    src/SocketApi/DataHandler/Lists/IList.h
    src/SocketApi/DataHandler/Lists/ListHandler.cpp
    src/SocketApi/DataHandler/Lists/ListHandler.h
    src/SocketApi/DataHandler/Lists/ListHandlerFactory.cpp
    src/SocketApi/DataHandler/Lists/ListHandlerFactory.h
    src/SocketApi/DataHandler/Lists/ListWrapper/DeviceHandleListWrapper.cpp
    src/SocketApi/DataHandler/Lists/ListWrapper/DeviceHandleListWrapper.h
    src/SocketApi/DataHandler/Lists/ListWrapper/DeviceListWrapper.cpp
    src/SocketApi/DataHandler/Lists/ListWrapper/DeviceListWrapper.h
    src/SocketApi/DataHandler/Lists/ListWrapper/UserListWrapper.cpp
    src/SocketApi/DataHandler/Lists/ListWrapper/UserListWrapper.h
    src/SocketApi/Devices/DeviceHandleHandler.cpp
    src/SocketApi/Devices/DeviceHandleHandler.h
    src/SocketApi/Devices/DeviceHandleHandlerFactory.cpp
    src/SocketApi/Devices/DeviceHandleHandlerFactory.h
    src/SocketApi/Devices/SocketDevice.cpp
    src/SocketApi/Devices/SocketDevice.h
    src/SocketApi/Devices/SocketDeviceHandler.cpp
    src/SocketApi/Devices/SocketDeviceHandler.h
    src/SocketApi/ResourceHandler/Services/ServiceHandlerFactory.cpp
    src/SocketApi/ResourceHandler/Services/ServiceHandlerFactory.h
    src/SocketApi/ResourceHandler/SynchronizedList/SynchronizedListHandler.cpp
    src/SocketApi/ResourceHandler/SynchronizedList/SynchronizedListHandler.h
    src/SocketApi/ResourceHandler/SynchronizedList/SynchronizedListHandlerFactory.cpp
    src/SocketApi/ResourceHandler/SynchronizedList/SynchronizedListHandlerFactory.h
    src/SocketApi/ResourceHandler/SynchronizedObject/SynchronizedObjectHandler.cpp
    src/SocketApi/ResourceHandler/SynchronizedObject/SynchronizedObjectHandler.h
    src/SocketApi/ResourceHandler/SynchronizedObject/SynchronizedObjectHandlerFactory.cpp
    src/SocketApi/ResourceHandler/SynchronizedObject/SynchronizedObjectHandlerFactory.h
    src/SocketApi/Services/ServiceRequestHandler.cpp
    src/SocketApi/Services/ServiceRequestHandler.h
    src/SocketApi/Session/SessionHandler.cpp
    src/SocketApi/Session/SessionHandler.h
    src/SocketApi/SocketCore/IRequestHandler.h
    src/SocketApi/SocketCore/IResourceHandler.cpp
    src/SocketApi/SocketCore/IResourceHandler.h
    src/SocketApi/SocketCore/IResourceHandlerFactory.h
    src/SocketApi/SocketCore/SocketResourceManager.cpp
    src/SocketApi/SocketCore/SocketResourceManager.h
    src/SocketApi/SocketServer.cpp
    src/SocketApi/SocketServer.h
    src/Storage/FileSystemLoader.cpp
    src/Storage/FileSystemLoader.h
    src/Storage/FileSystemPaths.cpp
    src/Storage/FileSystemPaths.h
    src/Storage/ListResourceFileSystemStorage.cpp
    src/Storage/ListResourceFileSystemStorage.h
    src/Storage/ListResourceMongoDBStorageFactory.cpp
    src/Storage/ListResourceMongoDBStorageFactory.h
    src/Storage/ListResourceTemporaryStorage.cpp
    src/Storage/ListResourceTemporaryStorage.h
    src/Storage/ObjectResourceFilesystemStorage.cpp
    src/Storage/ObjectResourceFilesystemStorage.h
    src/Storage/QObjectResourceFilesystemStorage.cpp
    src/Storage/QObjectResourceFilesystemStorage.h
    src/Storage/QObjectSerializer.cpp
    src/Storage/QObjectSerializer.h
)

# GUI-dependent sources
if(NOT QH_NO_GUI)
  list(APPEND CORE_SOURCES
        src/Server/Resources/ImageResource/IImageResourceStorage.h
        src/Server/Resources/ImageResource/IImageResourceStorageFactory.h
        src/Server/Resources/ImageResource/ImageResource.cpp
        src/Server/Resources/ImageResource/ImageResource.h
        src/Server/Resources/ImageResource/ImageResourceFactory.cpp
        src/Server/Resources/ImageResource/ImageResourceFactory.h
        src/SocketApi/ResourceHandler/ImageCollection/ImageCollectionHandler.cpp
        src/SocketApi/ResourceHandler/ImageCollection/ImageCollectionHandler.h
        src/SocketApi/ResourceHandler/ImageCollection/ImageCollectionHandlerFactory.cpp
        src/SocketApi/ResourceHandler/ImageCollection/ImageCollectionHandlerFactory.h
        src/Storage/ImageResourceFilesystemStorage.cpp
        src/Storage/ImageResourceFilesystemStorage.h
    )
endif()

qt_add_library(QHCore SHARED
    ${CORE_SOURCES}
)

target_compile_definitions(QHCore PRIVATE
    COREPLUGIN_LIBRARY
    NO_PLUGIN
)

if(QH_NO_GUI)
  target_compile_definitions(QHCore PRIVATE NO_GUI)
endif()

# Handle version-specific defines for Qt
if(Qt6_VERSION VERSION_GREATER_EQUAL 5.14.0)
  target_compile_definitions(QHCore PRIVATE
        SKIP_EMPTY_PARTS=Qt::SkipEmptyParts
    )
else()
  target_compile_definitions(QHCore PRIVATE
        SKIP_EMPTY_PARTS=QString::SkipEmptyParts
    )
endif()

target_link_libraries(QHCore PRIVATE
    Qt6::Core
    Qt6::WebSockets
    Qt6::Qml
    Qt6::Concurrent
    QHPluginSystem
)

if(NOT QH_NO_GUI)
  target_link_libraries(QHCore PRIVATE Qt6::Gui)
endif()

target_include_directories(QHCore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Connection>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Server>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Server/Authentication>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Server/Authentication/ApiToken>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Server/Automation>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Server/Automation/QmlComponents>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Server/Defines>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Server/Devices>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Server/Logging>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Server/Resources>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Server/Resources/ImageResource>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Server/Resources/ListResource>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Server/Resources/ObjectResource>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Server/Resources/ResourceManager>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Server/Services>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Server/Settings>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/SocketApi>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/SocketApi/DataHandler>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/SocketApi/DataHandler/Lists>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/SocketApi/DataHandler/Lists/ListWrapper>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/SocketApi/Devices>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/SocketApi/ResourceHandler>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/SocketApi/ResourceHandler/ImageCollection>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/SocketApi/ResourceHandler/Services>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/SocketApi/ResourceHandler/SynchronizedList>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/SocketApi/ResourceHandler/SynchronizedObject>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/SocketApi/Services>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/SocketApi/Session>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/SocketApi/SocketCore>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Storage>
    $<INSTALL_INTERFACE:include/quickhub-core>
)

# Set output directory to plugins
set_target_properties(QHCore PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins
)

install(TARGETS QHCore
    EXPORT QHCoreTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/plugins
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/plugins
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/plugins
)

install(DIRECTORY src/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/quickhub-core
    FILES_MATCHING PATTERN "*.h"
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/QHCoreConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/QHCoreConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/quickhub-core
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/QHCoreConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/QHCoreConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/QHCoreConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/quickhub-core
)

install(EXPORT QHCoreTargets
    FILE QHCoreTargets.cmake
    NAMESPACE quickhub::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/quickhub-core
)
