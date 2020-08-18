TRANSLATIONS += $$EXTENSION_DIR/translations_ca_ES.ts \
                $$EXTENSION_DIR/translations_es_ES.ts \
                $$EXTENSION_DIR/translations_en_GB.ts

DESTDIR = $$OUT_PWD

INCLUDEPATH += ../../../core \
               ../../../inputoutput
MOC_DIR = ../../../../tmp/moc
UI_DIR = ../../../../tmp/ui
win32-msvc2013 {
    OBJECTS_DIR = ../../../../tmp/obj/$${TARGET}
    }
    else {
    OBJECTS_DIR = ../../../../tmp/obj
    }
RCC_DIR = ../../../../tmp/rcc
CONFIG += warn_on \
          qt \
          opengl \
          thread \
          x11 \
          staticlib \
          exceptions \
          stl
TEMPLATE = lib

include(../vtk.pri)
include(../itk.pri)
include(../gdcm.pri)
include(../dcmtk.pri)
include(../threadweaver.pri)
include(../compilationtype.pri)

QT += widgets
