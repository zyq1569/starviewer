#Afegim els projectes de tercers que s'han de compilar que utilitza Starviewer.

QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

TEMPLATE = subdirs 
SUBDIRS = qtsingleapplication breakpad easylogging++

include(../compilationtype.pri)
