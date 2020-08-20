/*************************************************************************************
  Copyright (C) 2014 Laboratori de Gràfics i Imatge, Universitat de Girona &
  Institut de Diagnòstic per la Imatge.
  Girona 2014. All rights reserved.
  http://starviewer.udg.edu

  This file is part of the Starviewer (Medical Imaging Software) open source project.
  It is subject to the license terms in the LICENSE file found in the top-level
  directory of this distribution and at http://starviewer.udg.edu/license. No part of
  the Starviewer (Medical Imaging Software) open source project, including this file,
  may be copied, modified, propagated, or distributed except according to the
  terms contained in the LICENSE file.
 *************************************************************************************/

#ifndef UDGOBSCURANCETHREAD_H
#define UDGOBSCURANCETHREAD_H

#include <QThread>
#include <QVector>

#include "obscurancemainthread.h"
#include "transferfunction.h"
#include "vector3.h"

class vtkDirectionEncoder;
class vtkEncodedGradientEstimator;

namespace udg {

class Obscurance;

/**
    Thread que implementa els mètodes de càlcul d'obscurances.

    \author Grup de Gràfics de Girona (GGG) <vismed@ima.udg.edu>
  */
class ObscuranceThread : public QThread {
Q_OBJECT

public:
    ObscuranceThread(int id, int numberOfThreads, const TransferFunction &transferFunction, QObject *parent = 0);
    virtual ~ObscuranceThread();

    /// Assigna l'estimador del gradient, d'on es treuran les normals.
    void setGradientEstimator(vtkEncodedGradientEstimator *gradientEstimator);
    void setData(const ushort *data, int dataSize, const int dimensions[3], const int increments[3]);
    void setObscuranceParameters(double obscuranceMaximumDistance, ObscuranceMainThread::Function obscuranceFunction,
                                 ObscuranceMainThread::Variant obscuranceVariant, Obscurance *obscurance);
    void setSaliency(const double *saliency, double fxSaliencyA, double fxSaliencyB, double fxSaliencyLow, double fxSaliencyHigh);
    void setPerDirectionParameters(const Vector3 &direction, const Vector3 &forward, const int xyz[3], const int sXYZ[3],
                                   const QVector<Vector3> &lineStarts, qptrdiff startDelta);

protected:
    virtual void run();

private:
    typedef ObscuranceMainThread::Voxel Voxel;
    typedef ObscuranceMainThread::Function Function;
    typedef ObscuranceMainThread::Variant Variant;

    void runDensity();
    void runDensitySmooth();
    void runOpacity();
    void runOpacitySmooth();
    void runOpacitySaliency();
    void runOpacitySmoothSaliency();
    void runOpacityColorBleeding();
    void runOpacitySmoothColorBleeding();
    double obscurance(double distance) const;
    bool smoothBlocking(const Vector3 &blocking, const Vector3 &blocked, double distance, const float *blockedGradient) const;

    int m_id, m_numberOfThreads;
    const TransferFunction &m_transferFunction;
    vtkDirectionEncoder *m_directionEncoder;
    const ushort *m_encodedNormals;
    const ushort *m_data;
    int m_dataSize;
    const int *m_dimensions;
    const int *m_increments;
    double m_obscuranceMaximumDistance;
    Function m_obscuranceFunction;
    Variant m_obscuranceVariant;
    Obscurance *m_obscurance;
    const double *m_saliency;
    double m_fxSaliencyA, m_fxSaliencyB;
    double m_fxSaliencyLow, m_fxSaliencyHigh;

    Vector3 m_direction, m_forward;
    const int *m_xyz;
    const int *m_sXYZ;
    QVector<Vector3> m_lineStarts;
    qptrdiff m_startDelta;

};

}

#endif
