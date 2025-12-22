include(defaultdirectories.pri)

INCLUDEPATH +=  $${DCMTKINCLUDEDIR} \
                $${DCMTKINCLUDEDIR}/../ \
                $${DCMTKINCLUDEDIR}/dcmdata/include/ \
                $${DCMTKINCLUDEDIR}/dcmimgle/include/ \
                $${DCMTKINCLUDEDIR}/dcmnet/include/ \
                $${DCMTKINCLUDEDIR}/dcmsign/include/ \
                $${DCMTKINCLUDEDIR}/ofstd/include/ \
                $${DCMTKINCLUDEDIR}/oflog/include/ \
                $${DCMTKINCLUDEDIR}/dcmimage/include/ \
                $${DCMTKINCLUDEDIR}/config/include/  \
                $${DCMTKINCLUDEDIR}/dcmpstat/include/ \
                $${DCMTKINCLUDEDIR}/dcmsr/include/ \
                $${DCMTKINCLUDEDIR}/dcmjpeg/include/ \
                $${DCMTKINCLUDEDIR}/dcmjpls/include/ \
                $${DCMTKINCLUDEDIR}/dcmqrdb/include/ \
                $${DCMTKINCLUDEDIR}/dcmdata/include/dcmtk/dcmdata/ \
                $${DCMTKINCLUDEDIR}/dcmimgle/include/dcmtk/dcmimgle/ \
                $${DCMTKINCLUDEDIR}/dcmnet/include/dcmtk/dcmnet/ \
                $${DCMTKINCLUDEDIR}/dcmsign/include/dcmtk/dcmsign/ \
                $${DCMTKINCLUDEDIR}/ofstd/include/dcmtk/ofstd \
                $${DCMTKINCLUDEDIR}/oflog/include/dcmtk/oflog/ \
                $${DCMTKINCLUDEDIR}/dcmimage/include/dcmtk/dcmimage/ \
                $${DCMTKINCLUDEDIR}/config/include/dcmtk/config/ \
                $${DCMTKINCLUDEDIR}/dcmpstat/include/dcmtk/dcmpstat/ \
                $${DCMTKINCLUDEDIR}/dcmsr/include/dcmtk/dcmsr/ \
                $${DCMTKINCLUDEDIR}/dcmjpeg/include/dcmtk/dcmjpeg/ \
                $${DCMTKINCLUDEDIR}/dcmdata/include/dcmtk/ \
                $${DCMTKINCLUDEDIR}/dcmimgle/include/dcmtk/ \
                $${DCMTKINCLUDEDIR}/dcmnet/include/dcmtk/ \
                $${DCMTKINCLUDEDIR}/dcmsign/include/dcmtk/ \
                $${DCMTKINCLUDEDIR}/ofstd/include/dcmtk/\
                $${DCMTKINCLUDEDIR}/oflog/include/dcmtk/ \
                $${DCMTKINCLUDEDIR}/dcmimage/include/dcmtk/ \
                $${DCMTKINCLUDEDIR}/config/include/dcmtk/ \
                $${DCMTKINCLUDEDIR}/dcmpstat/include/dcmtk/ \
                $${DCMTKINCLUDEDIR}/dcmsr/include/dcmtk/ \
                $${DCMTKINCLUDEDIR}/dcmjpeg/include/dcmtk/ \
                $${DCMTKINCLUDEDIR}/dcmjpls/include/dcmtk/


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
        -ldcmjpls \
        -lijg8 \
        -lijg12 \
        -lijg16 \
        -loflog \
        -lofstd

unix {
        DEFINES += HAVE_CONFIG_H
        LIBS += -lz   -liconv
}

unix:!macx {
        LIBS += -lwrap -lssl
}

win32 {
        LIBS += -lNetAPI32 -lWSock32 -lSnmpAPI
        LIBS -= -lxml2
}
