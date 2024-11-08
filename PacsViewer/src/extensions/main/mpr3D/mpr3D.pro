# Fitxer generat pel gestor de qmake de kdevelop. 
# ------------------------------------------- 
# Subdirectori relatiu al directori principal del projecte: ./src/extensions/playground/optimalviewpoint
# L'objectiu � una biblioteca:  

FORMS += qmpr3Dextensionbase.ui

HEADERS += qmpr3Dextension.h \
           mpr3Dsettings.h \
           mpr3Dextensionmediator.h

SOURCES += qmpr3Dextension.cpp \
           mpr3Dsettings.cpp \
           mpr3Dextensionmediator.cpp

RESOURCES += mpr3D.qrc

EXTENSION_DIR = $$PWD
include(../../basicconfextensions.pri)

DESTDIR = ../../../../bin
