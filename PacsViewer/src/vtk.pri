include(defaultdirectories.pri)

#INCLUDEPATH += $${VTKINCLUDEDIR}

#8.2.0
INCLUDEPATH +=  $${VTKINCLUDEDIR} \
                $${VTKINCLUDEDIR}/more/ \
                $${VTKINCLUDEDIR}/Common/Core/ \
                $${VTKINCLUDEDIR}/Common/DataModel/ \
                $${VTKINCLUDEDIR}/Common/Math/ \
                $${VTKINCLUDEDIR}/Common/ComputationalGeometry/ \
                $${VTKINCLUDEDIR}/Common/ExecutionModel/ \
                $${VTKINCLUDEDIR}/Common/Misc/ \
                $${VTKINCLUDEDIR}/Common/System/ \
                $${VTKINCLUDEDIR}/Common/Transforms/ \
                $${VTKINCLUDEDIR}/Common/Color/ \
                $${VTKINCLUDEDIR}/Imaging/Core/ \
                $${VTKINCLUDEDIR}/Imaging/Color/ \
                $${VTKINCLUDEDIR}/Imaging/Fourier/ \
                $${VTKINCLUDEDIR}/Imaging/General/ \
                $${VTKINCLUDEDIR}/Imaging/Hybrid/ \
                $${VTKINCLUDEDIR}/Imaging/Math/ \
                $${VTKINCLUDEDIR}/Imaging/Morphological/ \
                $${VTKINCLUDEDIR}/Imaging/OpenGL2/ \
                $${VTKINCLUDEDIR}/Imaging/Sources/ \
                $${VTKINCLUDEDIR}/Imaging/Statistics/ \
                $${VTKINCLUDEDIR}/Imaging/Stencil/ \
                $${VTKINCLUDEDIR}/IO/ADIOS/ \
                $${VTKINCLUDEDIR}/IO/Core/ \
                $${VTKINCLUDEDIR}/IO/EnSight/ \
                $${VTKINCLUDEDIR}/IO/Exodus/ \
                $${VTKINCLUDEDIR}/IO/Export/ \
                $${VTKINCLUDEDIR}/IO/ExportOpenGL/ \
                $${VTKINCLUDEDIR}/IO/ExportOpenGL2/ \
                $${VTKINCLUDEDIR}/IO/FFMPEG/ \
                $${VTKINCLUDEDIR}/IO/GDAL/ \
                $${VTKINCLUDEDIR}/IO/GeoJSON/ \
                $${VTKINCLUDEDIR}/IO/Geometry/ \
                $${VTKINCLUDEDIR}/IO/Image/ \
                $${VTKINCLUDEDIR}/IO/Import/ \
                $${VTKINCLUDEDIR}/IO/Infovis/ \
                $${VTKINCLUDEDIR}/IO/LAS/ \
                $${VTKINCLUDEDIR}/IO/Legacy/ \
                $${VTKINCLUDEDIR}/IO/LSDyna/ \
                $${VTKINCLUDEDIR}/IO/MINC/ \
                $${VTKINCLUDEDIR}/IO/Movie/ \
                $${VTKINCLUDEDIR}/IO/MPIImage/ \
                $${VTKINCLUDEDIR}/IO/MPIParallel/ \
                $${VTKINCLUDEDIR}/IO/MySQL/ \
                $${VTKINCLUDEDIR}/IO/NetCDF/ \
                $${VTKINCLUDEDIR}/IO/ODBC/ \
                $${VTKINCLUDEDIR}/IO/Parallel/ \
                $${VTKINCLUDEDIR}/IO/ParallelExodus/ \
                $${VTKINCLUDEDIR}/IO/ParallelLSDyna/ \
                $${VTKINCLUDEDIR}/IO/ParallelNetCDF/ \
                $${VTKINCLUDEDIR}/IO/ParallelXdmf3/ \
                $${VTKINCLUDEDIR}/IO/ParallelXML/ \
                $${VTKINCLUDEDIR}/IO/PDAL/ \
                $${VTKINCLUDEDIR}/IO/PLY/ \
                $${VTKINCLUDEDIR}/IO/PostgreSQL/ \
                $${VTKINCLUDEDIR}/IO/SegY/ \
                $${VTKINCLUDEDIR}/IO/TecplotTable/ \
                $${VTKINCLUDEDIR}/IO/Video/ \
                $${VTKINCLUDEDIR}/IO/VPIC/ \
                $${VTKINCLUDEDIR}/IO/Xdmf2/ \
                $${VTKINCLUDEDIR}/IO/Xdmf3/ \
                $${VTKINCLUDEDIR}/IO/XML/ \
                $${VTKINCLUDEDIR}/IO/XMLParser/ \
                $${VTKINCLUDEDIR}/GUISupport/MFC/ \
                $${VTKINCLUDEDIR}/GUISupport/Qt/ \
                $${VTKINCLUDEDIR}/GUISupport/QtOpenGL/ \
                $${VTKINCLUDEDIR}/GUISupport/QtSQL/ \
                $${VTKINCLUDEDIR}/GUISupport/QtWebkit/ \
                $${VTKINCLUDEDIR}/Rendering/Annotation/ \
                $${VTKINCLUDEDIR}/Rendering/Context2D/ \
                $${VTKINCLUDEDIR}/Rendering/ContextOpenGL/ \
                $${VTKINCLUDEDIR}/Rendering/ContextOpenGL2/ \
                $${VTKINCLUDEDIR}/Rendering/Core/ \
                $${VTKINCLUDEDIR}/Rendering/External/ \
                $${VTKINCLUDEDIR}/Rendering/FreeType/ \
                $${VTKINCLUDEDIR}/Rendering/FreeTypeFontConfig/ \
                $${VTKINCLUDEDIR}/Rendering/GL2PS/ \
                $${VTKINCLUDEDIR}/Rendering/GL2PSOpenGL2/ \
                $${VTKINCLUDEDIR}/Rendering/Label/ \
                $${VTKINCLUDEDIR}/Rendering/Image/ \
                $${VTKINCLUDEDIR}/Rendering/Core/ \
                $${VTKINCLUDEDIR}/Rendering/LIC/ \
                $${VTKINCLUDEDIR}/Rendering/LICOpenGL2/ \
                $${VTKINCLUDEDIR}/Rendering/LOD/ \
                $${VTKINCLUDEDIR}/Rendering/Matplotlib/ \
                $${VTKINCLUDEDIR}/Rendering/Oculus/ \
                $${VTKINCLUDEDIR}/Rendering/OpenGL/ \
                $${VTKINCLUDEDIR}/Rendering/OpenGL2/ \
                $${VTKINCLUDEDIR}/Rendering/OpenVR/ \
                $${VTKINCLUDEDIR}/Rendering/External/ \
                $${VTKINCLUDEDIR}/Rendering/OptiX/ \
                $${VTKINCLUDEDIR}/Rendering/OSPRay/ \
                $${VTKINCLUDEDIR}/Rendering/GL2PS/ \
                $${VTKINCLUDEDIR}/Rendering/Parallel/ \
                $${VTKINCLUDEDIR}/Rendering/ParallelLIC/ \
                $${VTKINCLUDEDIR}/Rendering/Qt/ \
                $${VTKINCLUDEDIR}/Rendering/SceneGraph/ \
                $${VTKINCLUDEDIR}/Rendering/Tk/ \
                $${VTKINCLUDEDIR}/Rendering/Volume/ \
                $${VTKINCLUDEDIR}/Rendering/VolumeAMR/ \
                $${VTKINCLUDEDIR}/Rendering/VolumeOpenGL/ \
                $${VTKINCLUDEDIR}/Rendering/VolumeOpenGL2/ \
                $${VTKINCLUDEDIR}/Filters/Core/ \
                $${VTKINCLUDEDIR}/Filters/Extraction/ \
                $${VTKINCLUDEDIR}/Filters/FlowPaths/ \
                $${VTKINCLUDEDIR}/Filters/General/ \
                $${VTKINCLUDEDIR}/Filters/Generic/ \
                $${VTKINCLUDEDIR}/Filters/Geometry/ \
                $${VTKINCLUDEDIR}/Filters/Hybrid/ \
                $${VTKINCLUDEDIR}/Filters/HyperTree/ \
                $${VTKINCLUDEDIR}/Filters/Imaging/ \
                $${VTKINCLUDEDIR}/Filters/Modeling/ \
                $${VTKINCLUDEDIR}/Filters/OpenTurns/ \
                $${VTKINCLUDEDIR}/Filters/Parallel/ \
                $${VTKINCLUDEDIR}/Filters/ParallelDIY2/ \
                $${VTKINCLUDEDIR}/Filters/ParallelFlowPaths/ \
                $${VTKINCLUDEDIR}/Filters/ParallelGeometry/ \
                $${VTKINCLUDEDIR}/Filters/ParallelImaging/ \
                $${VTKINCLUDEDIR}/Filters/ParallelMPI/ \
                $${VTKINCLUDEDIR}/Filters/ParallelStatistics/ \
                $${VTKINCLUDEDIR}/Filters/ParallelVerdict/ \
                $${VTKINCLUDEDIR}/Filters/Points/ \
                $${VTKINCLUDEDIR}/Filters/Programmable/ \
                $${VTKINCLUDEDIR}/Filters/Python/ \
                $${VTKINCLUDEDIR}/Filters/ReebGraph/ \
                $${VTKINCLUDEDIR}/Filters/Selection/ \
                $${VTKINCLUDEDIR}/Filters/SMP/ \
                $${VTKINCLUDEDIR}/Filters/Sources/ \
                $${VTKINCLUDEDIR}/Filters/Statistics/ \
                $${VTKINCLUDEDIR}/Filters/Qt/ \
                $${VTKINCLUDEDIR}/Filters/SceneGraph/ \
                $${VTKINCLUDEDIR}/Filters/Texture/ \
                $${VTKINCLUDEDIR}/Utilities/KWIML/ \
                $${VTKINCLUDEDIR}/Utilities/KWIML/vtkkwiml/ \
                $${VTKINCLUDEDIR}/Interaction/Widgets/ \
                $${VTKINCLUDEDIR}/Interaction/Style/ \
                $${VTKINCLUDEDIR}/Interaction/Image/ \
                $${VTKINCLUDEDIR}/Filters/Verdict

VTKLIBSUFFIX = -8.2
VTKLIBS = \
    vtkCommonCore \
    vtkCommonDataModel \
    vtkCommonExecutionModel \
    vtkCommonMath \
    vtkCommonMisc \
    vtkCommonSystem \
    vtkCommonTransforms \
    vtkFiltersCore \
    vtkFiltersGeneral \
    vtkFiltersSources \
    vtkGUISupportQt \
    vtkImagingCore \
    vtkImagingGeneral \
    vtkImagingHybrid \
    vtkImagingMath \
    vtkInteractionStyle \
    vtkInteractionWidgets \
    vtkIOImage \
    vtkRenderingAnnotation \
    vtkRenderingCore \
    vtkRenderingFreeType \
    vtkRenderingImage \
    vtkRenderingOpenGL2 \
    vtkRenderingVolume \
    vtkRenderingVolumeOpenGL2

LIBS += -L$${VTKLIBDIR}

for(lib, VTKLIBS) {
    LIBS += -l$${lib}$${VTKLIBSUFFIX}
}
