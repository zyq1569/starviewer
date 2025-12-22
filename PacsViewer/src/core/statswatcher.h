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

/**
Widget Viewer for statistics
  EVERYTHING needs to be added a setting that tells us whether statistics are allowed or not
  and depending on their value record or not the statistics
*/

#ifndef UDGSTATSWATCHER_H
#define UDGSTATSWATCHER_H

#include <QObject>

// Fwd declarations
class QAbstractButton;
class QAbstractSlider;
class QAction;
class QMenu;

namespace udg {
class StatsWatcher : public QObject {
    Q_OBJECT
public:
    StatsWatcher(QObject *parent = 0);
    StatsWatcher(const QString &context, QObject *parent = 0);
    ~StatsWatcher();

    /// Click counter. For each click of the button it will tell us the name of the object
    void addClicksCounter(QAbstractButton *button);

    /// Count when an action has been fired
    // Count when shooting, either with a click or a shortcut
    void addTriggerCounter(QMenu *menu);

    /// Records actions performed on a slider
    void addSliderObserver(QAbstractSlider *slider);

    /// Method for logging statistical messages in the corresponding format
    static void log(const QString &message);

private slots:
    /// Log in the log on which the object was clicked
    /// Checks if the object is "checkable" (type QAbstractButton)
    /// Depending on this, a different message is recorded in order to differentiate
    /// clickable only objects and objects that can be activated or deactivated
    void registerClick(bool checked);

    ///Records the triggering of a QAction
    void registerActionTrigger(QAction *action);

    /// Records the action performed on a slider
    void registerSliderAction(int action = 10);

private:
    /// Add additional information about the context we are making the observation
    QString m_context;

    ///Indicates whether statistics logs will be logged or not
    bool m_registerLogs;
};

} // End namespace udg

#endif
