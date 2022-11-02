QT += core websockets concurrent


CONFIG += c++11
TARGET = QHCore
CONFIG += plugin
INCLUDEPATH += ../quickhub-pluginsystem/src
INCLUDEPATH += src
LIBS += -lQHPluginSystem -L../bin/lib
DESTDIR = ../bin/plugins
TEMPLATE = lib
DEFINES += COREPLUGIN_LIBRARY
DEFINES += NO_GUI

include(QHCore.pri)


SOURCES += \
    src/QHCorePlugin.cpp

HEADERS += \
    src/qhcore_global.h \
    src/QHCorePlugin.h


DISTFILES += \
    config.qdocconf \
    src/corePlugin.json

#INSTALLS += target
#target.path = /usr/lib
