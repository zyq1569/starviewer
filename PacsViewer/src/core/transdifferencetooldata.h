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

#ifndef UDGTRANSDIFFERENCETOOLDATA_H
#define UDGTRANSDIFFERENCETOOLDATA_H

#include "tooldata.h"
#include <QPair>
#include <QVector>

namespace udg {

class Q2DViewer;
class Volume;

/**
    Classe per guardar les dades de la tool de diferència amb translació.
  */
class TransDifferenceToolData : public ToolData {
Q_OBJECT
public:
    TransDifferenceToolData();
    ~TransDifferenceToolData();

    /// Set del volum d'entrada
    void setInputVolume(Volume *input);

    /// Get del volum d'entrada
    Volume* getInputVolume()
    {
        return m_inputVolume;
    }

    /// Set del volum diferència
    void setDifferenceVolume(Volume *input);

    /// Get del volum diferència
    Volume* getDifferenceVolume()
    {
        return m_differenceVolume;
    }

    /// Get X de la translacio
    int getSliceTranslationX(int sl)
    {
        return m_sliceTranslations[sl].first;
    }

    /// Set X de la translacio
    void setSliceTranslationX(int sl, int trX)
    {
        m_sliceTranslations[sl].first = trX;
    }

    /// Increase X de la translacio
    void increaseSliceTranslationX(int sl, int trX)
    {
        m_sliceTranslations[sl].first += trX;
    }

    /// Get Y de la translacio
    int getSliceTranslationY(int sl)
    {
        return m_sliceTranslations[sl].second;
    }

    /// Set Y de la translacio
    void setSliceTranslationY(int sl, int trY)
    {
        m_sliceTranslations[sl].second = trY;
    }

    /// Increase Y de la translacio
    void increaseSliceTranslationY(int sl, int trY)
    {
        m_sliceTranslations[sl].second += trY;
    }

    /// Get la llesca de referència
    int getReferenceSlice()
    {
        return m_referenceSlice;
    }

    /// Set la llesca de referència
    void setReferenceSlice(int sl)
    {
        m_referenceSlice = sl;
    }

    void setActualDisplacement(int dx, int dy)
    {
        emit actualDisplacement(dx, dy);
    }

    void setFinalDisplacement(int dx, int dy)
    {
        emit finalDisplacement(dx, dy);
    }

public slots:
signals:
    /// Envia el desplaçament que s'ha fet des de la posició d'origen
    void actualDisplacement(int, int);

    /// Envia el desplaçament que s'ha fet des de la posició d'origen
    void finalDisplacement(int, int);

private:
    /// Dades de les transformacions aplicades a cada llesca
    QVector<QPair<int, int> > m_sliceTranslations;

    /// Dades del volum original i la diferència
    Volume *m_inputVolume;
    Volume *m_differenceVolume;

    int m_dx, m_dy;

    /// Slice de referència
    int m_referenceSlice;
};

}

#endif
