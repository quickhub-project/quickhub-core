QT += core websockets concurrent


CONFIG += c++11

CONFIG += plugin
INCLUDEPATH += ../quickhub-pluginsystem/src
INCLUDEPATH += src
LIBS += -lQHPluginSystem -L../bin/lib
DESTDIR = ../bin/plugins
TEMPLATE = lib
DEFINES += COREPLUGIN_LIBRARY

include(src/SocketApi/SocketApi.pri)

SOURCES += src/Server/Authentication/AuthentificationService.cpp \
    src/Server/Authentication/Controller.cpp \
    src/Server/Authentication/IIdentitiy.cpp \
    src/Server/Authentication/User.cpp \
    src/Connection/VirtualConnection.cpp \
    src/Connection/Connection.cpp \
    src/Server/Devices/DevicePermissionManager.cpp \
    src/Server/Devices/DeviceService.cpp \
    src/Server/Devices/DeviceUpdateLogic.cpp \
    src/Server/Devices/IDevicePermissionController.cpp \
    src/Server/Resources/ListResource/ListResource.cpp \
    src/Server/Resources/ListResource/QObjectListResource.cpp \
    src/Server/Resources/ResourceManager/ResourceManager.cpp \
    src/Server/Resources/ObjectResource/ObjectResource.cpp \
    src/Server/Resources/ResourceManager/IResource.cpp \
    src/Server/Devices/DeviceManager.cpp \
    src/Server/Resources/ListResource/ListResourceFactory.cpp \
    src/Server/Resources/ObjectResource/ObjectResourceFactory.cpp \
    src/Server/Devices/IDevice.cpp \
    src/Server/Devices/DeviceHandle.cpp \
    src/QHCorePlugin.cpp \
    src/Server/Devices/DeviceProperty.cpp \
    src/Server/Settings/SettingsManager.cpp \
    src/Server/Settings/SettingsResource.cpp \
    src/Storage/FileSystemPaths.cpp \
    src/Storage/ListResourceFileSystemStorage.cpp \
    src/Server/Resources/ImageResource/ImageResource.cpp \
    src/Server/Resources/ImageResource/ImageResourceFactory.cpp \
    src/Storage/ImageResourceFilesystemStorage.cpp \
    src/Storage/ObjectResourceFilesystemStorage.cpp \
    src/Server/Logging/Logger.cpp \
    src/Server/Services/ServiceManager.cpp \
    src/Storage/ListResourceTemporaryStorage.cpp \
    src/Server/Authentication/IUser.cpp \
    src/Server/Authentication/DefaultAuthenticator.cpp \
    src/Storage/FileSystemLoader.cpp

HEADERS += \
    src/Server/Authentication/AuthentificationService.h \
    src/Server/Authentication/Controller.h \
    src/Server/Authentication/IIdentitiy.h \
    src/Server/Authentication/User.h \
    src/Connection/VirtualConnection.h \
    src/Connection/Connection.h \
    src/Server/Devices/DevicePermissionManager.h \
    src/Server/Devices/DeviceService.h \
    src/Server/Devices/DeviceUpdateLogic.h \
    src/Server/Devices/IDevicePermissionController.h \
    src/Server/Resources/ListResource/ListResource.h \
    src/Server/Resources/ListResource/QObjectListResource.h \
    src/Server/Resources/ResourceManager/IResource.h \
    src/Server/Resources/ResourceManager/ResourceManager.h \
    src/Server/Resources/ObjectResource/ObjectResource.h \
    src/Server/Devices/DeviceManager.h \
    src/Server/Resources/ResourceManager/IResourceFactory.h \
    src/Server/Resources/ListResource/ListResourceFactory.h \
    src/Server/Resources/ObjectResource/ObjectResourceFactory.h \
    src/Server/Devices/IDevice.h \
    src/Server/Defines/ErrDef.h \
    src/Server/Devices/DeviceHandle.h \
    src/Connection/ISocket.h \
    src/Connection/IConnectable.h \
    src/Server/Settings/SettingsManager.h \
    src/Server/Settings/SettingsResource.h \
    src/Storage/FileSystemPaths.h \
    src/qhcore_global.h \
    src/QHCorePlugin.h \
    src/Server/Devices/DeviceProperty.h \
    src/Server/Resources/ListResource/IListResourceStorage.h \
    src/Storage/ListResourceFileSystemStorage.h \
    src/Server/Resources/ListResource/IListResourceStorageFactory.h \
    src/Server/Resources/ImageResource/ImageResource.h \
    src/Server/Resources/ImageResource/ImageResourceFactory.h \
    src/Server/Resources/ImageResource/IImageResourceStorage.h \
    src/Server/Resources/ImageResource/IImageResourceStorageFactory.h \
    src/Storage/ImageResourceFilesystemStorage.h \
    src/Server/Resources/ObjectResource/IObjectResourceStorage.h \
    src/Server/Resources/ObjectResource/IObjectResourceStorageFactory.h \
    src/Storage/ObjectResourceFilesystemStorage.h \
    src/Server/Logging/Logger.h \
    src/Server/Services/ServiceManager.h \
    src/Server/Services/IService.h \
    src/Storage/ListResourceTemporaryStorage.h \
    src/Server/Authentication/IAuthenticator.h \
    src/Server/Authentication/IUser.h \
    src/Server/Authentication/DefaultAuthenticator.h \
    src/Storage/FileSystemLoader.h

DISTFILES += \
    config.qdocconf \
    src/corePlugin.json

#INSTALLS += target
#target.path = /usr/lib
