#include "experimental3dvolume.h"

#include "ambientvoxelshader2.h"
#include "celshadingvoxelshader.h"
#include "colorbleedingvoxelshader.h"
#include "colorvomivoxelshader.h"
#include "contourvoxelshader.h"
#include "coolwarmvoxelshader.h"
#include "directilluminationvoxelshader2.h"
#include "filteringambientocclusionmapvoxelshader.h"
#include "filteringambientocclusionstipplingvoxelshader.h"
#include "filteringambientocclusionvoxelshader.h"
#include "imivoxelshader.h"
#include "obscurance.h"
#include "obscurancevoxelshader.h"
#include "opacityvoxelshader.h"
#include "transferfunction.h"
#include "vmivoxelshader2.h"
#include "volume.h"
#include "vomicoolwarmvoxelshader.h"
#include "vomigammavoxelshader.h"
#include "vomivoxelshader.h"
#include "voxelsaliencyvoxelshader.h"
#include "vtk4dlinearregressiongradientestimator.h"
#include "vtkVolumeRayCastVoxelShaderCompositeFunction.h"
#include "whitevoxelshader.h"

#include <vtkEncodedGradientShader.h>
#include <vtkFiniteDifferenceGradientEstimator.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageShiftScale.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkPointData.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include "vtkVolumeRayCastCompositeFunction.h"
#include "vtkVolumeRayCastMapper.h"

namespace udg {

Experimental3DVolume::Experimental3DVolume(Volume *volume)
    : m_alternativeImage(0), m_finiteDifferenceGradientEstimator(0), m_4DLinearRegressionGradientEstimator(0)
{
    createImage(volume->getVtkData());
    createVolumeRayCastFunctions();
    createVoxelShaders();
    createMappers();
    createProperty();
    createVolume();
}

Experimental3DVolume::Experimental3DVolume(vtkImageData *image)
    : m_alternativeImage(0), m_finiteDifferenceGradientEstimator(0), m_4DLinearRegressionGradientEstimator(0)
{
    createImage(image);
    createVolumeRayCastFunctions();
    createVoxelShaders();
    createMappers();
    createProperty();
    createVolume();
}

Experimental3DVolume::~Experimental3DVolume()
{
    m_image->Delete();
    m_simpleVolumeRayCastFunction->Delete();
    m_shaderVolumeRayCastFunction->Delete();
    delete m_ambientVoxelShader;
    delete m_directIlluminationVoxelShader;
    delete m_contourVoxelShader;
    delete m_celShadingVoxelShader;
    delete m_obscuranceVoxelShader;
    delete m_colorBleedingVoxelShader;
    delete m_coolWarmVoxelShader;
    delete m_whiteVoxelShader;
    delete m_vmiVoxelShader2;
    delete m_vomiVoxelShader;
    delete m_vomiGammaVoxelShader;
    delete m_vomiCoolWarmVoxelShader;
    delete m_voxelSaliencyVoxelShader;
    delete m_colorVomiVoxelShader;
    delete m_opacityVoxelShader;
    delete m_filteringAmbientOcclusionVoxelShader;
    delete m_filteringAmbientOcclusionMapVoxelShader;
    delete m_filteringAmbientOcclusionStipplingVoxelShader;
    m_cpuRayCastMapper->Delete();
    m_gpuRayCastMapper->Delete();
    m_property->Delete();
    m_volume->Delete();
    if (m_finiteDifferenceGradientEstimator)
    {
        m_finiteDifferenceGradientEstimator->Delete();
    }
    if (m_4DLinearRegressionGradientEstimator)
    {
        m_4DLinearRegressionGradientEstimator->Delete();
    }
}

void Experimental3DVolume::setAlternativeImage(vtkImageData *alternativeImage)
{
    m_alternativeImage = alternativeImage;
    m_ambientVoxelShader->setAlternativeData(reinterpret_cast<unsigned short*>(m_alternativeImage->GetScalarPointer()));
    m_directIlluminationVoxelShader->setAlternativeData(reinterpret_cast<unsigned short*>(m_alternativeImage->GetScalarPointer()));
}

void Experimental3DVolume::setExtension(const QExperimental3DExtension *extension)
{
    m_extension = extension;
    m_ambientVoxelShader->setExtension(m_extension);
    m_directIlluminationVoxelShader->setExtension(m_extension);
}

vtkImageData* Experimental3DVolume::getImage() const
{
    return m_image;
}

vtkVolume* Experimental3DVolume::getVolume() const
{
    return m_volume;
}

unsigned short Experimental3DVolume::getRangeMin() const
{
    return m_rangeMin;
}

unsigned short Experimental3DVolume::getRangeMax() const
{
    return m_rangeMax;
}

unsigned int Experimental3DVolume::getSize() const
{
    return m_dataSize;
}

const unsigned char* Experimental3DVolume::gradientMagnitudes() const
{
    return this->gradientEstimator()->GetGradientMagnitudes();
}

void Experimental3DVolume::setInterpolation(Interpolation interpolation)
{
    switch (interpolation)
    {
        case NearestNeighbour:
            m_property->SetInterpolationTypeToNearest();
            break;
        case LinearInterpolateClassify:
            m_property->SetInterpolationTypeToLinear();
            m_simpleVolumeRayCastFunction->SetCompositeMethodToInterpolateFirst();
            m_shaderVolumeRayCastFunction->SetCompositeMethodToInterpolateFirst();
            break;
        case LinearClassifyInterpolate:
            m_property->SetInterpolationTypeToLinear();
            m_simpleVolumeRayCastFunction->SetCompositeMethodToClassifyFirst();
            m_shaderVolumeRayCastFunction->SetCompositeMethodToClassifyFirst();
            break;
    }

    m_image->Modified();
}

void Experimental3DVolume::setGradientEstimator(GradientEstimator gradientEstimator)
{
    m_gradientEstimator = gradientEstimator;

    switch (gradientEstimator)
    {
        case FiniteDifference:
            if (!m_finiteDifferenceGradientEstimator)
            {
                m_finiteDifferenceGradientEstimator = vtkFiniteDifferenceGradientEstimator::New();
            }
            m_cpuRayCastMapper->SetGradientEstimator(m_finiteDifferenceGradientEstimator);
            m_finiteDifferenceGradientEstimator->SetInputData(m_alternativeImage ? m_alternativeImage : m_image);
            break;
        case FourDLInearRegression1:
            if (!m_4DLinearRegressionGradientEstimator)
            {
                m_4DLinearRegressionGradientEstimator = Vtk4DLinearRegressionGradientEstimator::New();
            }
            m_4DLinearRegressionGradientEstimator->setRadius(1);
            m_cpuRayCastMapper->SetGradientEstimator(m_4DLinearRegressionGradientEstimator);
            m_4DLinearRegressionGradientEstimator->SetInputData(m_alternativeImage ? m_alternativeImage : m_image);
            break;
        case FourDLInearRegression2:
            if (!m_4DLinearRegressionGradientEstimator)
            {
                m_4DLinearRegressionGradientEstimator = Vtk4DLinearRegressionGradientEstimator::New();
            }
            m_4DLinearRegressionGradientEstimator->setRadius(2);
            m_cpuRayCastMapper->SetGradientEstimator(m_4DLinearRegressionGradientEstimator);
            m_4DLinearRegressionGradientEstimator->SetInputData(m_alternativeImage ? m_alternativeImage : m_image);
            break;
    }
}

void Experimental3DVolume::resetShadingOptions()
{
    m_volume->SetMapper(m_cpuRayCastMapper);
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_shaderVolumeRayCastFunction);
    m_shaderVolumeRayCastFunction->RemoveAllVoxelShaders();
}

void Experimental3DVolume::addAmbientLighting()
{
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_simpleVolumeRayCastFunction);
    m_shaderVolumeRayCastFunction->AddVoxelShader(m_ambientVoxelShader);
    if (m_shaderVolumeRayCastFunction->IndexOfVoxelShader(m_ambientVoxelShader) == 0)
    {
        m_volume->SetMapper(m_gpuRayCastMapper);
    }
    m_property->ShadeOff();
    if (m_alternativeImage)
    {
        m_ambientVoxelShader->setGradientMagnitudes(m_cpuRayCastMapper->GetGradientEstimator()->GetGradientMagnitudes());
    }
}

void Experimental3DVolume::addFullLighting(double ambient, double diffuse, double specular, double specularPower)
{
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_simpleVolumeRayCastFunction);
    m_shaderVolumeRayCastFunction->AddVoxelShader(m_directIlluminationVoxelShader);
    if (m_shaderVolumeRayCastFunction->IndexOfVoxelShader(m_directIlluminationVoxelShader) == 0)
    {
        m_volume->SetMapper(m_gpuRayCastMapper);
    }
    m_property->ShadeOn();
    m_property->SetAmbient(ambient);
    m_property->SetDiffuse(diffuse);
    m_property->SetSpecular(specular);
    m_property->SetSpecularPower(specularPower);

    m_directIlluminationVoxelShader->setEncodedNormals(m_cpuRayCastMapper->GetGradientEstimator()->GetEncodedNormals());
    vtkEncodedGradientShader *gradientShader = m_cpuRayCastMapper->GetGradientShader();
    m_directIlluminationVoxelShader->setDiffuseShadingTables(gradientShader->GetRedDiffuseShadingTable(m_volume),
                                                             gradientShader->GetGreenDiffuseShadingTable(m_volume),
                                                             gradientShader->GetBlueDiffuseShadingTable(m_volume));
    m_directIlluminationVoxelShader->setSpecularShadingTables(gradientShader->GetRedSpecularShadingTable(m_volume),
                                                              gradientShader->GetGreenSpecularShadingTable(m_volume),
                                                              gradientShader->GetBlueSpecularShadingTable(m_volume));
    if (m_alternativeImage)
    {
        m_directIlluminationVoxelShader->setGradientMagnitudes(m_cpuRayCastMapper->GetGradientEstimator()->GetGradientMagnitudes());
    }
}

void Experimental3DVolume::addCoolWarm(float b, float y, float alpha, float beta)
{
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_shaderVolumeRayCastFunction);
    if (m_shaderVolumeRayCastFunction->IndexOfVoxelShader(m_coolWarmVoxelShader) < 0)
    {
        m_shaderVolumeRayCastFunction->AddVoxelShader(m_coolWarmVoxelShader);
    }
    m_coolWarmVoxelShader->setGradientEstimator(gradientEstimator());
    m_coolWarmVoxelShader->setBYAlphaBeta(b, y, alpha, beta);
    m_coolWarmVoxelShader->setCombine(m_shaderVolumeRayCastFunction->IndexOfVoxelShader(m_coolWarmVoxelShader) != 0);
    m_volume->SetMapper(m_cpuRayCastMapper);
}

void Experimental3DVolume::addWhite()
{
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_shaderVolumeRayCastFunction);
    m_shaderVolumeRayCastFunction->AddVoxelShader(m_whiteVoxelShader);
    m_volume->SetMapper(m_cpuRayCastMapper);
}

void Experimental3DVolume::addContour(double threshold)
{
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_shaderVolumeRayCastFunction);
    m_shaderVolumeRayCastFunction->AddVoxelShader(m_contourVoxelShader);
    m_contourVoxelShader->setGradientEstimator(gradientEstimator());
    m_contourVoxelShader->setThreshold(threshold);
    m_volume->SetMapper(m_cpuRayCastMapper);
}

void Experimental3DVolume::addCelShading(int quantums)
{
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_shaderVolumeRayCastFunction);
    m_shaderVolumeRayCastFunction->AddVoxelShader(m_celShadingVoxelShader);
    m_celShadingVoxelShader->setQuantums(quantums);
    m_volume->SetMapper(m_cpuRayCastMapper);
}

void Experimental3DVolume::addObscurance(Obscurance *obscurance, double factor, double filterLow, double filterHigh, bool additive, double weight)
{
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_shaderVolumeRayCastFunction);
    if (!obscurance->hasColor())
    {
        m_shaderVolumeRayCastFunction->AddVoxelShader(m_obscuranceVoxelShader);
        m_obscuranceVoxelShader->setObscurance(obscurance);
        m_obscuranceVoxelShader->setFactor(factor);
        m_obscuranceVoxelShader->setFilters(filterLow, filterHigh);
        m_obscuranceVoxelShader->setAdditive(additive, weight);
    }
    else
    {
        m_shaderVolumeRayCastFunction->AddVoxelShader(m_colorBleedingVoxelShader);
        m_colorBleedingVoxelShader->setColorBleeding(obscurance);
        m_colorBleedingVoxelShader->setFactor(factor);
    }
    m_volume->SetMapper(m_cpuRayCastMapper);
}

void Experimental3DVolume::setTransferFunction(const TransferFunction &transferFunction, bool useGradientOpacityTransferFunction)
{
    m_property->SetColor(transferFunction.vtkColorTransferFunction());
    m_property->SetScalarOpacity(transferFunction.vtkScalarOpacityTransferFunction());
    m_property->SetGradientOpacity(transferFunction.vtkGradientOpacityTransferFunction());
    if (useGradientOpacityTransferFunction)
    {
        m_property->DisableGradientOpacityOff();
        m_volume->SetMapper(m_cpuRayCastMapper);
    }
    else
    {
        m_property->DisableGradientOpacityOn();
    }
    m_ambientVoxelShader->setTransferFunction(transferFunction);
    m_directIlluminationVoxelShader->setTransferFunction(transferFunction);
    m_whiteVoxelShader->setTransferFunction(transferFunction);
    m_obscuranceVoxelShader->setTransferFunction(transferFunction);
    m_vmiVoxelShader2->setTransferFunction(transferFunction);
    m_vomiVoxelShader->setTransferFunction(transferFunction);
    m_vomiGammaVoxelShader->setTransferFunction(transferFunction);
    m_voxelSaliencyVoxelShader->setTransferFunction(transferFunction);
    m_imiVoxelShader->setTransferFunction(transferFunction);
    m_coolWarmVoxelShader->setTransferFunction(transferFunction);
    m_filteringAmbientOcclusionMapVoxelShader->setTransferFunction(transferFunction);
    m_filteringAmbientOcclusionStipplingVoxelShader->setTransferFunction(transferFunction);
}

void Experimental3DVolume::forceCpuRendering()
{
    m_volume->SetMapper(m_cpuRayCastMapper);
}

void Experimental3DVolume::forceCpuShaderRendering()
{
    m_volume->SetMapper(m_cpuRayCastMapper);
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_shaderVolumeRayCastFunction);
    // fem això perquè el mapper no ens foti enlaire les trampes perquè el gradient sigui el d'm_alternativeImage
    if (m_alternativeImage)
    {
        m_property->ShadeOff();
    }
}

void Experimental3DVolume::startVmiMode()
{
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_shaderVolumeRayCastFunction);
    m_volume->SetMapper(m_cpuRayCastMapper);
}

void Experimental3DVolume::startVmiSecondPass()
{
    m_shaderVolumeRayCastFunction->RemoveAllVoxelShaders();
    m_shaderVolumeRayCastFunction->AddVoxelShader(m_vmiVoxelShader2);
    m_vmiVoxelShader2->clearViewedVolumes();
}

QVector<float> Experimental3DVolume::finishVmiSecondPass()
{
    return m_vmiVoxelShader2->objectProbabilities();
}

float Experimental3DVolume::viewedVolumeInVmiSecondPass() const
{
    return m_vmiVoxelShader2->viewedVolume();
}

void Experimental3DVolume::addVomi(const QVector<float> &vomi, float minimumVomi, float maximumVomi, float factor, bool additive, float weight)
{
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_shaderVolumeRayCastFunction);
    if (m_shaderVolumeRayCastFunction->IndexOfVoxelShader(m_vomiVoxelShader) < 0)
    {
        m_shaderVolumeRayCastFunction->AddVoxelShader(m_vomiVoxelShader);
    }
    m_vomiVoxelShader->setVomi(vomi, minimumVomi, maximumVomi, factor);
    m_vomiVoxelShader->setCombine(m_shaderVolumeRayCastFunction->IndexOfVoxelShader(m_vomiVoxelShader) != 0);
    m_vomiVoxelShader->setAdditive(additive, weight);
    m_volume->SetMapper(m_cpuRayCastMapper);
}

void Experimental3DVolume::addVomiGamma(const QVector<float> &vomi, float maximumVomi, float factor, float gamma, bool additive, float weight)
{
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_shaderVolumeRayCastFunction);
    if (m_shaderVolumeRayCastFunction->IndexOfVoxelShader(m_vomiGammaVoxelShader) < 0)
    {
        m_shaderVolumeRayCastFunction->AddVoxelShader(m_vomiGammaVoxelShader);
    }
    m_vomiGammaVoxelShader->setVomi(vomi, maximumVomi, factor, gamma);
    m_vomiGammaVoxelShader->setAdditive(additive, weight);
    m_volume->SetMapper(m_cpuRayCastMapper);
}

void Experimental3DVolume::addVomiCoolWarm(const QVector<float> &vomi, float maximumVomi, float factor, float y, float b)
{
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_shaderVolumeRayCastFunction);
    if (m_shaderVolumeRayCastFunction->IndexOfVoxelShader(m_vomiCoolWarmVoxelShader) < 0)
    {
        m_shaderVolumeRayCastFunction->AddVoxelShader(m_vomiCoolWarmVoxelShader);
    }
    m_vomiCoolWarmVoxelShader->setVomi(vomi, maximumVomi, factor);
    m_vomiCoolWarmVoxelShader->setYB(y, b);
    m_volume->SetMapper(m_cpuRayCastMapper);
}

void Experimental3DVolume::addColorVomi(const QVector<Vector3Float> &colorVomi, float maximumColorVomi, float factor)
{
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_shaderVolumeRayCastFunction);
    if (m_shaderVolumeRayCastFunction->IndexOfVoxelShader(m_colorVomiVoxelShader) < 0)
    {
        m_shaderVolumeRayCastFunction->AddVoxelShader(m_colorVomiVoxelShader);
    }
    m_colorVomiVoxelShader->setColorVomi(colorVomi, maximumColorVomi, factor);
    m_volume->SetMapper(m_cpuRayCastMapper);
}

void Experimental3DVolume::addImi(const QVector<float> &imi, float maximumImi, float factor, bool additive, float weight)
{
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_shaderVolumeRayCastFunction);
    if (m_shaderVolumeRayCastFunction->IndexOfVoxelShader(m_imiVoxelShader) < 0)
    {
        m_shaderVolumeRayCastFunction->AddVoxelShader(m_imiVoxelShader);
    }
    m_imiVoxelShader->setImi(imi, maximumImi, factor);
    m_imiVoxelShader->setCombine(m_shaderVolumeRayCastFunction->IndexOfVoxelShader(m_imiVoxelShader) != 0);
    m_imiVoxelShader->setAdditive(additive, weight);
    m_volume->SetMapper(m_cpuRayCastMapper);
}

void Experimental3DVolume::addVoxelSaliencies(const QVector<float> &voxelSaliencies, float maximumSaliency, float factor)
{
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_shaderVolumeRayCastFunction);
    m_shaderVolumeRayCastFunction->AddVoxelShader(m_voxelSaliencyVoxelShader);
    m_voxelSaliencyVoxelShader->setVoxelSaliencies(voxelSaliencies, maximumSaliency, factor);
    m_volume->SetMapper(m_cpuRayCastMapper);
}

void Experimental3DVolume::addOpacity(const QVector<float> &data, float maximum, float lowThreshold, float lowFactor, float highThreshold, float highFactor)
{
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_shaderVolumeRayCastFunction);
    m_shaderVolumeRayCastFunction->AddVoxelShader(m_opacityVoxelShader);
    m_opacityVoxelShader->setData(data, maximum, lowThreshold, lowFactor, highThreshold, highFactor);
    m_volume->SetMapper(m_cpuRayCastMapper);
}

void Experimental3DVolume::addOpacity(const QVector<float> &data, float maximum)
{
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_shaderVolumeRayCastFunction);
    m_shaderVolumeRayCastFunction->AddVoxelShader(m_opacityVoxelShader);
    m_opacityVoxelShader->setData(data, maximum, 1.0f, 1.0f, std::numeric_limits<float>::infinity(), 1.0f);
    m_volume->SetMapper(m_cpuRayCastMapper);
}

QVector<float> Experimental3DVolume::computeVomiGradient(const QVector<float> &vomi)
{
    vtkFloatArray *vomiArray = vtkFloatArray::New();
    vomiArray->SetArray(const_cast<float*>(vomi.data()), m_dataSize, 1);
    vtkImageData *vomiData = vtkImageData::New();
    vomiData->CopyStructure(m_image);
    vtkPointData *vomiPointData = vomiData->GetPointData();
    vomiPointData->SetScalars(vomiArray);

    vtkEncodedGradientEstimator *gradientEstimator = this->gradientEstimator();
    gradientEstimator->SetInputData(vomiData);
    unsigned char *gradientMagnitudes = gradientEstimator->GetGradientMagnitudes();

    QVector<float> vomiGradient(m_dataSize);
    float maxVomiGradient = 0.0f;
    for (unsigned int i = 0; i < m_dataSize; i++)
    {
        if (gradientMagnitudes[i] > maxVomiGradient)
        {
            maxVomiGradient = gradientMagnitudes[i];
        }
    }
    for (unsigned int i = 0; i < m_dataSize; i++)
    {
        vomiGradient[i] = gradientMagnitudes[i] / maxVomiGradient;
    }

    gradientEstimator->SetInputData(m_image);

    vomiArray->Delete();
    vomiData->Delete();

    return vomiGradient;
}

void Experimental3DVolume::addFilteringAmbientOcclusion(const QVector<float> &filteringAmbientOcclusion, float maximum, float lambda)
{
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_shaderVolumeRayCastFunction);
    m_shaderVolumeRayCastFunction->AddVoxelShader(m_filteringAmbientOcclusionVoxelShader);
    m_filteringAmbientOcclusionVoxelShader->setFilteringAmbientOcclusion(filteringAmbientOcclusion, maximum, lambda);
    m_volume->SetMapper(m_cpuRayCastMapper);
}

void Experimental3DVolume::addFilteringAmbientOcclusionMap(const QVector<float> &filteringAmbientOcclusion, float maximum, float factor)
{
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_shaderVolumeRayCastFunction);
    m_shaderVolumeRayCastFunction->AddVoxelShader(m_filteringAmbientOcclusionMapVoxelShader);
    m_filteringAmbientOcclusionMapVoxelShader->setFilteringAmbientOcclusion(filteringAmbientOcclusion, maximum, factor);
    m_volume->SetMapper(m_cpuRayCastMapper);
}

void Experimental3DVolume::addFilteringAmbientOcclusionStippling(const QVector<float> &filteringAmbientOcclusion, float maximum, float threshold, float factor)
{
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_shaderVolumeRayCastFunction);
    m_shaderVolumeRayCastFunction->AddVoxelShader(m_filteringAmbientOcclusionStipplingVoxelShader);
    m_filteringAmbientOcclusionStipplingVoxelShader->setFilteringAmbientOcclusion(filteringAmbientOcclusion, maximum, threshold, factor);
    m_volume->SetMapper(m_cpuRayCastMapper);
}

void Experimental3DVolume::createImage(vtkImageData *image)
{
    // sembla que el volum arriba sempre com a short
    // normalment els volums aprofiten només 12 bits com a màxim, per tant no hi hauria d'haver problema

    double *range = image->GetScalarRange();
    double min = range[0], max = range[1];
    DEBUG_LOG(QString("original range: min = %1, max = %2").arg(min).arg(max));
    double shift = -min;
    // fem servir directament un vtkImageShiftScale, que permet fer castings també
    vtkImageShiftScale *imageShiftScale = vtkImageShiftScale::New();
    imageShiftScale->SetInputData(image);
    imageShiftScale->SetOutputScalarTypeToUnsignedShort();
    imageShiftScale->SetShift(shift);
    imageShiftScale->Update();

    m_rangeMin = 0; m_rangeMax = static_cast<unsigned short>(max + shift);
    m_image = imageShiftScale->GetOutput(); m_image->Register(0);   // el register és necessari (comprovat)
    m_data = reinterpret_cast<unsigned short*>(m_image->GetPointData()->GetScalars()->GetVoidPointer(0));
    m_dataSize = m_image->GetPointData()->GetScalars()->GetSize();

    imageShiftScale->Delete();
}

void Experimental3DVolume::createVolumeRayCastFunctions()
{
    m_simpleVolumeRayCastFunction = vtkVolumeRayCastCompositeFunction::New();
    m_shaderVolumeRayCastFunction = vtkVolumeRayCastVoxelShaderCompositeFunction::New();
}

void Experimental3DVolume::createVoxelShaders()
{
    m_ambientVoxelShader = new AmbientVoxelShader2();
    m_ambientVoxelShader->setData(m_data, m_rangeMax);
    m_directIlluminationVoxelShader = new DirectIlluminationVoxelShader2();
    m_directIlluminationVoxelShader->setData(m_data, m_rangeMax);
    m_contourVoxelShader = new ContourVoxelShader();
    m_whiteVoxelShader = new WhiteVoxelShader();
    m_whiteVoxelShader->setData(m_data, m_rangeMax);
    m_celShadingVoxelShader = new CelShadingVoxelShader();
    m_obscuranceVoxelShader = new ObscuranceVoxelShader();
    m_obscuranceVoxelShader->setData(m_data, m_rangeMax);
    m_colorBleedingVoxelShader = new ColorBleedingVoxelShader();
    m_vmiVoxelShader2 = new VmiVoxelShader2();
    m_vmiVoxelShader2->setData(m_data, m_rangeMax, m_dataSize);
    m_vomiVoxelShader = new VomiVoxelShader();
    m_vomiVoxelShader->setData(m_data, m_rangeMax);
    m_vomiGammaVoxelShader = new VomiGammaVoxelShader();
    m_vomiGammaVoxelShader->setData(m_data, m_rangeMax);
    m_vomiCoolWarmVoxelShader = new VomiCoolWarmVoxelShader();
    m_voxelSaliencyVoxelShader = new VoxelSaliencyVoxelShader();
    m_voxelSaliencyVoxelShader->setData(m_data, m_rangeMax);
    m_colorVomiVoxelShader = new ColorVomiVoxelShader();
    m_opacityVoxelShader = new OpacityVoxelShader();
    m_imiVoxelShader = new ImiVoxelShader();
    m_imiVoxelShader->setData(m_data, m_rangeMax);
    m_coolWarmVoxelShader = new CoolWarmVoxelShader();
    m_coolWarmVoxelShader->setData(m_data, m_rangeMax);
    m_filteringAmbientOcclusionVoxelShader = new FilteringAmbientOcclusionVoxelShader();
    m_filteringAmbientOcclusionMapVoxelShader = new FilteringAmbientOcclusionMapVoxelShader();
    m_filteringAmbientOcclusionMapVoxelShader->setData(m_data, m_rangeMax);
    m_filteringAmbientOcclusionStipplingVoxelShader = new FilteringAmbientOcclusionStipplingVoxelShader();
    m_filteringAmbientOcclusionStipplingVoxelShader->setData(m_data, m_rangeMax);
}

void Experimental3DVolume::createMappers()
{
    m_cpuRayCastMapper = vtkVolumeRayCastMapper::New();
    m_cpuRayCastMapper->SetInputData(m_image);
    m_cpuRayCastMapper->SetVolumeRayCastFunction(m_simpleVolumeRayCastFunction);
    m_gpuRayCastMapper = vtkOpenGLGPUVolumeRayCastMapper::New();
    m_gpuRayCastMapper->SetInputData(m_image);
}

void Experimental3DVolume::createProperty()
{
    m_property = vtkVolumeProperty::New();
}

void Experimental3DVolume::createVolume()
{
    m_volume = vtkVolume::New();
    m_volume->SetMapper(m_gpuRayCastMapper);    // li hem d'assignar un mapper per poder saber el centre
    m_volume->SetProperty(m_property);

    // Centrem el volum a (0,0,0)
    double *center = m_volume->GetCenter();
    m_volume->AddPosition(-center[0], -center[1], -center[2]);
}

vtkEncodedGradientEstimator* Experimental3DVolume::gradientEstimator() const
{
    switch (m_gradientEstimator)
    {
        case FiniteDifference:
        default:
            return m_finiteDifferenceGradientEstimator;
        case FourDLInearRegression1:
        case FourDLInearRegression2:
            return m_4DLinearRegressionGradientEstimator;
    }
}

} // namespace udg
