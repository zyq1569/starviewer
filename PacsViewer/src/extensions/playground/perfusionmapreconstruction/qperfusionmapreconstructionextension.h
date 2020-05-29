/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGQPERFUSIONMAPRECONSTRUCTION_H
#define UDGQPERFUSIONMAPRECONSTRUCTION_H

#include "ui_qperfusionmapreconstructionextensionbase.h"

#include <itkImage.h>
#include <volume.h>

#include <QString>
#include <QVector>

// FWD declarations
class QAction;
class vtkImageMask;
class vtkActor;
class vtkUnsignedCharArray;

namespace udg {

// FWD declarations
class Volume;
class ToolManager;
class DrawerPoint;
class PerfusionMapCalculatorMainThread;
class SeedToolData;

/**
    @author Grup de Gràfics de Girona (GGG) <vismed@ima.udg.es>
*/
class QPerfusionMapReconstructionExtension : public QWidget , private Ui::QPerfusionMapReconstructionExtensionBase {
Q_OBJECT
public:
    QPerfusionMapReconstructionExtension(QWidget *parent = 0);
    ~QPerfusionMapReconstructionExtension();

public slots:
    /// Li assigna el volum principal
    void setInput(Volume *input);

private:
    typedef itk::Image<bool, 3> BoolImageType;
    typedef itk::Image<double, 3> DoubleImageType;
    typedef itk::Image<double, 4> DoubleTemporalImageType;
    static const double TE;
    static const double TR;

    void initializeTools();

    /// Crea les connexions entre signals i slots
    void createConnections();

    /// Guarda i llegeix les característiques
    void readSettings();
    void writeSettings();
    bool findProbableSeries();

    void createColorMap();
    void createColorMap2();

private slots:

    void computePerfusionMap();
    void paintMap();
    void changeMap(int value);

     /// gestiona els events del m_2DView
    void eventHandler(unsigned long id);

    ///connecta amb el canvi de window level
    void createColorMap(double window, double level);
    void createColorMap(DoubleImageType::Pointer image, Q2DViewer* viewer);
    void createColorMap(Volume::ItkImageType::Pointer image, Q2DViewer* viewer);
    void getPerfusionColormapTable(vtkUnsignedCharArray * table);

    ///aplica un filtre al mapa
    void applyFilterMapImage();

    /// gestiona els events del moviment del cursor
    void paintCursorSignal();

    /// pinta senyal temporal si la llavor està activada
    void paintAIFSignal();

    /// pinta la mitjana del deltaR de cada llesca
    void paintMeanSlice(int slice);

    /// pinta les dades de la ROI
    void paintROIData();

    /// gestiona els events del botó esquerre
    void leftButtonEventHandler();

    /// desactiva el booleà que ens diu si està el botó esquerra apretat
    void setLeftButtonOff();

    /// Visualitza el menú que permet escollir la sèrie del DSC
    void contextMenuDSCRelease();
    void contextMenuEvent(QContextMenuEvent *event);
    void setVolume(Volume *volume);

private:
    /// El volum principal
    Volume *m_mainVolume;
    Volume *m_DSCVolume;

    bool m_isLeftButtonPressed;

    int m_minValue, m_maxValue;
    int m_insideValue, m_outsideValue;
    int m_mapMax, m_mapMin;

    /// Tool manager
    ToolManager *m_toolManager;

    /// Dades de la seed Tool per saber quin és l'arterial input function
    SeedToolData *m_seedToolData;
    
    ///Calculadora de mapes de perfusió
    PerfusionMapCalculatorMainThread* m_mapCalculator;

    DrawerPoint* m_aifDrawPoint;
    int m_aifIndex[3];
    int m_aifSlice;

    ///Valors de l'echo time i repetition time
    double m_newTR;
    double m_newTE;

    //DeltaR mitjana per cada llesca
    QVector<QVector<double> > m_meanseries;
};

} // end namespace udg

#endif
