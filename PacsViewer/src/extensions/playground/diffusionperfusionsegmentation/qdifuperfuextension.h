/***************************************************************************
 *   Copyright (C) 2007 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/

#ifndef UDGQDIFFUSIONPERFUSIONSEGMENTATIONEXTENSION_H
#define UDGQDIFFUSIONPERFUSIONSEGMENTATIONEXTENSION_H

#include "ui_qdifuperfuextensionbase.h"

#include "itkRegistre3DAffine.h"
#include "volume.h"

// Forward declarations
class vtkActor;
class vtkImageActor;
class vtkLookupTable;

namespace udg {

// Forward declarations
class StrokeSegmentationMethod;
class ToolManager;

/**
 * TODO
 *
 * Clic per posar la llavor no funciona sempre a la primera
 *
 * Les llesques canvien de 2 en 2 (a vegades)
 *
 * Queda un quadrat verd de l'ajustador de màscara
 *
 * Que es mantingui obert el mateix DICOMDIR?????????????????
 *  - Fet (a mitges)
 *
 * Es pot posar llavor a perfu
 */

/**
 * Extensió que permet segmentar una lesió i calcular-ne el volum a partir de les imatges de difusió
 * i perfusió.
 *
 * @author Grup de Gràfics de Girona (GGG) <vismed@ima.udg.es>
*/
class QDifuPerfuSegmentationExtension : public QWidget, private ::Ui::QDifuPerfuExtensionBase {
Q_OBJECT

public:
    QDifuPerfuSegmentationExtension(QWidget * parent = 0);
    ~QDifuPerfuSegmentationExtension();

public slots:
    /// Li assigna el volum principal
    void setDiffusionInput(Volume * input);
    void setPerfusionInput(Volume * input);

    /// Executa l'algorisme de segmentació
    void applyStrokeSegmentation();
    void applyVentriclesMethod();
    void applyRegistration();
    void computeBlackpointEstimation();
    void applyPenombraSegmentation();

private slots:
    void setMaxDiffusionImage(int max);
    void setDiffusionImage(int index);
    void setMaxPerfusionImage(int max);
    void setPerfusionImage(int index);

    /// actualitza el valor llindar baix
    void setStrokeLowerValue(int x);
    /// actualitza el valor llindar alt
    void setStrokeUpperValue(int x);

    /// Visualitza la màscara donats uns thresholds
    void viewThresholds();
    void viewThresholds2();

    /// visualitza la informació de la llavor del mètode de segmentació
    void setSeedPosition(double x, double y, double z);

    /// Diversos mètodes per moure l'splitter
    void moveViewerSplitterToLeft();
    void moveViewerSplitterToRight();
    void moveViewerSplitterToCenter();

    /// Refresca el resultat del volum
    //void updateStrokeVolume();

    void applyFilterDiffusionImage();

    ///Pinta el mapa de colors segons un threshold determinat
    void setPerfusionLut(int threshold);

    /// Canvia la opacitat de la màscara (difusió)
    void setDiffusionOpacity(int opacity);

    /// Canvia la opacitat de la màscara (perfusió)
    void setPerfusionOpacity(int opacity);

    /// Visualitza els diferents overlays
    void viewLesionOverlay();
    void viewVentriclesOverlay();

    // [temporal] el farem servir mentre no s'actualitzi la tècnica d'overlay del Q2DViewer
    void setPerfusionSlice(int slice);
    void synchronizeSlices(bool sync);

    ///Calcula la diferència de volums
    void computePenombraVolume();

    ///Desa la màscara de la difusió
    void saveDiffusionVolume();
    void saveDiffusionMask();
    ///Desa la màscara i diferents volums de la perfusió
    void savePerfusionVolume();
    void saveRegisteredPerfusionVolume();
    void savePerfusionMask();
    ///Desa la transformada entre la perfusió i la difusió
    void saveTransform();

    ///Actualitza els volums de les màscares de la perfusió i la difusió
    void updateStrokeVolume();
    void updatePenombraVolume();

private:
    /// inicialitza les tools
    void initializeTools();

    /// Crea les accions
    /// \TODO 'pujar' al pare com a mètode virtual com a Extensions? [hauria de ser protected]
    void createActions();

    /// Crea les connexions entre signals i slots
    void createConnections();

    /// Carrega la configuració
    void readSettings();

    /// Desa la configuració
    void writeSettings();

    /// Calcula el volum de la màscara d'stroke
    double calculateStrokeVolume();

    ///Calcula els volums de les màscares de la perfusió i la difusió
    double calculateDiffusionMaskVolume();
    double calculatePerfusionMaskVolume();

private:
    typedef Volume::ItkImageType ItkImageType;
    typedef itkRegistre3DAffine< ItkImageType, ItkImageType >::TransformType TransformType;

    /// Registration parameters
    static const double RegistrationFixedStandardDeviation;
    static const double RegistrationMovingStandardDeviation;
    static const int RegistrationNumberOfSpacialSamples;
    static const int RegistrationFixedVariance;
    static const int RegistrationMovingVariance;
    static const int RegistrationMaximumStep;
    static const double RegistrationMinimumStep;
    static const int RegistrationNumberOfIterations;

    /// Volum d'entrada de difusió
    Volume *m_diffusionInputVolume;

    /// Volum d'entrada de perfusió
    Volume *m_perfusionInputVolume;

    /// Volum principal de difusió
    Volume *m_diffusionMainVolume;

    /// Volum principal de perfusió
    Volume *m_perfusionMainVolume;

    /// Volum de difusió amb valors entre 0 i 255
    Volume *m_diffusionRescaledVolume;

    /// Volum de perfusió amb valors entre 0 i 255
    Volume *m_perfusionRescaledVolume;

    Volume *m_activedMaskVolume;
    Volume *m_strokeMaskVolume;
    Volume *m_ventriclesMaskVolume;

    Volume *m_blackpointEstimatedVolume;
    Volume *m_penombraMaskVolume;

    ItkImageType::PixelType m_diffusionMinValue, m_diffusionMaxValue;
    ItkImageType::PixelType m_perfusionMinValue, m_perfusionMaxValue;
    ItkImageType::PixelType m_penombraMaskMinValue, m_penombraMaskMaxValue;

    vtkImageActor *m_perfusionOverlay;
    vtkLookupTable *m_perfusionHueLut;

    /// Stroke segmentation
    StrokeSegmentationMethod *m_strokeSegmentationMethod;
    int m_strokeCont;
    double m_strokeVolume;

    /// Registration
    TransformType::Pointer m_registerTransform;

    /// Penombra segmentation
    int m_penombraCont;
    double m_penombraVolume;

    double m_seedPosition[3];

    /// Accions
    QAction *m_lesionViewAction;
    QAction *m_ventriclesViewAction;

    /// Grups de botons en format exclusiu
    QActionGroup *m_viewOverlayActionGroup;

    ///Directori on guardem les màscares
    QString m_savingMaskDirectory;

    /// ToolManager
    ToolManager *m_toolManager;
};

} // end namespace udg



#endif
