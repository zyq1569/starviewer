CONFIG += debug_and_release c++14

# Use gold linker
linux:QMAKE_LFLAGS += -fuse-ld=gold

unix {
    QMAKE_CXXFLAGS_RELEASE += -Wno-deprecated
    QMAKE_CXXFLAGS_DEBUG += -Wno-deprecated

    !macx:LIBS += -lGLU
}

win32 {
#    DEFINES  -= UNICODE
    # To have glew built into the code. Only needed in windows.
    DEFINES += GLEW_STATIC
    # Avoid definition of min and max macros by windows.h
    DEFINES += NOMINMAX
    
    # Volem veure missatges a std::cout en debug
    CONFIG(debug, debug|release) {
        CONFIG += console
    }
    else {
        CONFIG -= console
    }

    LIBS += -lws2_32 #dcmtk >= 3.6.3 need
    # Required since Qt 5.6 to use OpenGl directly
    LIBS += opengl32.lib glu32.lib
    # Required since Qt 5.6 by ITKCommon
    LIBS += gdi32.lib

    # we define that for visual studio-based windows compilation systems
    # compilation is done in as many cores as possible
    QMAKE_CXXFLAGS += /MP

    # We indicate that for debug compilations, Runtime Library
    # is Multi-threaded DLL (as in release) and not Multi-threaded Debug DLL
    QMAKE_CXXFLAGS_DEBUG -= -MDd
    QMAKE_CXXFLAGS_DEBUG += -MD

    QMAKE_CXXFLAGS -= -Zc:strictStrings
}

macx {
    QMAKE_CXXFLAGS += -stdlib=libc++
    QMAKE_LFLAGS += -stdlib=libc++
    LIBS += -framework Cocoa
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.11
}

# We define variable to build lite version at compile time
lite_version:DEFINES += STARVIEWER_LITE

# EasyLogging++ is set to be thread safe, to not have a default log file, and to not handle crashes
DEFINES += ELPP_THREAD_SAFE ELPP_NO_DEFAULT_LOG_FILE ELPP_DISABLE_DEFAULT_CRASH_HANDLING
