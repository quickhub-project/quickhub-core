
!contains(DEFINES, NO_GUI) {
    SOURCES += \
        $$PWD/ResourceHandler/ImageCollection/ImageCollectionHandler.cpp \
        $$PWD/ResourceHandler/ImageCollection/ImageCollectionHandlerFactory.cpp

    HEADERS += \
        $$PWD/ResourceHandler/ImageCollection/ImageCollectionHandler.h \
        $$PWD/ResourceHandler/ImageCollection/ImageCollectionHandlerFactory.h \

}

SOURCES += $$PWD/Devices/SocketDeviceHandler.cpp \
           $$PWD/ResourceHandler/SynchronizedList/SynchronizedListHandler.cpp \
           $$PWD/ResourceHandler/SynchronizedList/SynchronizedListHandlerFactory.cpp \
           $$PWD/ResourceHandler/SynchronizedObject/SynchronizedObjectHandler.cpp \
           $$PWD/ResourceHandler/SynchronizedObject/SynchronizedObjectHandlerFactory.cpp \
           $$PWD/DataHandler/Lists/IList.cpp \
           $$PWD/DataHandler/Lists/ListWrapper/DeviceListWrapper.cpp \
           $$PWD/SocketCore/SocketResourceManager.cpp \
           $$PWD/SocketServer.cpp \
           $$PWD/Session/SessionHandler.cpp \
           $$PWD/SocketCore/IResourceHandler.cpp \
           $$PWD/DataHandler/Lists/ListHandler.cpp \
           $$PWD/DataHandler/Lists/ListHandlerFactory.cpp \
           $$PWD/DataHandler/Lists/ListWrapper/UserListWrapper.cpp \
           $$PWD/DataHandler/Lists/ListWrapper/DeviceHandleListWrapper.cpp \
           $$PWD/Devices/DeviceHandleHandler.cpp \
           $$PWD/Devices/DeviceHandleHandlerFactory.cpp \
           $$PWD/Devices/SocketDevice.cpp \
           $$PWD/Services/ServiceRequestHandler.cpp

HEADERS +=  $$PWD/SocketCore/SocketResourceManager.h \
            $$PWD/SocketServer.h \
            $$PWD/Session/SessionHandler.h \
            $$PWD/SocketCore/IRequestHandler.h \
            $$PWD/SocketCore/IResourceHandler.h \
            $$PWD/SocketCore/IResourceHandlerFactory.h \
            $$PWD/Devices/SocketDeviceHandler.h \
            $$PWD/ResourceHandler/SynchronizedList/SynchronizedListHandler.h \
            $$PWD/ResourceHandler/SynchronizedList/SynchronizedListHandlerFactory.h \
            $$PWD/ResourceHandler/SynchronizedObject/SynchronizedObjectHandler.h \
            $$PWD/ResourceHandler/SynchronizedObject/SynchronizedObjectHandlerFactory.h \
            $$PWD/DataHandler/Lists/IList.h \
            $$PWD/DataHandler/Lists/ListHandler.h \
            $$PWD/DataHandler/Lists/ListWrapper/DeviceListWrapper.h \
            $$PWD/DataHandler/Lists/ListHandlerFactory.h \
            $$PWD/DataHandler/Lists/ListWrapper/UserListWrapper.h \
            $$PWD/DataHandler/Lists/ListWrapper/DeviceHandleListWrapper.h \
            $$PWD/Devices/DeviceHandleHandler.h \
            $$PWD/Devices/DeviceHandleHandlerFactory.h \
            $$PWD/Devices/SocketDevice.h \
            $$PWD/ResourceHandler/Services/ServiceHandlerFactory.h \
            $$PWD/Services/ServiceRequestHandler.h


INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/..

