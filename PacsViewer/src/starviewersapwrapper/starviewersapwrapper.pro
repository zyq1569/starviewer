QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

TEMPLATE = app

include(../applicationstargetnames.pri)
TARGET = $${TARGET_STARVIEWER_SAP_WRAPPER}

DESTDIR = ../../bin

macx {
    DESTDIR = $${DESTDIR}/$${TARGET_STARVIEWER}.app/Contents/MacOS
}

HEADERS = ../core/starviewerapplication.h \
    ../thirdparty/easylogging++/easylogging++.h

SOURCES = starviewersapwrapper.cpp \ 
    ../thirdparty/easylogging++/easylogging++.cc

INCLUDEPATH += ../core

include(../corelibsconfiguration.pri)
include(../compilationtype.pri)
#include(../log4cxx.pri)

QT += widgets
