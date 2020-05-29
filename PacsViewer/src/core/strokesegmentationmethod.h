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

#ifndef UDGSTROKESEGMENTATIONMETHOD_H
#define UDGSTROKESEGMENTATIONMETHOD_H

#include "volume.h"

namespace udg {

class StrokeSegmentationMethod {

public:
    StrokeSegmentationMethod();

    ~StrokeSegmentationMethod();

    double applyMethod();
    double applyMethodVTK();
    void applyMethodVTKRecursive(vtkImageData *imMask, int a, int b, int c, int prof);

    /// Neteja els casos propers al crani
    double applyCleanSkullMethod();

    void applyFilter(Volume *output);

    void computeSpeedMap(itk::Image<float, 3> *speedMap);

    double erfc(double x);

    double applyMethodEdema(Volume *lesionMask);

    double applyMethodEdema2(Volume *lesionMask);

    int getNumberOfVoxels()
    {
        return m_cont;
    }

    int getEdemaNumberOfVoxels()
    {
        return m_edemaCont;
    }

    double applyVentriclesMethod();

    void applyMethod2();

    int applyMethod3();

    int applyMethod4();

    void setVolume(Volume *vol)
    {
        m_Volume = vol;
    }

    void setMask(Volume *vol)
    {
        m_Mask = vol;
    }

    void setSeedPosition(double x, double y, double z);

    void setHistogramLowerLevel(int x);

    void setHistogramUpperLevel(int x);

    void setInsideMaskValue(int x)
    {
        m_insideMaskValue = x;
    };

    void setOutsideMaskValue(int x)
    {
        m_outsideMaskValue = x;
    };

    void setInitialDistance(double x)
    {
        m_initialDistance = x;
    };

    void setTimeThreshold(int x)
    {
        m_timeThreshold = x;
    };

    void setStoppingTime(double x)
    {
        m_stoppingTime = x;
    };

    void setMultiplier(double x)
    {
        m_multiplier = x;
    };

    void setMean(double x)
    {
        m_mean = x;
    };

    void setVariance(double x)
    {
        m_variance = x;
    };

    void setConstant(double x)
    {
        m_constant = x;
    };

    void setAlpha(double x)
    {
        m_alpha = x;
    };

    void setLowerVentriclesThreshold(int x)
    {
        m_lowerVentriclesThreshold = x;
    };

    void setUpperVentriclesThreshold(int x)
    {
        m_upperVentriclesThreshold = x;
    };

private:
    /// Volum que volem segmentar
    Volume *m_Volume;

    /// Volum on guardarem el resultat de la segmetnació que volem segmentar
    Volume *m_Mask;
    Volume *m_filteredInputImage;

    /// Posició de la llavor
    double m_px, m_py, m_pz;

    /// Llindars de l'histograma
    int m_lowerThreshold, m_upperThreshold;

    /// Llindars de l'histograma
    int m_lowerVentriclesThreshold, m_upperVentriclesThreshold;

    /// Valors interns i externs de la màscara
    int m_insideMaskValue, m_outsideMaskValue;

    /// Valors interns i externs de la màscara
    double m_volume;
    int m_cont;
    int m_edemaCont;

    /// Valors mètode edema
    double m_initialDistance;
    int m_timeThreshold;
    double m_stoppingTime;
    double m_multiplier;
    double m_mean;
    double m_variance;
    double m_constant;
    double m_alpha;

    /// Retorna quants voxels != de 0 hi ha a la màscara
    int computeSizeMask();

};

}

#endif
