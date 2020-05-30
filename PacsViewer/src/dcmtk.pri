include(defaultdirectories.pri)

INCLUDEPATH += $${DCMTKINCLUDEDIR} \
    $${DCMTKINCLUDEDIR}/../ \
    $${DCMTKINCLUDEDIR}/dcmdata/ \
    $${DCMTKINCLUDEDIR}/dcmimgle/ \
    $${DCMTKINCLUDEDIR}/dcmnet/ \
    $${DCMTKINCLUDEDIR}/dcmsign/ \
    $${DCMTKINCLUDEDIR}/ofstd/ \
    $${DCMTKINCLUDEDIR}/oflog/ \
    $${DCMTKINCLUDEDIR}/dcmimage/ \
    $${DCMTKINCLUDEDIR}/config/ \
    $${DCMTKINCLUDEDIR}/dcmpstat/ \
    $${DCMTKINCLUDEDIR}/dcmsr/ \
    $${DCMTKINCLUDEDIR}/dcmjpeg

LIBS += -L$${DCMTKLIBDIR} \
    -lsqlite3 \
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
LIBS += -lShell32 \
    -lGlu32 \
    -ladvapi32 \
    -lwbemuuid \
    -lIphlpapi \
    -lopengl32 \
    -lNetAPI32 \
    -lSnmpAPI \
    -lwsock32 \
    -lrpcrt4 \
    -luuid \
    -lWS2_32 \
    -lMSWSOCK \
    -lODBC32 \
    -lGdi32 \
    -lQt5Core

unix {
    DEFINES += HAVE_CONFIG_H
    LIBS += -lssl
}

unix:!macx {
    LIBS += -lwrap
}

win32 {
    LIBS += -lNetAPI32 -lWSock32 -lSnmpAPI

    contains(QMAKE_TARGET.arch, x86_64) {
        INCLUDEPATH += D:/SDK/sqlite3-64
        LIBS += -LD:/SDK/sqlite3-64
    }
    else {
        INCLUDEPATH +=D:/SDK/sqlite3-32
        LIBS += -LD:/SDK/sqlite3-32
    }
    LIBS -= -lxml2
}

