#ifndef CUDAVIEWPOINTINFORMATIONCHANNEL_H
#define CUDAVIEWPOINTINFORMATIONCHANNEL_H

#include "vector3.h"

#include <QVector>

class vtkImageData;

namespace udg {
class Matrix4;
class TransferFunction;
}

using namespace udg;

#ifdef __cplusplus
extern "C" {
#endif

void cvicSetupRayCast(vtkImageData *image, const TransferFunction &transferFunction, int renderSize, int displaySize, QColor backgroundColor, bool display);
QVector<float> cvicRayCastAndGetHistogram(Vector3 viewpoint, Matrix4 viewMatrix);
void cvicCleanupRayCast();

void cvicSetupVoxelProbabilities();
void cvicAccumulateVoxelProbabilities(float viewProbability, float totalViewedVolume);
QVector<float> cvicGetVoxelProbabilities();
void cvicCleanupVoxelProbabilities();

void cvicSetupVomi(bool HVz, bool vomi, bool colorVomi);
void cvicAccumulateHVz(float viewProbability, float totalViewedVolume);
void cvicAccumulateVomi(float viewProbability, float totalViewedVolume);
void cvicAccumulateColorVomi(float viewProbability, const Vector3Float &viewColor, float totalViewedVolume);
QVector<float> cvicGetHVz();
QVector<float> cvicGetVomi();
QVector<Vector3Float> cvicGetColorVomi();
void cvicCleanupVomi();

QVector<float> cvicComputeVomi2(float HV, const QVector<float> &HVz);

void cvicSetupVomi3();
void cvicAccumulateVomi3(float viewProbability, float totalViewedVolume, float vmi2);
QVector<float> cvicGetVomi3();
void cvicCleanupVomi3();

#ifdef __cplusplus
}
#endif

#endif // CUDAVIEWPOINTINFORMATIONCHANNEL_H
