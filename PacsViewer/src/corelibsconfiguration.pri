# Configuraci� gen�rica de core, interfaces, inputoutput i main

CONFIG += warn_on \
          thread \
          qt \
          stl
          
win32-msvc2013 {
    OBJECTS_DIR = ../../QTtmp/obj/$${TARGET}
    }
    else {
    OBJECTS_DIR = ../../QTtmp/obj
    }

UI_DIR  = ../../QTtmp/ui
MOC_DIR = ../../QTtmp/moc
RCC_DIR = ../../QTtmp/rcc

include(makefixdebug.pri)
