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

#ifndef UDGQ3DVIEWEREXTENSION_H
#define UDGQ3DVIEWEREXTENSION_H

#include "ui_q3dviewerextensionbase.h"

#include <QDir>
#include <QMap>

#include "transferfunction.h"

class QAction;
class QModelIndex;
class QStandardItemModel;

namespace udg {

class Q3DViewer;
class Volume;
class ToolManager;

/**
 * ...
*/
class Q3DViewerExtension : public QWidget, private ::Ui::Q3DViewerExtensionBase {
Q_OBJECT
public:
    Q3DViewerExtension(QWidget *parent = 0);
    virtual ~Q3DViewerExtension();

    /// We give it input to process
    void setInput(Volume *input);

public slots:
    ///Applies a predefined transfer function to the display.
    void applyPresetClut(const QString &clutName);

    /// Applies the last transfer function to the display.
    void applyClut(const TransferFunction &clut, bool preset = false);

    void setScalarRange(double min, double max);

    void updateUiForBlendMode(int blendModeIndex);

    /// Refreshes the view according to all current interface settings.
    void updateView(bool fast = true);

private:
    /// Tune in the tools that can be used in the extension
    void initializeTools();

    /// Gets the list of predefined transfer functions.
    void loadClutPresets();

    ///Load predefined rendering styles.
    void loadRenderingStyles();

    ///Establishes signal and slot connections
    void createConnections();

    /// Causes updateView () to be called when it changes any element of the interface.
    void enableAutoUpdate();
    /// Makes it not called updateView () when it changes any interface element.
    void disableAutoUpdate();

private slots:
    void removeBed();
private slots:
    void render();
    void loadClut();
    void saveClut();
    void switchEditor();
    void applyEditorClut();
    void toggleClutEditor();
    void hideClutEditor();
    /// Changes the status of the custom style button depending on whether or not the editor is being displayed at that time.
    void setCustomStyleButtonStateBySplitter();
    void changeViewerTransferFunction();
    void applyRenderingStyle(const QModelIndex &index);
    void showScreenshotsExporterDialog();

private:
    /// The input volume
    Volume *m_input;

    /// GE is Tor's tools
    ToolManager *m_toolManager;

    /// Directory of predefined transfer functions.
    QDir m_clutsDir;
    ///Map between transfer function names and the corresponding file name.
    QMap<QString, QString> m_clutNameToFileName;

    ///Last applied clut.
    TransferFunction m_currentClut;

    ///It will be true before entering the first input.
    bool m_firstInput;

    /// Model that saves rendering styles.
    QStandardItemModel *m_renderingStyleModel;

    /// Timer used to make quality renderings after a quick render.
    QTimer *m_timer;

    //
    bool m_firstRemoveBed;
    vtkImageData *m_saveVtkdata;
};

} // end namespace udg

#endif
