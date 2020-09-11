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

#include "patientbrowsermenu.h"

#include "patient.h"
#include "patientbrowsermenuextendedinfo.h"
#include "patientbrowsermenuextendeditem.h"
#include "patientbrowsermenulist.h"
#include "series.h"
#include "image.h"
#include "volume.h"
#include "volumerepository.h"
#include "screenmanager.h"

#include "patientbrowsermenugroup.h"

#include <QPainter>
#include <QVBoxLayout>

namespace udg {

PatientBrowserMenu::PatientBrowserMenu(QWidget *parent)
    : QWidget(parent), m_patientBrowserList(0), m_patientAdditionalInfo(0)
{
    // Initially we do not know on which screen the menu can be displayed
    m_currentScreenID = m_leftScreenID = m_rightScreenID = -1;

    m_showFusionOptions = false;
}

PatientBrowserMenu::~PatientBrowserMenu()
{
}

void PatientBrowserMenu::setPatient(Patient *patient)
{
    createWidgets();
    QString caption;
    QString label;
    
    foreach (Study *study, patient->getStudies())
    {
        // We extract the caption from the study
        caption = tr("Study %1 %2 [%3] %4")
                .arg(study->getDate().toString(Qt::ISODate))
                .arg(study->getTimeAsString())
                .arg(study->getModalitiesAsSingleString())
                .arg(study->getDescription());

        // For each series of the study we will extract its label and identifier
        QList<QPair<QString, QString> > itemsList;
        QList<QPair<QString, QString> > fusionItemsList;
        foreach (Series *series, study->getViewableSeries())
        {
            label = tr(" Series %1: %2 %3 %4 %5")
                    .arg(series->getSeriesNumber().trimmed())
                    .arg(series->getProtocolName().trimmed())
                    .arg(series->getDescription().trimmed())
                    .arg(series->getBodyPartExamined())
                    .arg(series->getViewPosition());

            int volumeNumber = 1;
            foreach (Volume *volume, series->getVolumesList())
            {
                QPair<QString, QString> itemPair;
                // Label
                if (series->getNumberOfVolumes() > 1)
                {
                    itemPair.first = label + " (" + QString::number(volumeNumber) + ")";
                }
                else
                {
                    itemPair.first = label;
                }

                volumeNumber++;
                // Identifier
                itemPair.second = QString::number(volume->getIdentifier().getValue());
                // We add the pair to the list
                itemsList << itemPair;

                // Look for fusion pairs
                if (m_showFusionOptions)
                {
                    if (series->getModality() == "CT" && !series->isCTLocalizer())
                    {
                        AnatomicalPlane acquisitionPlane = volume->getAcquisitionPlane();

                        if (acquisitionPlane != AnatomicalPlane::NotAvailable)
                        {
                            int zIndex = volume->getCorrespondingOrthogonalPlane(acquisitionPlane).getZIndex();
                            double margin = series->getImages().first()->getSliceThickness() * 5;

                            double range1[2];
                            range1[0] = volume->getImages().first()->getImagePositionPatient()[zIndex];
                            range1[1] = volume->getImages().last()->getImagePositionPatient()[zIndex];
                            
                            foreach (Series * secondSeries, study->getViewableSeries())
                            {
                                if ((secondSeries->getModality() == "PT" || secondSeries->getModality() == "NM") && series->getFrameOfReferenceUID() == secondSeries->getFrameOfReferenceUID())
                                {
                                    foreach (Volume *secondVolume, secondSeries->getVolumesList())
                                    {
                                        if (secondVolume->getAcquisitionPlane() == acquisitionPlane)
                                        {
                                            double range2[2];
                                            range2[0] = secondVolume->getImages().first()->getImagePositionPatient()[zIndex];
                                            range2[1] = secondVolume->getImages().last()->getImagePositionPatient()[zIndex];
                                            
                                            if ((range1[0] + margin > range2[0] && range1[1] - margin < range2[1]) || (range2[0] + margin > range1[0] && range2[1] - margin < range1[1]))
                                            {
                                                QPair<QString, QString> itemPair;
                                                // Label
                                                itemPair.first = QString("%1 + %2").arg(series->getProtocolName().trimmed() + series->getDescription().trimmed()).arg(secondSeries->getProtocolName().trimmed() + secondSeries->getDescription().trimmed());
                                                // Identifier
                                                itemPair.second = QString("%1+%2").arg(volume->getIdentifier().getValue()).arg(secondVolume->getIdentifier().getValue());
                                                //We add the pair to the list
                                                fusionItemsList << itemPair;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        // We add the series grouped by study
        m_patientBrowserList->addItemsGroup(caption, itemsList << fusionItemsList);
    }

    connect(m_patientBrowserList, SIGNAL(isActive(QString)), SLOT(updateActiveItemView(QString)));
    connect(m_patientBrowserList, SIGNAL(selectedItem(QString)), SLOT(processSelectedItem(QString)));
}

void PatientBrowserMenu::setShowFusionOptions(bool show)
{
    m_showFusionOptions = show;
}

void PatientBrowserMenu::updateActiveItemView(const QString &identifier)
{
    if (identifier.contains("+"))
    {
        QList<PatientBrowserMenuExtendedItem*> items;
        foreach(const QString &stringID, identifier.split("+"))
        {
            Identifier id(stringID.toInt());
            Volume *volume = VolumeRepository::getRepository()->getVolume(id);
            if (volume)
            {
                //We update the item data with additional information
                PatientBrowserMenuExtendedItem *item = new PatientBrowserMenuExtendedItem(m_patientAdditionalInfo);
                item->setPixmap(volume->getThumbnail());
                Series *series = volume->getImage(0)->getParentSeries();
                item->setText(QString(tr("%1\n%2\n%3\n%4 Images"))
                              .arg(series->getDescription().trimmed())
                              .arg(series->getModality().trimmed())
                              .arg(series->getProtocolName().trimmed())
                              .arg(volume->getNumberOfFrames())
                              );
                items << item;
            }
        }
        m_patientAdditionalInfo->setItems(items);
    }
    else
    {
        Identifier id(identifier.toInt());
        Volume *volume = VolumeRepository::getRepository()->getVolume(id);
        if (volume)
        {
            // We update the item data with additional information
            PatientBrowserMenuExtendedItem *item = new PatientBrowserMenuExtendedItem(m_patientAdditionalInfo);
            item->setPixmap(volume->getThumbnail());
            Series *series = volume->getImage(0)->getParentSeries();
            item->setText(QString(tr("%1\n%2\n%3\n%4 Images"))
                          .arg(series->getDescription().trimmed())
                          .arg(series->getModality().trimmed())
                          .arg(series->getProtocolName().trimmed())
                          .arg(volume->getNumberOfFrames())
                          );
            m_patientAdditionalInfo->setItems(QList<PatientBrowserMenuExtendedItem*>() << item);
        }
    }
    placeAdditionalInfoWidget();
}

void PatientBrowserMenu::popup(const QPoint &point, const QString &identifier)
{
    // We mark the current item in the list
    m_patientBrowserList->markItem(identifier);

    // We determine what screens we have
    ScreenManager screenManager;
    m_currentScreenID = screenManager.getScreenID(point);
    ScreenLayout screenLayout = screenManager.getScreenLayout();
    m_leftScreenID = screenLayout.getScreenOnTheLeftOf(m_currentScreenID);
    m_rightScreenID = screenLayout.getScreenOnTheRightOf(m_currentScreenID);

    QRect currentScreenGeometry = screenLayout.getScreen(m_currentScreenID).getAvailableGeometry();

    m_patientBrowserList->setMaximumWidth(currentScreenGeometry.width() - 300);
    m_patientBrowserList->setMaximumHeight(currentScreenGeometry.height() - currentScreenGeometry.height() / 5);

    ///We calculate the alignment of the menu
    bool rightAligned = shouldAlignMenuToTheRight(currentScreenGeometry);

    /// Calculate how much of the list is left out according to the geometry of the screen where
    /// we have been asked to open the menu
    /// and thus be able to determine the position of the widget so that it does not go out
    /// of the screen and its content can be seen.
    QSize outside;
    computeListOutsideSize(point, outside, rightAligned);

    /// TODO What do we do if the additional screen has a smaller height than the current one?
    /// Do we recalculate the entire menu so that it has the lowest height?
    /// At the moment this will not be taken into account

    /// If the values are positive it will be necessary to move the original position at
    /// least everything that we leave the borders
    /// TODO In theory this Margin would not be necessary if we used frameGeometry () instead of sizeHint ()
    const int Margin = 5;
    int menuXPosition = point.x();
    int menuYPosition = point.y();
    if (outside.width() > 0)
    {
        menuXPosition -= outside.width() + Margin;
    }
    if (outside.height() > 0)
    {
        menuYPosition -= outside.height() + Margin;
    }

    //We move the menu window to the point it touches
    m_patientBrowserList->move(menuXPosition, menuYPosition);

    /// We place the widget with the additional information to the right
    /// or left of the main one according to the available space
    placeAdditionalInfoWidget();
    m_patientAdditionalInfo->show();
    /// TODO: HACK if we don't show the "Additional information" before the "browser list" then it doesn't process the events
    /// correctly and when we select a series the signal does not get anywhere and no series can be selected
    /// related to ticket # 555 This only happens with qt 4.3, with qt 4.2 we don't have this one
    /// problem. With qt 4.2 we can do show in the order we want. What we do to avoid flickering is show it under itself
    /// from the "browser list" and so it is not so noticeable
    /// This, as of Qt 4.7 seems to only happen on Mac. It would seem that this is the
    /// symptom of another problem: that it is displayed as a popup?
    m_patientBrowserList->show();

    // TODO It is not taken into account if after having moved the widget it already looks correctly,
    // since the widget might not fit in full screen
    // Behavior should be improved in certain aspects, such as redistributing rows and columns
    // from the list if it doesn't fit on the screen, for example.
}

bool PatientBrowserMenu::shouldAlignMenuToTheRight(const QRect &currentScreenGeometry)
{
    bool rightAligned = true;
    // Check if the resulting list is wider
    if (m_patientBrowserList->sizeHint().width() > currentScreenGeometry.width())
    {
        if (m_leftScreenID == -1)
        {
            if (m_rightScreenID == -1)
            {
                // We can't expand anymore, we don't have more screens.
                // TODO Explore top and bottom?
            }
            else
            {
                //We have screen to expand to the right
                rightAligned = false;
            }
        }
        else
        {
            if (m_rightScreenID == -1)
            {
                //We have screen to expand to the left
                rightAligned = true;
            }
            else
            {
                // We have screens on the left and right. You need to choose the most appropriate one.
                // Option 1) The screen with the same height or greater to maintain appearance
                ScreenLayout screenLayout = ScreenManager().getScreenLayout();
                QRect leftScreenGeometry = screenLayout.getScreen(m_leftScreenID).getGeometry();
                QRect rightScreenGeometry = screenLayout.getScreen(m_rightScreenID).getGeometry();

                if (leftScreenGeometry.height() >= rightScreenGeometry.height())
                {
                    rightAligned = true;
                }
                else
                {
                    rightAligned = false;
                }
            }
        }
    }

    return rightAligned;
}

void PatientBrowserMenu::computeListOutsideSize(const QPoint &popupPoint, QSize &out, bool rightAligned)
{
    // We get the geometry of the screen where we have been asked to open the menu
    ScreenLayout screenLayout = ScreenManager().getScreenLayout();
    QRect currentScreenGeometry = screenLayout.getScreen(m_currentScreenID).getAvailableGeometry();
    QPoint currentScreenGlobalOriginPoint = currentScreenGeometry.topLeft();

    // We calculate the sizes of the widgets to know where to place them
    // TODO These measurements are not the most accurate. The appropriate ones should be based on the values ​​of QWidget :: frameGeometry ()
    // as explained at http://doc.trolltech.com/4.7/application-windows.html#window-geometry
    // The problem is that we need to have done a show () before to know the true sizes. One option would also be to implement
    // this positioning when a resize of these widgets occurs. This is one of the reasons we see the menu
    // additional with a small shadow when it stays to the right of the main one.
    int mainMenuApproximateWidth = m_patientBrowserList->sizeHint().width();
    int wholeMenuApproximateHeight = qMax(m_patientBrowserList->sizeHint().height(), m_patientAdditionalInfo->sizeHint().height());

    //// Calculate the borders where the menu could come out (right / left side)
    if (rightAligned)
    {
        int globalRight = currentScreenGlobalOriginPoint.x() + currentScreenGeometry.width();
        int widgetRight = mainMenuApproximateWidth + popupPoint.x();
        out.setWidth(widgetRight - globalRight);
    }
    else
    {
        int globalLeft = currentScreenGlobalOriginPoint.x();
        int widgetLeft = popupPoint.x();
        out.setWidth(widgetLeft - globalLeft);
    }
    
    // We calculate the borders where the menu could come out (height)
    int globalBottom = currentScreenGlobalOriginPoint.y() + currentScreenGeometry.height();
    int widgetBottom = wholeMenuApproximateHeight + popupPoint.y();
    out.setHeight(widgetBottom - globalBottom);
}

void PatientBrowserMenu::placeAdditionalInfoWidget()
{
    ScreenLayout screenLayout = ScreenManager().getScreenLayout();
    QRect currentScreenGeometry = screenLayout.getScreen(m_currentScreenID).getAvailableGeometry();
    QPoint currentScreenGlobalOriginPoint = currentScreenGeometry.topLeft();

    // TODOThis measure is not the most accurate. The appropriate one should be based on the values of QWidget :: frameGeometry ()
    int mainMenuApproximateWidth = m_patientBrowserList->sizeHint().width();
    int menuXPosition = m_patientBrowserList->pos().x();
    int menuYPosition = m_patientBrowserList->pos().y();
    //An adjust size must be made first so that the widget sizes are updated correctly
    m_patientAdditionalInfo->adjustSize();
    int additionalInfoWidgetApproximateWidth = m_patientAdditionalInfo->frameGeometry().width();

    int menuXShift = 0;
    if (menuXPosition + mainMenuApproximateWidth + additionalInfoWidgetApproximateWidth > currentScreenGlobalOriginPoint.x() + currentScreenGeometry.width())
    {
        ///In case the menu combination itself already occupies the entire screen
        /// we check on which screen it is better to place the additional menu
        if (mainMenuApproximateWidth + additionalInfoWidgetApproximateWidth >= currentScreenGlobalOriginPoint.x() + currentScreenGeometry.width())
        {
            if (m_leftScreenID != -1)
            {
                // To the left
                menuXShift = -additionalInfoWidgetApproximateWidth;
            }
            else if (m_rightScreenID != -1)
            {
                //On the right
                menuXShift = mainMenuApproximateWidth;
            }
            else
            {
                //  We have no screens either left or right, we indicate on the left
                menuXShift = -additionalInfoWidgetApproximateWidth;
            }
        }
        else
        {
            // We have space on the left, inside the same screen
            menuXShift = -additionalInfoWidgetApproximateWidth;
        }
    }
    else
    {
        //  We have space on the right, inside the same screen
        menuXShift = mainMenuApproximateWidth;
    }

    m_patientAdditionalInfo->move(menuXPosition + menuXShift, menuYPosition);
}

void PatientBrowserMenu::processSelectedItem(const QString &identifier)
{
    m_patientAdditionalInfo->hide();
    m_patientBrowserList->hide();

    if (m_patientBrowserList->getMarkedItem() != identifier)
    {
        if (!identifier.contains("+"))
        {
            Identifier id(identifier.toInt());
            emit selectedVolume(VolumeRepository::getRepository()->getVolume(id));
        }
        else
        {
            QList<Volume*> volumes;
            foreach (const QString &id, identifier.split("+"))
            {
                volumes << VolumeRepository::getRepository()->getVolume(Identifier(id.toInt()));
            }
            emit selectedVolumes(volumes);
        }
    }
}

void PatientBrowserMenu::createWidgets()
{
    if (m_patientAdditionalInfo)
    {
        delete m_patientAdditionalInfo;
    }

    if (m_patientBrowserList)
    {
        delete m_patientBrowserList;
    }

    m_patientAdditionalInfo = new PatientBrowserMenuExtendedInfo(this);
    m_patientBrowserList = new PatientBrowserMenuList(this);

    m_patientAdditionalInfo->setWindowFlags(Qt::Popup);
    m_patientBrowserList->setWindowFlags(Qt::Popup);

    connect(m_patientAdditionalInfo, SIGNAL(closed()), m_patientBrowserList, SLOT(close()));
    connect(m_patientBrowserList, SIGNAL(closed()), m_patientAdditionalInfo, SLOT(close()));
}

}
