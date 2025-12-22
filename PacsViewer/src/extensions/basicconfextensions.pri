TRANSLATIONS += $$EXTENSION_DIR/translations_ca_ES.ts \
                $$EXTENSION_DIR/translations_es_ES.ts \
                $$EXTENSION_DIR/translations_en_GB.ts

DESTDIR = $$OUT_PWD

INCLUDEPATH += ../../../core \
               ../../../inputoutput
MOC_DIR = ../../../../QTtmp/moc
UI_DIR = ../../../../QTtmp/ui
win32-msvc2013 {
    OBJECTS_DIR = ../../../../QTtmp/obj/$${TARGET}
    }
    else {
    OBJECTS_DIR = ../../../../QTtmp/obj
    }
RCC_DIR = ../../../../QTtmp/rcc
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
