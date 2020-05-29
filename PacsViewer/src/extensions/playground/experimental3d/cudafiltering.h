#ifndef CUDAFILTERING_H
#define CUDAFILTERING_H

#include <QVector>

class vtkImageData;

#ifdef __cplusplus
extern "C" {
#endif

QVector<float> cfGaussianDifference(vtkImageData *image, int radius);
QVector<float> cfBoxMeanDifference(vtkImageData *image, int radius);

// Mètodes on calculem l'ambient occlusion com la probabilitat que un vòxel tingui un valor més baix que els del seu entorn (P(Z >= z))
// estimació de P(Z >= z) amb Chebychev (variant)
QVector<float> cfProbabilisticAmbientOcclusionGaussianChebychev(vtkImageData *image, int radius);
// estimació de P(Z >= z) amb Chebychev (bona)
QVector<float> cfProbabilisticAmbientOcclusionBoxMeanChebychev(vtkImageData *image, int radius);
// estimació de P(Z >= z) amb una gaussiana
QVector<float> cfProbabilisticAmbientOcclusionGaussian(vtkImageData *image, int radius);
// estimació de P(Z >= z) amb la freqüència de Z >= z en un cub
QVector<float> cfProbabilisticAmbientOcclusionCube(vtkImageData *image, int radius);
// estimació de P(Z >= z) amb la freqüència de Z >= z en una esfera
QVector<float> cfProbabilisticAmbientOcclusionSphere(vtkImageData *image, int radius);
// estimació de P(Z >= z) amb Chebychev i cub tangent (László)
QVector<float> cfProbabilisticAmbientOcclusionTangentSphereVariance(vtkImageData *image, int radius);
// estimació de P(Z >= z) amb el mètode del cub tangent (László)
QVector<float> cfProbabilisticAmbientOcclusionTangentSphereCdf(vtkImageData *image, int radius);
// estimació de P(Z >= z) amb la cdf d'una gaussiana
QVector<float> cfProbabilisticAmbientOcclusionTangentSphereGaussian(vtkImageData *image, int radius);

// Variància del volum
QVector<float> cfVolumeVariance(vtkImageData *image, int radius);

#ifdef __cplusplus
}
#endif

#endif // CUDAFILTERING_H
