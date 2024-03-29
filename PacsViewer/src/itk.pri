include(defaultdirectories.pri)

#INCLUDEPATH += $${ITKINCLUDEDIR}

#5.0.1
INCLUDEPATH +=  $${ITKINCLUDEDIR} \
                $${ITKINCLUDEDIR}/Modules/Core/Common/include/ \
                $${ITKINCLUDEDIR}/Modules/Core/FiniteDifference/include/ \
                $${ITKINCLUDEDIR}/Modules/Core/GPUCommon/include/ \
                $${ITKINCLUDEDIR}/Modules/Core/GPUFiniteDifference/include/ \
                $${ITKINCLUDEDIR}/Modules/Core/ImageAdaptors/include/ \
                $${ITKINCLUDEDIR}/Modules/Core/ImageFunction/include/ \
                $${ITKINCLUDEDIR}/Modules/Core/Mesh/include/ \
                $${ITKINCLUDEDIR}/Modules/Core/QuadEdgeMesh/include/ \
                $${ITKINCLUDEDIR}/Modules/Core/SpatialObjects/include/ \
                $${ITKINCLUDEDIR}/Modules/Core/TestKernel/include/ \
                $${ITKINCLUDEDIR}/Modules/Core/Transform/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/ImageBase/include/ \
                $${ITKINCLUDEDIR}/Modules/Bridge/NumPy/include/ \
                $${ITKINCLUDEDIR}/Modules/Bridge/VTK/include/ \
                $${ITKINCLUDEDIR}/Modules/Bridge/VtkGlue/include/ \
                $${ITKINCLUDEDIR}/Modules/Compatibility/Deprecated/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/AnisotropicSmoothing/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/AntiAlias/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/BiasCorrection/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/BinaryMathematicalMorphology/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/Colormap/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/Convolution/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/CurvatureFlow/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/Deconvolution/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/DiffusionTensorImage/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/DisplacementField/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/DistanceMap/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/FFT/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/GPUAnisotropicSmoothing/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/GPUImageFilterBase/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/GPUSmoothing/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/GPUThresholding/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/FastMarching/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/ImageCompare/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/ImageCompose/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/ImageFeature/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/ImageFilterBase/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/ImageFrequency/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/ImageFusion/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/ImageGradient/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/ImageGrid/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/ImageIntensity/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/ImageLabel/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/ImageNoise/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/ImageSources/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/ImageStatistics/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/LabelMap/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/MathematicalMorphology/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/Path/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/Smoothing/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/SpatialFunction/include/ \
                $${ITKINCLUDEDIR}/Modules/Filtering/Thresholding/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/BioRad/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/BMP/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/Bruker/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/CSV/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/DCMTK/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/GDCM/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/GE/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/GIPL/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/HDF5/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/ImageBase/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/IPL/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/JPEG/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/JPEG2000/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/LSM/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/Mesh/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/MeshBYU/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/MeshFreeSurfer/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/MeshGifti/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/MeshOBJ/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/MeshOFF/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/MeshVTK/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/Meta/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/MINC/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/MRC/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/NIFTI/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/NRRD/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/PhilipsREC/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/PNG/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/RAW/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/Siemens/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/SpatialObjects/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/TIFF/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/TransformBase/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/TransformFactory/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/TransformHDF5/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/TransformInsightLegacy/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/TransformMatlab/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/TransformMINC/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/VTK/include/ \
                $${ITKINCLUDEDIR}/Modules/IO/XML/include/ \
                $${ITKINCLUDEDIR}/Modules/Nonunit/IntegratedTest/include/ \
                $${ITKINCLUDEDIR}/Modules/Nonunit/Review/include/ \
                $${ITKINCLUDEDIR}/Modules/Numerics/Eigen/include/ \
                $${ITKINCLUDEDIR}/Modules/Numerics/FEM/include/ \
                $${ITKINCLUDEDIR}/Modules/Numerics/NarrowBand/include/ \
                $${ITKINCLUDEDIR}/Modules/Numerics/Optimizers/include/ \
                $${ITKINCLUDEDIR}/Modules/Numerics/Optimizersv4/include/ \
                $${ITKINCLUDEDIR}/Modules/Numerics/Polynomials/include/ \
                $${ITKINCLUDEDIR}/Modules/Numerics/Statistics/include/ \
                $${ITKINCLUDEDIR}/Modules/Registration/Common/include/ \
                $${ITKINCLUDEDIR}/Modules/Registration/FEM/include/ \
                $${ITKINCLUDEDIR}/Modules/Registration/GPUCommon/include/ \
                $${ITKINCLUDEDIR}/Modules/Registration/GPUPDEDeformable/include/ \
                $${ITKINCLUDEDIR}/Modules/Registration/Metricsv4/include/ \
                $${ITKINCLUDEDIR}/Modules/Registration/PDEDeformable/include/ \
                $${ITKINCLUDEDIR}/Modules/Registration/RegistrationMethodsv4/include/ \
                $${ITKINCLUDEDIR}/Modules/Segmentation/Classifiers/include/ \
                $${ITKINCLUDEDIR}/Modules/Segmentation/ConnectedComponents/include/ \
                $${ITKINCLUDEDIR}/Modules/Segmentation/DeformableMesh/include/ \
                $${ITKINCLUDEDIR}/Modules/Segmentation/KLMRegionGrowing/include/ \
                $${ITKINCLUDEDIR}/Modules/Segmentation/LabelVoting/include/ \
                $${ITKINCLUDEDIR}/Modules/Segmentation/LevelSets/include/ \
                $${ITKINCLUDEDIR}/Modules/Segmentation/LevelSetsv4/include/ \
                $${ITKINCLUDEDIR}/Modules/Segmentation/MarkovRandomFieldsClassifiers/include/ \
                $${ITKINCLUDEDIR}/Modules/Segmentation/RegionGrowing/include/ \
                $${ITKINCLUDEDIR}/Modules/Segmentation/SignedDistanceFunction/include/ \
                $${ITKINCLUDEDIR}/Modules/Segmentation/SuperPixel/include/ \
                $${ITKINCLUDEDIR}/Modules/Segmentation/Voronoi/include/ \
                $${ITKINCLUDEDIR}/Modules/Segmentation/Watersheds/include/ \
                $${ITKINCLUDEDIR}/Modules/ThirdParty/GoogleTest/src/itkgoogletest/googletest/include/ \
                $${ITKINCLUDEDIR}/Modules/ThirdParty/KWIML/src/itkkwiml/include/ \
                $${ITKINCLUDEDIR}/Modules/ThirdParty/MINC/src/libminc/volume_io/include/ \
                $${ITKINCLUDEDIR}/Modules/ThirdParty/libLBFGS/include/ \
                $${ITKINCLUDEDIR}/Modules/ThirdParty/VNL/src/vxl/core/ \
                $${ITKINCLUDEDIR}/Modules/ThirdParty/MetaIO/src/MetaIO/src/ \
                $${ITKINCLUDEDIR}/Modules/ThirdParty/VNL/src/vxl/vcl/ \
                $${ITKINCLUDEDIR}/Modules/ThirdParty/ZLIB/src/itkzlib/ \
                $${ITKINCLUDEDIR}/Modules/ThirdParty/Eigen3/src/ \
                $${ITKINCLUDEDIR}/Modules/ThirdParty/Eigen3/src/itkeigen/Eigen/ \
                $${ITKINCLUDEDIR}/Modules/ThirdParty/KWSys/src/KWSys/ \
                $${ITKINCLUDEDIR}/Modules/ThirdParty/GDCM/src/gdcm/Utilities/VTK/ \
                $${ITKINCLUDEDIR}/Modules/Video/BridgeOpenCV/include/ \
                $${ITKINCLUDEDIR}/Modules/Video/BridgeVXL/include/ \
                $${ITKINCLUDEDIR}/Modules/Video/Core/include/ \
                $${ITKINCLUDEDIR}/Modules/Video/Filtering/include/ \
                $${ITKINCLUDEDIR}/Modules/Video/IO/include/ \
                $${ITKINCLUDEDIR}/more/

#ITKLIBSUFFIX = -4.13
ITKLIBSUFFIX = -5.0
ITKLIBS = \
        ITKCommon \
        ITKMetaIO \
        ITKNrrdIO \
        ITKSpatialObjects \
        ITKStatistics \
        itkjpeg \
        itksys \
        itkvnl_algo \
        itkvnl \
        itkvcl \
        itkv3p_netlib \
        ITKVNLInstantiation \
        ITKniftiio \
        ITKznz \
        ITKOptimizers \
        ITKVTK \
        ITKIOGDCM \
        ITKIOJPEG \
        ITKIOImageBase \
        itksys \
        itkdouble-conversion \
        ITKTransform \
        itkzlib \
        ITKEXPAT

macx:official_release:ITKLIBS += itkzlib

LIBS += -L$${ITKLIBDIR}

for(lib, ITKLIBS) {
    LIBS += -l$${lib}$${ITKLIBSUFFIX}
}

win32:LIBS += -lShell32
unix:LIBS += -ldl
