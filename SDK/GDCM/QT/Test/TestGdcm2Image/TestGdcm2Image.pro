QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
# Avoid definition of min and max macros by windows.h
DEFINES += NOMINMAX
# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_CXXFLAGS_DEBUG -= -MDd
QMAKE_CXXFLAGS_DEBUG += -MD

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default install prefix is /usr/local
SDK_INSTALL_PREFIX = D:\SDK\starviewer-sdk-win64-0.15
isEmpty(SDK_INSTALL_PREFIX){
    unix:SDK_INSTALL_PREFIX = $$(HOME)/starviewer-sdk-0.15/usr/local
    win32:SDK_INSTALL_PREFIX = $$(USERPROFILE)/starviewer-sdk-0.15/32
    win32:contains(QMAKE_TARGET.arch, x86_64):SDK_INSTALL_PREFIX = $$(USERPROFILE)/starviewer-sdk-0.15/64
}

GDCMLIBDIR = $$(GDCMLIBDIR)
isEmpty(GDCMLIBDIR){
    unix:GDCMLIBDIR = $$SDK_INSTALL_PREFIX/lib
    win32:GDCMLIBDIR = $$SDK_INSTALL_PREFIX/gdcm/3.0.21/lib
}
GDCMINCLUDEDIR = $$(GDCMINCLUDEDIR)
isEmpty(GDCMINCLUDEDIR){
    unix:GDCMINCLUDEDIR = $$SDK_INSTALL_PREFIX/include/gdcm-3.0
    win32:GDCMINCLUDEDIR = $$SDK_INSTALL_PREFIX/gdcm/3.0.21/include/gdcm-3.0
}

# DCMTK Libraries

DCMTKLIBDIR = $$(DCMTKLIBDIR)
isEmpty(DCMTKLIBDIR){
    unix:DCMTKLIBDIR = $$SDK_INSTALL_PREFIX/lib
#    win32:DCMTKLIBDIR = $$SDK_INSTALL_PREFIX/dcmtk/3.6.1_20120515/lib
#    win32:DCMTKLIBDIR = $$SDK_INSTALL_PREFIX/dcmtk/3.6.3/lib
     win32:DCMTKLIBDIR = $$SDK_INSTALL_PREFIX/dcmtk/3.6.5/lib
}
DCMTKINCLUDEDIR = $$(DCMTKINCLUDEDIR)
isEmpty(DCMTKINCLUDEDIR){
    unix:DCMTKINCLUDEDIR = $$SDK_INSTALL_PREFIX/include/dcmtk
#    win32:DCMTKINCLUDEDIR = $$SDK_INSTALL_PREFIX/dcmtk/3.6.1_20120515/include/dcmtk
#    win32:DCMTKINCLUDEDIR = $$SDK_INSTALL_PREFIX/dcmtk/3.6.3/include/dcmtk
     win32:DCMTKINCLUDEDIR = $$SDK_INSTALL_PREFIX/dcmtk/3.6.5/include/dcmtk
}
INCLUDEPATH +=  $${DCMTKINCLUDEDIR} \
                $${DCMTKINCLUDEDIR}/../ \
                $${DCMTKINCLUDEDIR}/dcmdata/ \
                $${DCMTKINCLUDEDIR}/dcmimgle/ \
                $${DCMTKINCLUDEDIR}/dcmnet/ \
                $${DCMTKINCLUDEDIR}/dcmsign/ \
                $${DCMTKINCLUDEDIR}/ofstd/ \
                $${DCMTKINCLUDEDIR}/oflog/ \
                $${DCMTKINCLUDEDIR}/dcmimage/ \
                $${DCMTKINCLUDEDIR}/config \
                $${DCMTKINCLUDEDIR}/dcmpstat \
                $${DCMTKINCLUDEDIR}/dcmsr \
                $${DCMTKINCLUDEDIR}/dcmjpeg

LIBS += -L$${DCMTKLIBDIR} \
        -ldcmpstat \
        -ldcmnet \
        -ldcmdata \
        -ldcmimgle \
        -ldcmimage \
        -ldcmsr \
        -ldcmqrdb \
        -ldcmtls \
        -ldcmdsig \
        -ldcmjpeg \
        -lijg8 \
        -lijg12 \
        -lijg16 \
        -loflog \
        -lofstd


INCLUDEPATH += $${GDCMINCLUDEDIR}
LIBS += -L$${GDCMLIBDIR} \
        -lgdcmcharls \
        -lgdcmCommon \
        -lgdcmDICT \
        -lgdcmDSED \
        -lgdcmexpat \
        -lgdcmIOD \
        -lgdcmjpeg8 \
        -lgdcmjpeg12 \
        -lgdcmjpeg16 \
        -lgdcmMSFF \
        -lvtkgdcm

LIBS += -lws2_32 #dcmtk >= 3.6.3 need
LIBS += -lNetAPI32 -lWSock32 -lSnmpAPI
LIBS += -ladvapi32 \
         -lRpcrt4 \
         -lwbemuuid \
         -lIphlpapi


win32:LIBS += -lShell32
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
