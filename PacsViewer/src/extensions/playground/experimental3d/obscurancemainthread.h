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

#ifndef UDGOBSCURANCEMAINTHREAD_H
#define UDGOBSCURANCEMAINTHREAD_H

#include <QThread>

#include <QVector>

#include "obscurance.h"
#include "transferfunction.h"

class vtkVolume;

namespace udg {

/**
    Thread principal per al càlcul d'obscurances. Controla els altres threads.
  */
class ObscuranceMainThread : public QThread {
Q_OBJECT

public:
    /// Estructura que guarda les coordenades d'un vòxel.
    struct Voxel { int x, y, z; };
    /// Funcions d'obscurances.
    enum Function { Constant0, Distance, SquareRoot, Exponential, ExponentialNorm, CubeRoot };
    /// Variants de les obscurances.
    enum Variant { Density, DensitySmooth, Opacity, OpacitySmooth, OpacitySaliency, OpacitySmoothSaliency, OpacityColorBleeding, OpacitySmoothColorBleeding };

    static bool hasColor(Variant variant);

    ObscuranceMainThread(int numberOfDirections, double maximumDistance, Function function, Variant variant, bool doublePrecision = true, QObject *parent = 0);
    virtual ~ObscuranceMainThread();

    bool hasColor() const;
    void setVolume(vtkVolume *volume);
    void setTransferFunction(const TransferFunction &transferFunction);
    void setSaliency(const double *saliency, double fxSaliencyA, double fxSaliencyB, double fxSaliencyLow, double fxSaliencyHigh);

    Obscurance* getObscurance() const;

public slots:
    void stop();

signals:
    void progress(int percent);
    void computed();

protected:
    virtual void run();

private:
    static void getLineStarts(QVector<Vector3> &lineStarts, int dimX, int dimY, int dimZ, const Vector3 &forward);
    QVector<Vector3> getDirections() const;

private:
    int m_numberOfDirections;
    double m_maximumDistance;
    Function m_function;
    Variant m_variant;
    bool m_doublePrecision;
    vtkVolume *m_volume;
    TransferFunction m_transferFunction;
    Obscurance *m_obscurance;
    const double *m_saliency;
    double m_fxSaliencyA, m_fxSaliencyB;
    double m_fxSaliencyLow, m_fxSaliencyHigh;

    bool m_stopped;

};

}

#endif
