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

#ifndef UDGDICOMDUMPDEFAULTTAGS_H
#define UDGDICOMDUMPDEFAULTTAGS_H

#include "dicomtag.h"
#include "dicomdumpdefaulttagsrestriction.h"
#include <QString>
#include <QList>

namespace udg {

class DICOMTag;

/**
    Classe que ens representa el Mappeig d'un fitxer que conté la llista de DefaulTags utilitzat pel DICOMDump
  */
class DICOMDumpDefaultTags {

public:
    DICOMDumpDefaultTags();

    ~DICOMDumpDefaultTags();

    /// Method for defining the SOPClassUID
    void setSOPClassUID (const QString & sopClassUid);

    /// Method that returns the SOPClassUID
    QString getSOPClassUID () const;

    /// Method for adding a constraint to the constraint list
    void addRestriction (const DICOMDumpDefaultTagsRestriction & restriction);

    /// Method for defining the constraint list of a DICOMDumpDefaultTags
    void addRestrictions (const QList <DICOMDumpDefaultTagsRestriction> & restrictionsList);

    /// Method that returns the constraint list of a DICOMDumpDefaultTags
    QList <DICOMDumpDefaultTagsRestriction> getRestrictions () const;

    /// Method that returns the number of constraints that a DICOMDumpDefaultTags has defined
    int getNumberOfRestrictions () const;

    /// Method for adding a DICOMTag to the list of DICOMTags to display
    void addTagToShow (const DICOMTag & dicomTag);

    /// Method for adding the list of Tags to be displayed by default in a DICOMDumpDefaultTags
    void addTagsToShow (const QList <DICOMTag> & tagsToShow);

    /// Returns the list of Tags to be displayed by default for the defined SOPClassUID
    QList<DICOMTag> getTagsToShow() const;

private:
    QString m_SOPClassUID;
    QList<DICOMDumpDefaultTagsRestriction> m_restrictions;
    QList<DICOMTag> m_tagsToShow;
};

}

#endif
