QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

include(../../compilationtype.pri)

QT -= core gui

TEMPLATE = lib
CONFIG += staticlib
DESTDIR = ./

HEADERS += easylogging++.h
SOURCES += easylogging++.cc
