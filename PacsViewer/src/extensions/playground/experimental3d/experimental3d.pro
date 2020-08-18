# Fitxer generat pel gestor de qmake de kdevelop.
# -------------------------------------------
# Subdirectori relatiu al directori principal del projecte: ./src/extensions/playground/experimental3d
# L'objectiu s una aplicaci??:
FORMS += qexperimental3dextensionbase.ui \
    qviewpointdistributionwidgetbase.ui \
    qgraphictransferfunctioneditorbase.ui
HEADERS += experimental3dextensionmediator.h \
    qexperimental3dextension.h \
    experimental3dsettings.h \
    qexperimental3dviewer.h \
    experimental3dvolume.h \
    qviewpointdistributionwidget.h \
    colorbleedingvoxelshader.h \
    volumereslicer.h \
    histogram.h \
    informationtheory.h \
    qbasicgraphictransferfunctioneditor.h \
    qgraphictransferfunctioneditor.h \
    vmivoxelshader2.h \
    voxelsaliencyvoxelshader.h \
    vomivoxelshader.h \
    colorvomivoxelshader.h \
    opacityvoxelshader.h \
    celshadingvoxelshader.h \
    vomicoolwarmvoxelshader.h \
    coolwarmvoxelshader.h \
    viewpointinformationchannel.h \
    filteringambientocclusionvoxelshader.h \
    filteringambientocclusionmapvoxelshader.h \
    vomigammavoxelshader.h \
    imivoxelshader.h \
    filteringambientocclusionstipplingvoxelshader.h \
    whitevoxelshader.h \
    optimizetransferfunctioncommand.h \
    ambientvoxelshader2.h \
    directilluminationvoxelshader2.h \
    camera.h \
    matrix3.h \
    matrix4.h \
    quaternion.h \
    obscurance.h \
    obscurancemainthread.h \
    obscurancethread.h \
    obscurancevoxelshader.h \
    vtkVolumeRayCastVoxelShaderCompositeFunction.h \
    vtkVolumeRayCastMapper.h \
    vtkVolumeRayCastFunction.h \
    vtkVolumeRayCastCompositeFunction.h

SOURCES += experimental3dextensionmediator.cpp \
    qexperimental3dextension.cpp \
    experimental3dsettings.cpp \
    qexperimental3dviewer.cpp \
    experimental3dvolume.cpp \
    qviewpointdistributionwidget.cpp \
    colorbleedingvoxelshader.cpp \
    volumereslicer.cpp \
    histogram.cpp \
    informationtheory.cpp \
    qbasicgraphictransferfunctioneditor.cpp \
    qgraphictransferfunctioneditor.cpp \
    vmivoxelshader2.cpp \
    voxelsaliencyvoxelshader.cpp \
    vomivoxelshader.cpp \
    colorvomivoxelshader.cpp \
    opacityvoxelshader.cpp \
    celshadingvoxelshader.cpp \
    vomicoolwarmvoxelshader.cpp \
    coolwarmvoxelshader.cpp \
    viewpointinformationchannel.cpp \
    filteringambientocclusionvoxelshader.cpp \
    filteringambientocclusionmapvoxelshader.cpp \
    vomigammavoxelshader.cpp \
    imivoxelshader.cpp \
    filteringambientocclusionstipplingvoxelshader.cpp \
    whitevoxelshader.cpp \
    optimizetransferfunctioncommand.cpp \
    ambientvoxelshader2.cpp \
    directilluminationvoxelshader2.cpp \
    camera.cpp \
    matrix3.cpp \
    matrix4.cpp \
    quaternion.cpp \
    obscurance.cpp \
    obscurancemainthread.cpp \
    obscurancethread.cpp \
    obscurancevoxelshader.cpp \
    vtkVolumeRayCastVoxelShaderCompositeFunction.cxx \
    vtkVolumeRayCastMapper.cxx \
    vtkVolumeRayCastFunction.cxx \
    vtkVolumeRayCastCompositeFunction.cxx

RESOURCES += experimental3d.qrc
EXTENSION_DIR = $$PWD
cuda { 
    HEADERS += cudaviewpointinformationchannel.h \
        qcudarenderwindow.h \
        cudafiltering.h \
        viewpointintensityinformationchannel.h \
        cudaviewpointintensityinformationchannel.h
    SOURCES += qcudarenderwindow.cpp \
        viewpointintensityinformationchannel.cpp
    CUSOURCES += cudaviewpointinformationchannel.cu \
        cudafiltering.cu \
        cudaviewpointintensityinformationchannel.cu
    OTHER_FILES += cudaviewpointinformationchannel.cu \
        cudafiltering.cu \
        cudaviewpointintensityinformationchannel.cu
}
include(../../basicconfextensions.pri)
include(../../../cuda.pri)
include(../../../thirdparty/glew/glew.pri)
QT += opengl concurrent
