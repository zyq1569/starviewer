# We add extensions dependencies

include(../applicationstargetnames.pri)

TARGET = $${TARGET_STARVIEWER}
DESTDIR = ../../bin
TEMPLATE = app

# CrashHandler
SOURCES += crashhandler.cpp \
            ../thirdparty/qtsingleapplication/src/qtlocalpeer.cpp \
            ../thirdparty/qtsingleapplication/src/qtsingleapplication.cpp
HEADERS += crashhandler.h \
            ../thirdparty/qtsingleapplication/src/qtlocalpeer.h \
            ../thirdparty/qtsingleapplication/src/qtsingleapplication.h

# End CrashHandler

SOURCES += main.cpp \
           syncactionsregister.cpp \
           applicationtranslationsloader.cpp
HEADERS += applicationtranslationsloader.h \
           syncactionsregister.h \
           diagnosistests.h \
           vtkinit.h
TRANSLATIONS += main_ca_ES.ts \
                main_es_ES.ts \
                main_en_GB.ts

RESOURCES = main.qrc ../qml/qml.qrc

official_release {
    win32:RESOURCES += qtconf/win/qtconf.qrc
    #macx:RESOURCES += qtconf/mac/qtconf.qrc    # For future use
    #linux:RESOURCES += qtconf/linux/qtconf.qrc # For future use
}

win32{
RC_FILE = starviewer.rc
}
macx {
    ICON = images/logo/logo.icns
}

# We define that for visual studio-based windows compilation systems
# activate the flag / LARGEADDRESSAWARE, which allows them to be used
# more than 2Gb of memory per process. This will be effective on 64-bit systems
win32-msvc2013:QMAKE_LFLAGS += /LARGEADDRESSAWARE

include(../../sourcelibsdependencies.pri)

# Thirdparty libraries
#addLibraryDependency($$PWD/../thirdparty, $$OUT_PWD/../thirdparty, breakpad)
addLibraryDependency($$PWD/../thirdparty,$$OUT_PWD/../../bin, breakpad)
# --------------------include  lib
#outputDirectory = ../../bin
#LIBS += -L$${outputDirectory} \
#        -breakpad

INCLUDEPATH += $$PWD/../thirdparty/qtsingleapplication/src

# --------------------include  lib
include(../corelibsconfiguration.pri)
#include(../thirdparty/qtsingleapplication/src/qtsingleapplication.pri)

include(installextensions.pri)

QT += xml opengl network xmlpatterns qml concurrent quick quickwidgets sql webenginewidgets

#TODO: Qt 4.5.3 does not add UI_DIR information as include when compiling main.cpp
INCLUDEPATH += ../../QTtmp/ui
