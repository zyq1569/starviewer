include(addlibrarydependency.pri)
include($$PWD/src/extensions.pri)

for(dir, PLAYGROUND_EXTENSIONS) {
#    addLibraryDependency($$PWD/src/extensions/playground, $$OUT_PWD/../../src/extensions/playground, $$dir)
addLibraryDependency($$PWD/src/extensions/playground, $$OUT_PWD/../../bin, $$dir)
}

for(dir, CONTRIB_EXTENSIONS) {
#    addLibraryDependency($$PWD/src/extensions/contrib, $$OUT_PWD/../../src/extensions/contrib, $$dir)
addLibraryDependency($$PWD/src/extensions/contrib, $$OUT_PWD/../../bin, $$dir)
}

for(dir, MAIN_EXTENSIONS) {
#    addLibraryDependency($$PWD/src/extensions/main, $$OUT_PWD/../../src/extensions/main, $$dir)
addLibraryDependency($$PWD/src/extensions/main, $$OUT_PWD/../../bin, $$dir)
}

#addLibraryDependency($$PWD/src/thirdparty, $$OUT_PWD/../../src/thirdparty, easylogging++)
addLibraryDependency($$PWD/src/thirdparty, $$OUT_PWD/../../bin, easylogging++)
# Dependències de llibreries core
#addLibraryDependency($$PWD/src, $$OUT_PWD/../../src, interface)
addLibraryDependency($$PWD/src, $$OUT_PWD/../../bin, interface)
linux:LIBS += -Wl,--start-group
#addLibraryDependency($$PWD/src, $$OUT_PWD/../../src, inputoutput)
addLibraryDependency($$PWD/src, $$OUT_PWD/../../bin, inputoutput)
#addLibraryDependency($$PWD/src, $$OUT_PWD/../../src, core)
addLibraryDependency($$PWD/src, $$OUT_PWD/../../bin, core)
linux:LIBS += -Wl,--end-group

win32{
  LIBS += -ladvapi32 \
          -lRpcrt4 \
          -lwbemuuid \
          -lIphlpapi
}

include($$PWD/src/dcmtk.pri)
include($$PWD/src/vtk.pri)
include($$PWD/src/itk.pri)
include($$PWD/src/gdcm.pri)
include($$PWD/src/cuda.pri)
include($$PWD/src/compilationtype.pri)
include($$PWD/src/threadweaver.pri)
