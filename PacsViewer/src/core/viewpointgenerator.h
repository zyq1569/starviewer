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

#ifndef UDGVIEWPOINTGENERATOR_H
#define UDGVIEWPOINTGENERATOR_H

#include <QVector>

#include "vector3.h"

namespace udg {

/**
    Un generador de punts de vista distribuïts uniformement o quasi-uniforme.
  */
class ViewpointGenerator {

public:
    /// Retorna un vector cap amunt per al punt de vista passat.
    static Vector3 up(const Vector3 &viewpoint);

    /// Crea el generador sense que generi cap punt de vista.
    ViewpointGenerator();
    ~ViewpointGenerator();

    /// Genera 4 punts de vista distribuïts uniformement.
    void setToUniform4(float radius = 1.0);
    /// Genera 8 punts de vista distribuïts uniformement.
    void setToUniform6(float radius = 1.0);
    /// Genera 12 punts de vista distribuïts uniformement.
    void setToUniform8(float radius = 1.0);
    /// Genera 16 punts de vista distribuïts uniformement.
    void setToUniform12(float radius = 1.0);
    /// Genera 20 punts de vista distribuïts uniformement.
    void setToUniform20(float radius = 1.0);
    /// Genera 10*4^depth+2 punts de vista distribuïts quasi-uniformement.
    void setToQuasiUniform(unsigned char depth, float radius = 1.0);
    /// Retorna els punts de vista generats.
    QVector<Vector3> viewpoints() const;
    /// Retorna el punt de vista \a i d'entre els generats.
    Vector3 viewpoint(int i) const;
    /// Retorna els índex dels veïns del punt amb índex \a i.
    QVector<int> neighbours(int i) const;

private:
    QVector<Vector3> m_viewpoints;
    QVector<QVector<int> > m_neighbours;

};

}

#endif
