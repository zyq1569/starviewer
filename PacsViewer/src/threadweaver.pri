include(defaultdirectories.pri)

LIBS += -L$${THREADWEAVERLIBDIR} -lKF5ThreadWeaver_static

INCLUDEPATH += $${THREADWEAVERINCLUDEDIR}
