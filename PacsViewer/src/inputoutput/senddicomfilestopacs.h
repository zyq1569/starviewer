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

#ifndef UDGSENDDICOMFILESTOPACS_H
#define UDGSENDDICOMFILESTOPACS_H

#include <QList>
#include <QObject>
#include <ofcond.h>

#include "pacsdevice.h"
#include "pacsrequeststatus.h"
#include "dimsecservice.h"

class DcmDataset;

struct T_DIMSE_C_StoreRSP;
struct T_ASC_Association;

namespace udg {

class Image;
class PACSConnection;

class SendDICOMFilesToPACS : public QObject, public DIMSECService {
    Q_OBJECT
public:
    SendDICOMFilesToPACS(PacsDevice pacsDevice);

    /// Returns the PACS that was passed to the constructor
    /// and with which the DICOM file is sent
    PacsDevice getPacs();

    /// Saves the images specified in the list in the pacs set by the connection
    /// @param ImageListStore of images to send to PACS
    /// @return indicates method status
    PACSRequestStatus::SendRequestStatus send(QList<Image*> imageListToSend);

    /// Please cancel sending images. Cancellation of
    /// images is asynchronous is not carried out until the sending of the
    /// image that was being sent at the time of requesting cancellation
    void requestCancel();

    ///Returns the number of images sent successfully
    int getNumberOfDICOMFilesSentSuccesfully();

    ///Returns the number of images that failed to send
    int getNumberOfDICOMFilesSentFailed();

    /// Returns the number of images sent but warned,
    /// can give warning for example in case the PACS modifies some data of
    /// the picture
    int getNumberOfDICOMFilesSentWarning();

signals:
    /// Signal indicating that the past image has been sent
    /// per parameter in the PACS, and the number of images that are sent
    void DICOMFileSent(Image *image, int numberOfDICOMFilesSent);

protected:

    /// Number of files that have been sent successfully.
    int m_numberOfDICOMFilesSentSuccessfully;

private:

    /// Creates and returns a PACS connection to the given PACS device.
    virtual PACSConnection* createPACSConnection(const PacsDevice &pacsDevice) const;

    /// Removes images from the list when multiple images
    /// point to the same file, so that at the end each file is present only once.
    void removeDuplicateFiles(QList<Image*> &imageList) const;

    /// Initialize image counters to control how many failed / were sent ....
    void initialitzeDICOMFilesCounters(int numberOfDICOMFilesToSend);

    /// Process a Store SCP response that did not have Successfull Status
    void processResponseFromStoreSCP(unsigned int dimseStatusCode, QString filePathDicomObjectStoredFailed);

    /// Send an image to the PACS with the association passed by parameter,
    /// returns if the image was sent successfully
    virtual bool storeSCU(T_ASC_Association *association, QString filePathToStore);

    /// Returns a Status indicating how the C-Store operation ended
    PACSRequestStatus::SendRequestStatus getStatusStoreSCU();

private:

    /// Number of files that have been sent but with a warning.
    int m_numberOfDICOMFilesSentWithWarning;
    /// Total number of files that had to be sent.
    int m_numberOfDICOMFilesToSend;
    PacsDevice m_pacs;
    bool m_abortIsRequested;
    OFCondition m_lastOFCondition;

};

}

#endif
