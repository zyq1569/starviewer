# Afegim dependències de les extensions
QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

include(../applicationstargetnames.pri)

TARGET = $${TARGET_STARVIEWER}
DESTDIR = ../../bin
TEMPLATE = app

# CrashHandler
SOURCES += crashhandler.cpp
HEADERS += crashhandler.h

# End CrashHandler

SOURCES += main.cpp \
           syncactionsregister.cpp \
           applicationtranslationsloader.cpp
HEADERS += applicationtranslationsloader.h \
           syncactionsregister.h \
           diagnosistests.h \
           vtkinit.h
RESOURCES = main.qrc ../qml/qml.qrc

win32{
RC_FILE = starviewer.rc
}
macx {
    ICON = images/starviewer.icns
}

# We define that for visual studio-based windows compilation systems
# activate the flag / LARGEADDRESSAWARE, which allows them to be used
# more than 2Gb of memory per process. This will be effective on 64-bit systems
win32-msvc2013:QMAKE_LFLAGS += /LARGEADDRESSAWARE

include(../../sourcelibsdependencies.pri)

# Thirdparty libraries
DUMMY = $$addLibraryDependency(../thirdparty, breakpad)

include(../corelibsconfiguration.pri)
include(../thirdparty/qtsingleapplication/src/qtsingleapplication.pri)
include(../breakpad.pri)

include(installextensions.pri)

QT += xml opengl network webkit xmlpatterns qml declarative concurrent webkitwidgets

#TODO: Qt 4.5.3 no afegeix la informacio de UI_DIR com a include a l'hora de compilar el main.cpp
INCLUDEPATH += ../../tmp/ui
