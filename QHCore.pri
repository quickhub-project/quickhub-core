QT += core websockets concurrent

CONFIG += c++11
INCLUDEPATH += src
DEFINES += NO_PLUGIN
include(src/SocketApi/SocketApi.pri)

!contains(DEFINES, NO_GUI) {

QT += gui

SOURCES += \
    $$PWD/src/Server/Resources/ImageResource/ImageResource.cpp \
    $$PWD/src/Server/Resources/ImageResource/ImageResourceFactory.cpp \
    $$PWD/src/Storage/ImageResourceFilesystemStorage.cpp

HEADERS += \
    $$PWD/src/Server/Resources/ImageResource/ImageResource.h \
    $$PWD/src/Server/Resources/ImageResource/ImageResourceFactory.h \
    $$PWD/src/Server/Resources/ImageResource/IImageResourceStorage.h \
    $$PWD/src/Server/Resources/ImageResource/IImageResourceStorageFactory.h \
    $$PWD/src/Storage/ImageResourceFilesystemStorage.h
}

SOURCES += $$PWD/src/Server/Authentication/AuthentificationService.cpp \
    $$PWD/src/Server/Authentication/Controller.cpp \
    $$PWD/src/Server/Authentication/IIdentitiy.cpp \
    $$PWD/src/Server/Authentication/User.cpp \
    $$PWD/src/Connection/VirtualConnection.cpp \
    $$PWD/src/Connection/Connection.cpp \
    $$PWD/src/Server/Devices/DevicePermissionManager.cpp \
    $$PWD/src/Server/Devices/DeviceService.cpp \
    $$PWD/src/Server/Devices/DeviceUpdateLogic.cpp \
    $$PWD/src/Server/Devices/IDevicePermissionController.cpp \
    $$PWD/src/Server/Resources/ListResource/ListResource.cpp \
    $$PWD/src/Server/Resources/ListResource/QObjectListResource.cpp \
    $$PWD/src/Server/Resources/ObjectResource/QObjectResource.cpp \
    $$PWD/src/Server/Resources/ResourceManager/ResourceManager.cpp \
    $$PWD/src/Server/Resources/ObjectResource/ObjectResource.cpp \
    $$PWD/src/Server/Resources/ResourceManager/IResource.cpp \
    $$PWD/src/Server/Devices/DeviceManager.cpp \
    $$PWD/src/Server/Resources/ListResource/ListResourceFactory.cpp \
    $$PWD/src/Server/Resources/ObjectResource/ObjectResourceFactory.cpp \
    $$PWD/src/Server/Devices/IDevice.cpp \
    $$PWD/src/Server/Devices/DeviceHandle.cpp \
    $$PWD/src/Server/Devices/DeviceProperty.cpp \
    $$PWD/src/Server/Settings/SettingsManager.cpp \
    $$PWD/src/Server/Settings/SettingsResource.cpp \
    $$PWD/src/Storage/FileSystemPaths.cpp \
    $$PWD/src/Storage/ListResourceFileSystemStorage.cpp  \
    $$PWD/src/Storage/ObjectResourceFilesystemStorage.cpp \
    $$PWD/src/Server/Logging/Logger.cpp \
    $$PWD/src/Server/Services/ServiceManager.cpp \
    $$PWD/src/Storage/ListResourceTemporaryStorage.cpp \
    $$PWD/src/Server/Authentication/IUser.cpp \
    $$PWD/src/Server/Authentication/DefaultAuthenticator.cpp \
    $$PWD/src/Storage/FileSystemLoader.cpp 

HEADERS += \
    $$PWD/src/Server/Authentication/AuthentificationService.h \
    $$PWD/src/Server/Authentication/Controller.h \
    $$PWD/src/Server/Authentication/IIdentitiy.h \
    $$PWD/src/Server/Authentication/User.h \
    $$PWD/src/Connection/VirtualConnection.h \
    $$PWD/src/Connection/Connection.h \
    $$PWD/src/Server/Devices/DevicePermissionManager.h \
    $$PWD/src/Server/Devices/DeviceService.h \
    $$PWD/src/Server/Resources/ListResource/ListResourceFactory.h \
    $$PWD/src/Server/Devices/DeviceUpdateLogic.h \
    $$PWD/src/Server/Devices/IDevicePermissionController.h \
    $$PWD/src/Server/Resources/ListResource/ListResource.h \
    $$PWD/src/Server/Resources/ListResource/QObjectListResource.h \
    $$PWD/src/Server/Resources/ObjectResource/QObjectResource.h \
    $$PWD/src/Server/Resources/ResourceManager/IResource.h \
    $$PWD/src/Server/Resources/ResourceManager/ResourceManager.h \
    $$PWD/src/Server/Resources/ObjectResource/ObjectResource.h \
    $$PWD/src/Server/Devices/DeviceManager.h \
    $$PWD/src/Server/Resources/ResourceManager/IResourceFactory.h \
    $$PWD/src/Server/Resources/ObjectResource/ObjectResourceFactory.h \
    $$PWD/src/Server/Devices/IDevice.h \
    $$PWD/src/Server/Defines/ErrDef.h \
    $$PWD/src/Server/Devices/DeviceHandle.h \
    $$PWD/src/Connection/ISocket.h \
    $$PWD/src/Connection/IConnectable.h \
    $$PWD/src/Server/Settings/SettingsManager.h \
    $$PWD/src/Server/Settings/SettingsResource.h \
    $$PWD/src/Storage/FileSystemPaths.h \
    $$PWD/src/Server/Devices/DeviceProperty.h \
    $$PWD/src/Server/Resources/ListResource/IListResourceStorage.h \
    $$PWD/src/Storage/ListResourceFileSystemStorage.h \
    $$PWD/src/Server/Resources/ListResource/IListResourceStorageFactory.h \
    $$PWD/src/Server/Resources/ObjectResource/IObjectResourceStorage.h \
    $$PWD/src/Server/Resources/ObjectResource/IObjectResourceStorageFactory.h \
    $$PWD/src/Storage/ObjectResourceFilesystemStorage.h \
    $$PWD/src/Server/Logging/Logger.h \
    $$PWD/src/Server/Services/ServiceManager.h \
    $$PWD/src/Server/Services/IService.h \
    $$PWD/src/Storage/ListResourceTemporaryStorage.h \
    $$PWD/src/Server/Authentication/IAuthenticator.h \
    $$PWD/src/Server/Authentication/IUser.h \
    $$PWD/src/Server/Authentication/DefaultAuthenticator.h \
    $$PWD/src/Storage/FileSystemLoader.h