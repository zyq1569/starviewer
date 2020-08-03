# Fitxer generat pel gestor de qmake de kdevelop. 
# ------------------------------------------- 
# Subdirectori relatiu al directori principal del projecte: .
# L'objectiu és un subdirectori del projecte 

include(src/compilationtype.pri)

QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

SUBDIRS += src
!official_release:SUBDIRS += tests
TEMPLATE = subdirs
CONFIG += warn_on \
          qt \
          thread \
          ordered
