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

#include "statswatcher.h"
#include "logging.h"
#include "coresettings.h"

#include <QAbstractButton>
#include <QAbstractSlider>
#include <QAction>
#include <QMenu>

namespace udg {

StatsWatcher::StatsWatcher(QObject *parent)
 : QObject(parent), m_context("*")
{
    Settings settings;
    m_registerLogs = settings.getValue(CoreSettings::RegisterStatLogs).toBool();
}

StatsWatcher::StatsWatcher(const QString &context, QObject *parent)
 : QObject(parent), m_context(context)
{
    Settings settings;
    m_registerLogs = settings.getValue(CoreSettings::RegisterStatLogs).toBool();
}

StatsWatcher::~StatsWatcher()
{
}

void StatsWatcher::addClicksCounter(QAbstractButton *button)
{
    if (m_registerLogs)
    {
        connect(button, SIGNAL(clicked(bool)), SLOT(registerClick(bool)));
    }
}

void StatsWatcher::addTriggerCounter(QMenu *menu)
{
    if (m_registerLogs)
    {
        connect(menu, SIGNAL(triggered(QAction*)), SLOT(registerActionTrigger(QAction*)));
    }
}

void StatsWatcher::log(const QString &message)
{
    Settings settings;
    if (settings.getValue(CoreSettings::RegisterStatLogs).toBool())
    {
        INFO_LOG(QString("STAT: ") + message);
    }
}

void StatsWatcher::addSliderObserver(QAbstractSlider *slider)
{
    if (m_registerLogs)
    {
        connect(slider, SIGNAL(actionTriggered(int)), SLOT(registerSliderAction(int)));
        connect(slider, SIGNAL(sliderReleased()), SLOT(registerSliderAction()));
    }
}

void StatsWatcher::registerClick(bool checked)
{
    QString statMessage;
    QAbstractButton *button = qobject_cast<QAbstractButton*>(sender());
    if (button)
    {
        if (button->isCheckable())
        {
            if (checked)
            {
                statMessage = "S'ha activat amb un click el botó";
            }
            else
            {
                statMessage = "S'ha desactivat amb un click el botó";
            }
        }
        else
        {
            statMessage = "Sha fet un click sobre el botó";
        }
    }
    // És un altre tipus d'objecte
    else
    {
        statMessage = "S'ha fet un click sobre l'objecte";
    }

    statMessage = QString("[%1] %2 %3").arg(m_context).arg(statMessage).arg(sender()->objectName());
    log(statMessage);
}

void StatsWatcher::registerActionTrigger(QAction *action)
{
    log(QString("[%1] S'ha disparat l'acció [%2], objecte [%3]").arg(m_context).arg(action->text()).arg(action->objectName()));
}

void StatsWatcher::registerSliderAction(int action)
{
    QAbstractSlider *slider = qobject_cast<QAbstractSlider*>(sender());
    if (!slider)
    {
        DEBUG_LOG("Aquest slot només s'hauria de connectar amb sliders. Retornem de la funció");
        return;
    }
    QString statMessage;
    switch (action)
    {
        case QAbstractSlider::SliderNoAction:
            statMessage = "No Action";
            break;

        case QAbstractSlider::SliderSingleStepAdd:
            statMessage = "+1 Pas";
            break;

        case QAbstractSlider::SliderSingleStepSub:
            statMessage = "-1 Pas";
            break;

        case QAbstractSlider::SliderPageStepAdd:
            statMessage = "+1 Page";
            break;

        case QAbstractSlider::SliderPageStepSub:
            statMessage = "-1 Page";
            break;

        case QAbstractSlider::SliderToMinimum:
            statMessage = "A mínim";
            break;

        case QAbstractSlider::SliderToMaximum:
            statMessage = "A màxim";
            break;

        case QAbstractSlider::SliderMove:
            // Només registrem quan s'ha fet un move amb rodeta
            if (!slider->isSliderDown())
            {
                statMessage = "Desplaçar amb rodeta";
            }
            break;

            // Cas especial. petit hack per connectar amb sliderReleased
        case 10:
            statMessage = "Desplaçar clickant";
            break;
    }

    if (!statMessage.isEmpty())
    {
        log(QString("[%1] S'ha fet l'acció [%2] sobre l'slider [%3]").arg(m_context).arg(statMessage).arg(sender()->objectName()));
    }
}

} // End namespace udg
