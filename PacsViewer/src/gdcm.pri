include(defaultdirectories.pri)

#INCLUDEPATH += $${GDCMINCLUDEDIR}
#3.0.0
INCLUDEPATH +=  $${GDCMINCLUDEDIR} \
                $${GDCMINCLUDEDIR}/Source/Common/ \
                $${GDCMINCLUDEDIR}/Source/MediaStorageAndFileFormat/ \
                $${GDCMINCLUDEDIR}/Source/DataStructureAndEncodingDefinition/ \
                $${GDCMINCLUDEDIR}/Source/DataDictionary/ \
                $${GDCMINCLUDEDIR}/Source/InformationObjectDefinition/ \
                $${GDCMINCLUDEDIR}/Source/Attribute/ \
                $${GDCMINCLUDEDIR}/Source/Common/ \
                $${GDCMINCLUDEDIR}/Source/MessageExchangeDefinition/ \
                $${GDCMINCLUDEDIR}/more/ \
                $${GDCMINCLUDEDIR}/Utilities/ \
                $${GDCMINCLUDEDIR}/Utilities/gdcmcharls/ \
                $${GDCMINCLUDEDIR}/Utilities/gdcmexpat/ \
                $${GDCMINCLUDEDIR}/Utilities/gdcmjpeg/ \
                $${GDCMINCLUDEDIR}/Utilities/gdcmmd5/ \
                $${GDCMINCLUDEDIR}/Utilities/gdcmopenjpeg/ \
                $${GDCMINCLUDEDIR}/Utilities/gdcmrle/ \
                $${GDCMINCLUDEDIR}/Utilities/gdcmutfcpp/ \
                $${GDCMINCLUDEDIR}/Utilities/gdcmuuid/ \
                $${GDCMINCLUDEDIR}/Utilities/gdcmzlib/ \
                $${GDCMINCLUDEDIR}/Utilities/getopt/ \
                $${GDCMINCLUDEDIR}/Utilities/KWStyle/ \
                $${GDCMINCLUDEDIR}/Utilities/pvrg/ \
                $${GDCMINCLUDEDIR}/Utilities/VTK/ \
                $${GDCMINCLUDEDIR}/Utilities/socketxx

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
-lgdcmopenjp2 \
        -lvtkgdcm

win32 {
    LIBS += -lgdcmgetopt -lgdcmzlib
}
