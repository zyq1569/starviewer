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

#ifndef UDGQVIEWERWORKINPROGRESSWIDGET_H
#define UDGQVIEWERWORKINPROGRESSWIDGET_H

#include "ui_qviewerworkinprogresswidget.h"

class QMovie;

namespace udg {

/**
     Widget that displays the progress of any task that may be in a QViewer.
     For example, the progress of loading an asynchronous volume, downloading previews ...
     In the event that any of these tasks fail, it gives the option to display these errors in it
     widget.
  */

class QViewerWorkInProgressWidget : public QWidget, private Ui::QViewerWorkInProgressWidget {
    Q_OBJECT
public:
    explicit QViewerWorkInProgressWidget(QWidget *parent = 0);

    /// Specifies the title given to the task being performed
    void setTitle (const QString & text);

    /// Show the error that happens to you. Useful for showing the user if an error occurs while doing the "work in progress".
    void showError (const QString & errorText);

    /// Restart the widget.
    void reset ();

public slots:
    /// Update work in progress.
    void updateProgress(int progress);

protected:
    /// To prevent the animation from consuming resources when it is not needed, we only activate it when the widget is visible,
    /// disabling it if it is not.
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);

private:
    /// Reseteja els widgets involucrats en mostrar el progrés
    void resetProgressWidgets();

private:
    QMovie *m_progressBarAnimation;
};

} // End namespace udg

#endif // UDGQVIEWERWORKINPROGRESSWIDGET_H
