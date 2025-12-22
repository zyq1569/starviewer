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
                $${VTKINCLUDEDIR}/Rendering/UI/ \
                $${VTKINCLUDEDIR}/Rendering/Volume/ \
                $${VTKINCLUDEDIR}/Rendering/VolumeAMR/ \
                $${VTKINCLUDEDIR}/Rendering/VolumeOpenGL/ \
                $${VTKINCLUDEDIR}/Rendering/VolumeOpenGL2/ \
                $${VTKINCLUDEDIR}/Rendering/HyperTreeGrid/ \
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
                $${VTKINCLUDEDIR}/Filters/Verdict/ \
                $${VTKINCLUDEDIR}/ThirdParty/nlohmannjson/ \
                $${VTKINCLUDEDIR}/Utilities/KWSys/
 #               $${VTKINCLUDEDIR}/Filters/Verdict/ \\
                #$${VTKINCLUDEDIR_VC}/more/ \
                #$${VTKINCLUDEDIR_VC}/Common/Core/ \
                #$${VTKINCLUDEDIR_VC}/Common/DataModel/ \
                #$${VTKINCLUDEDIR_VC}/Common/Math/ \
                #$${VTKINCLUDEDIR_VC}/Common/ComputationalGeometry/ \
                #$${VTKINCLUDEDIR_VC}/Common/ExecutionModel/ \
                #$${VTKINCLUDEDIR_VC}/Common/Misc/ \
                #$${VTKINCLUDEDIR_VC}/Common/System/ \
                #$${VTKINCLUDEDIR_VC}/Common/Transforms/ \
                #$${VTKINCLUDEDIR_VC}/Common/Color/ \
                #$${VTKINCLUDEDIR_VC}/Imaging/Core/ \
                #$${VTKINCLUDEDIR_VC}/Imaging/Color/ \
                #$${VTKINCLUDEDIR_VC}/Imaging/Fourier/ \
                #$${VTKINCLUDEDIR_VC}/Imaging/General/ \
                #$${VTKINCLUDEDIR_VC}/Imaging/Hybrid/ \
                #$${VTKINCLUDEDIR_VC}/Imaging/Math/ \
                #$${VTKINCLUDEDIR_VC}/Imaging/Morphological/ \
                #$${VTKINCLUDEDIR_VC}/Imaging/OpenGL2/ \
                #$${VTKINCLUDEDIR_VC}/Imaging/Sources/ \
                #$${VTKINCLUDEDIR_VC}/Imaging/Statistics/ \
                #$${VTKINCLUDEDIR_VC}/Imaging/Stencil/ \
                #$${VTKINCLUDEDIR_VC}/IO/ADIOS/ \
                #$${VTKINCLUDEDIR_VC}/IO/Core/ \
                #$${VTKINCLUDEDIR_VC}/IO/EnSight/ \
                #$${VTKINCLUDEDIR_VC}/IO/Exodus/ \
                #$${VTKINCLUDEDIR_VC}/IO/Export/ \
                #$${VTKINCLUDEDIR_VC}/IO/ExportOpenGL/ \
                #$${VTKINCLUDEDIR_VC}/IO/ExportOpenGL2/ \
                #$${VTKINCLUDEDIR_VC}/IO/FFMPEG/ \
                #$${VTKINCLUDEDIR_VC}/IO/GDAL/ \
                #$${VTKINCLUDEDIR_VC}/IO/GeoJSON/ \
                #$${VTKINCLUDEDIR_VC}/IO/Geometry/ \
                #$${VTKINCLUDEDIR_VC}/IO/Image/ \
                #$${VTKINCLUDEDIR_VC}/IO/Import/ \
                #$${VTKINCLUDEDIR_VC}/IO/Infovis/ \
                #$${VTKINCLUDEDIR_VC}/IO/LAS/ \
                #$${VTKINCLUDEDIR_VC}/IO/Legacy/ \
                #$${VTKINCLUDEDIR_VC}/IO/LSDyna/ \
                #$${VTKINCLUDEDIR_VC}/IO/MINC/ \
                #$${VTKINCLUDEDIR_VC}/IO/Movie/ \
                #$${VTKINCLUDEDIR_VC}/IO/MPIImage/ \
                #$${VTKINCLUDEDIR_VC}/IO/MPIParallel/ \
                #$${VTKINCLUDEDIR_VC}/IO/MySQL/ \
                #$${VTKINCLUDEDIR_VC}/IO/NetCDF/ \
                #$${VTKINCLUDEDIR_VC}/IO/ODBC/ \
                #$${VTKINCLUDEDIR_VC}/IO/Parallel/ \
                #$${VTKINCLUDEDIR_VC}/IO/ParallelExodus/ \
                #$${VTKINCLUDEDIR_VC}/IO/ParallelLSDyna/ \
                #$${VTKINCLUDEDIR_VC}/IO/ParallelNetCDF/ \
                #$${VTKINCLUDEDIR_VC}/IO/ParallelXdmf3/ \
                #$${VTKINCLUDEDIR_VC}/IO/ParallelXML/ \
                #$${VTKINCLUDEDIR_VC}/IO/PDAL/ \
                #$${VTKINCLUDEDIR_VC}/IO/PLY/ \
                #$${VTKINCLUDEDIR_VC}/IO/PostgreSQL/ \
                #$${VTKINCLUDEDIR_VC}/IO/SegY/ \
                #$${VTKINCLUDEDIR_VC}/IO/TecplotTable/ \
                #$${VTKINCLUDEDIR_VC}/IO/Video/ \
                #$${VTKINCLUDEDIR_VC}/IO/VPIC/ \
                #$${VTKINCLUDEDIR_VC}/IO/Xdmf2/ \
                #$${VTKINCLUDEDIR_VC}/IO/Xdmf3/ \
                #$${VTKINCLUDEDIR_VC}/IO/XML/ \
                #$${VTKINCLUDEDIR_VC}/IO/XMLParser/ \
                #$${VTKINCLUDEDIR_VC}/GUISupport/MFC/ \
                #$${VTKINCLUDEDIR_VC}/GUISupport/Qt/ \
                #$${VTKINCLUDEDIR_VC}/GUISupport/QtOpenGL/ \
                #$${VTKINCLUDEDIR_VC}/GUISupport/QtSQL/ \
                #$${VTKINCLUDEDIR_VC}/GUISupport/QtWebkit/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/Annotation/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/Context2D/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/ContextOpenGL/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/ContextOpenGL2/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/Core/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/External/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/FreeType/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/FreeTypeFontConfig/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/GL2PS/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/GL2PSOpenGL2/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/Label/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/Image/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/Core/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/LIC/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/LICOpenGL2/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/LOD/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/Matplotlib/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/Oculus/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/OpenGL/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/OpenGL2/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/OpenVR/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/External/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/OptiX/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/OSPRay/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/GL2PS/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/Parallel/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/ParallelLIC/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/Qt/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/SceneGraph/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/Tk/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/Volume/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/VolumeAMR/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/VolumeOpenGL/ \
                #$${VTKINCLUDEDIR_VC}/Rendering/VolumeOpenGL2/ \
                #$${VTKINCLUDEDIR_VC}/Filters/Core/ \
                #$${VTKINCLUDEDIR_VC}/Filters/Extraction/ \
                #$${VTKINCLUDEDIR_VC}/Filters/FlowPaths/ \
                #$${VTKINCLUDEDIR_VC}/Filters/General/ \
                #$${VTKINCLUDEDIR_VC}/Filters/Generic/ \
                #$${VTKINCLUDEDIR_VC}/Filters/Geometry/ \
                #$${VTKINCLUDEDIR_VC}/Filters/Hybrid/ \
                #$${VTKINCLUDEDIR_VC}/Filters/HyperTree/ \
                #$${VTKINCLUDEDIR_VC}/Filters/Imaging/ \
                #$${VTKINCLUDEDIR_VC}/Filters/Modeling/ \
                #$${VTKINCLUDEDIR_VC}/Filters/OpenTurns/ \
                #$${VTKINCLUDEDIR_VC}/Filters/Parallel/ \
                #$${VTKINCLUDEDIR_VC}/Filters/ParallelDIY2/ \
                #$${VTKINCLUDEDIR_VC}/Filters/ParallelFlowPaths/ \
                #$${VTKINCLUDEDIR_VC}/Filters/ParallelGeometry/ \
                #$${VTKINCLUDEDIR_VC}/Filters/ParallelImaging/ \
                #$${VTKINCLUDEDIR_VC}/Filters/ParallelMPI/ \
                #$${VTKINCLUDEDIR_VC}/Filters/ParallelStatistics/ \
                #$${VTKINCLUDEDIR_VC}/Filters/ParallelVerdict/ \
                #$${VTKINCLUDEDIR_VC}/Filters/Points/ \
                #$${VTKINCLUDEDIR_VC}/Filters/Programmable/ \
                #$${VTKINCLUDEDIR_VC}/Filters/Python/ \
                #$${VTKINCLUDEDIR_VC}/Filters/ReebGraph/ \
                #$${VTKINCLUDEDIR_VC}/Filters/Selection/ \
                #$${VTKINCLUDEDIR_VC}/Filters/SMP/ \
                #$${VTKINCLUDEDIR_VC}/Filters/Sources/ \
                #$${VTKINCLUDEDIR_VC}/Filters/Statistics/ \
                #$${VTKINCLUDEDIR_VC}/Filters/Qt/ \
                #$${VTKINCLUDEDIR_VC}/Filters/SceneGraph/ \
                #$${VTKINCLUDEDIR_VC}/Filters/Texture/ \
                #$${VTKINCLUDEDIR_VC}/Utilities/KWIML/ \
                #$${VTKINCLUDEDIR_VC}/Utilities/KWIML/vtkkwiml/ \
                #$${VTKINCLUDEDIR_VC}/Interaction/Widgets/ \
                #$${VTKINCLUDEDIR_VC}/Interaction/Style/ \
                #$${VTKINCLUDEDIR_VC}/Interaction/Image/ \
                #$${VTKINCLUDEDIR_VC}/Filters/Verdict

VTKLIBSUFFIX = -9.4.a
VTKLIBS = \
#            vtkCommonCore                 \
#            vtkCommonDataModel            \
#            vtkCommonExecutionModel       \
#            vtkCommonMath                 \
#            vtkCommonMisc                 \
#            vtkCommonSystem               \
#            vtkCommonTransforms           \
#            vtkFiltersCore                \
#            vtkFiltersGeneral             \
#            vtkFiltersSources             \
#            vtkGUISupportQt               \
#            vtkImagingCore                \
#            vtkImagingGeneral             \
#            vtkImagingHybrid              \
#            vtkImagingMath                \
#            vtkInteractionStyle           \
#            vtkInteractionWidgets         \
#            vtkIOImage                    \
#            vtkRenderingAnnotation        \
#            vtkRenderingCore              \
#            vtkRenderingFreeType          \
#            vtkRenderingImage             \
#            vtkRenderingOpenGL2           \
#            vtkRenderingVolume            \
#            vtkRenderingVolumeOpenGL2     \
#            vtksys                        \
#            vtktiff                       \
#            vtkjpeg                       \
#            vtkpng                        \
#            vtkmetaio                     \
#            #vtkglew                       \
#            vtkfreetype                   \
#            vtkFiltersGeometry            \
#            vtkCommonColor                \
#            vtkzlib                       \
#            vtkglad                 \
#            vtkChartsCore
#
#
            vtkCommonCore                                  \
            vtkCommonDataModel                        \
            vtkCommonExecutionModel       \
            vtkCommonTransforms           \
            vtkCommonComputationalGeometry   \
            vtkFiltersCore                          \
            vtkFiltersModeling          \
            vtkRenderingCore              \
            vtkRenderingFreeType          \
            vtkRenderingOpenGL2           \
            vtkRenderingUI                         \
            vtkInteractionStyle           \
            vtkInteractionWidgets            \
            vtkCommonMath                 \
            vtkCommonMisc                 \
            vtkCommonColor   \
            vtkCommonSystem               \
            vtkFiltersGeneral             \
            vtkFiltersSources             \
            vtkFiltersExtraction           \
            vtkFiltersStatistics              \
            vtkVerdict             \
            vtkFiltersGeometry              \
            vtkFiltersTexture                 \
            vtkFiltersCellGrid                    \
            vtkGUISupportQt               \
            vtkDICOMParser                  \
            vtkImagingCore                \
            vtkImagingGeneral             \
            vtkglad                           \
            vtkImagingHybrid              \
            vtkParallelDIY                  \
            vtkImagingMath                \
            vtkInteractionImage        \
            vtkImagingGeneral        \
            vtkIOImage                    \
            vtkIOCore                    \
            vtkIOXML                                \
            vtkIOXMLParser                     \
            vtkRenderingAnnotation        \
            vtkRenderingImage             \
            vtkRenderingVolume            \
            vtkRenderingVolumeOpenGL2     \
            vtkRenderingContext2D  \
            vtkRenderingHyperTreeGrid        \
            vtkpugixml                \
            vtksys          \
            vtktiff                       \
            vtkjpeg                       \
            vtkpng                        \
            vtkmetaio                     \
            vtkIOLegacy                  \
            vtkIOCellGrid                  \
            #vtkglew                       \
            vtkImagingColor             \
            vtkfreetype                   \
            vtkFiltersGeometry            \
            vtkFiltersReduction            \
            vtkFiltersHyperTree               \
            vtkFiltersVerdict                      \
            vtkCommonColor                \
            vtkzlib                       \
            vtktoken                   \
            vtkChartsCore   \
            vtkIOCore  \
            vtkpugixml  \
            vtkdoubleconversion  \
            vtkFiltersGeneral  \
            vtkFiltersModeling  \
            vtkFiltersHybrid  \
            vtkImagingSources  \
            vtkParallelCore  \
            vtkParallelDIY  \
            vtkImagingGeneral  \
            vtkFiltersExtraction           \
            vtkpugixml   \
            vtkdoubleconversion  \
            vtkInfovisCore             \
            vtkloguru  \           # 解决所有 vtkloguru::xxx
            vtkfmt  \              # 解决 vtkfmt::vformat
            vtkkissfft  \          # 解决 _vtkkissfft_fft 等
            vtklz4  \              # 解决 _vtklz4_LZ4_xxx
            vtklzma  \             # 解决 _vtklzma_xxx
            vtkexpat  \            # vtkXMLParser 依赖 expat（最关键！）
            vtkzlib               # zlib 压缩

#LIBS += -L$${VTKLIBDIR}
#
#for(lib, VTKLIBS) {
#    LIBS += -l$${lib}$${VTKLIBSUFFIX}
#}

LIBS += -L$${VTKLIBDIR}

for(lib, VTKLIBS) {
    LIBS += -Wl,-force_load,$${VTKLIBDIR}$${lib}$${VTKLIBSUFFIX}
}
