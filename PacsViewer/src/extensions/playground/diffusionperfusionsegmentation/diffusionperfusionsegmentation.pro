# Fitxer generat pel gestor de qmake de kdevelop. 
# ------------------------------------------- 
# Subdirectori relatiu al directori principal del projecte: ./src/extensions/diffusionperfusionsegmentation
# L'objectiu és una biblioteca:  

FORMS += qdifuperfuextensionbase.ui 
HEADERS += qdifuperfuextension.h \
           diffusionperfusionsegmentationsettings.h \
           itkRegistre3DAffine.h \
           udgPerfusionEstimator.h \
           udgBinaryMaker.h \
           diffusionperfusionsegmentationextensionmediator.h \
           volumecalculator.h
SOURCES += qdifuperfuextension.cpp \
           diffusionperfusionsegmentationsettings.cpp \
           itkRegistre3DAffine.cpp \
           udgPerfusionEstimator.cpp \
           udgBinaryMaker.cpp \
           diffusionperfusionsegmentationextensionmediator.cpp \
           volumecalculator.cpp

RESOURCES += diffusionperfusionsegmentation.qrc

EXTENSION_DIR = $$PWD
include(../../basicconfextensions.pri)
