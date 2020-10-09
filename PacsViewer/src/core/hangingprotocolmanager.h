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

#ifndef UDGHANGINGPROTOCOLMANAGER_H
#define UDGHANGINGPROTOCOLMANAGER_H

#include <QObject>
#include <QList>
#include <QMultiHash>
#include <QPointer>
#include <QProgressDialog>

namespace udg {

class ViewersLayout;
class HangingProtocol;
class HangingProtocolImageSet;
class HangingProtocolDisplaySet;
class Patient;
class Series;
class Study;
class Image;
class Q2DViewerWidget;
class Q2DViewer;
class RelatedStudiesManager;

/**
HP Management Class: Find HP candidates and apply HP.
Because HPs are modified to assign the series to be displayed,
a copy of the repository is made.
*/
class HangingProtocolManager : public QObject {
    Q_OBJECT
public:
    HangingProtocolManager(QObject *parent = 0);
    ~HangingProtocolManager();

    /// Search for available hanging protocols
    QList<HangingProtocol*> searchHangingProtocols(Study *study);
    QList<HangingProtocol*> searchHangingProtocols(Study *study, const QList<Study*> &previousStudies);

    /// Apply a specific hanging protocol
    void applyHangingProtocol(HangingProtocol *hangingProtocol, ViewersLayout *layout, Patient *patient);
    void applyHangingProtocol(HangingProtocol *hangingProtocol, ViewersLayout *layout, Patient *patient, const QRectF &geometry);

    ///Apply the best hanging protocol from the given list
    HangingProtocol* setBestHangingProtocol(Patient *patient, const QList<HangingProtocol*> &hangingProtocolList, ViewersLayout *layout, const QRectF &geometry);

    ///If there were downloading studies, it is removed from the list
    void cancelAllHangingProtocolsDownloading();
    void cancelHangingProtocolDownloading(HangingProtocol *hangingProtocol);

protected:

    /// Make a copy of the HP repository so that you can modify them without the repository being affected.
    virtual void copyHangingProtocolRepository();

protected:
    ///Copy of the HP repository but be able to modify them without affecting the repository
    QList<HangingProtocol*> m_availableHangingProtocols;

private slots:
    /// A previous requested study has been downloaded
    void previousStudyDownloaded(Study *study);

    /// Slot that checks if the error in downloading a study is one that was expected
    void errorDownloadingPreviousStudies(const QString &studyUID);

private:
    ///See if the protocol can be applied to the patient
    bool isModalityCompatible(HangingProtocol *protocol, Study *study);

    /// See if the mode supports the protocol
    bool isModalityCompatible(HangingProtocol *protocol, const QString &modality);

    /// See if the institution supports the protocol
    bool isInstitutionCompatible(HangingProtocol *protocol, Study *study);

    /// Check if the protocol is applicable to the institution.
    ///  If the protocol does not have regular expression by institution it is applicable
    bool isValidInstitution(HangingProtocol *protocol, const QString &institutionName);

    /// Method responsible for assigning input to the viewer
    /// based on the specifications of the displaySet + imageSet.
    void setInputToViewer(Q2DViewerWidget *viewerWidget, HangingProtocolDisplaySet *displaySet);

private:
    /// Structure to store the data needed when it is received that a patient has merged with a new study
    /// We have to save all the information because we only know
    /// that it is a previous one and until it has been downloaded we cannot know which series and images you have
    struct StructPreviousStudyDownloading
    {
        /// Widget where the information should be displayed
        QPointer<Q2DViewerWidget> widgetToDisplay;
        /// We save the display set to be able to choose the
        /// orientation (useful on the breast) and if necessary a tool as well
        HangingProtocolDisplaySet *displaySet;
    };

    QHash<HangingProtocol*, QMultiHash<QString, StructPreviousStudyDownloading*>*> *m_hangingProtocolsDownloading;

    ///Object used to download related studies.
    ///  QueryScreen is not used for folder dependency issues.
    RelatedStudiesManager *m_relatedStudiesManager;
};

}

#endif
