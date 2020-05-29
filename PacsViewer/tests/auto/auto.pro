
TARGET = autotests
DESTDIR = ./
TEMPLATE = app

CONFIG -= app_bundle

INCLUDEPATH += shared
DEPENDPATH += shared

SOURCES += autotests.cpp

QT += testlib

QT += xml opengl network webkit xmlpatterns gui declarative concurrent webkitwidgets qml

OBJECTS_DIR = ../../tmp/obj
UI_DIR = ../../tmp/ui
MOC_DIR = ../../tmp/moc
RCC_DIR = ../../tmp/rcc

include(shared/shared.pri)
include(unittests/unittests.pri)
include(../../sourcelibsdependencies.pri)
include(../../src/makefixdebug.pri)

#TODO: No afegeix la informacio de UI_DIR com a include a l'hora de compilar el testos
INCLUDEPATH += ../../tmp/ui

RESOURCES = ../../src/main/main.qrc
