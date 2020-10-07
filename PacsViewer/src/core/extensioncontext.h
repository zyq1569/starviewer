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

#ifndef UDGEXTENSIONCONTEXT_H
#define UDGEXTENSIONCONTEXT_H

#include <QStringList>

namespace udg {

class Patient;
class Volume;

/**
Context where an extension is executed. It allows access to information
that may be useful to the extension such as the Patient object, the window where it runs, the "readers".
*/
class ExtensionContext {
public:
    ExtensionContext();

    ~ExtensionContext();

    /// Get / set the patient with whom the extension should work.
    Patient* getPatient() const;
    void setPatient(Patient *patient);

    /// Convenience method that gives us a default volume from the assigned context.
    /// This will help us to get a volume from both the patient or not
    /// we have patient (case mhd's) do it with the volumeIdentifier
    Volume* getDefaultVolume() const;

    ///   /// Returns true if the patient of this context contains at least one image and false otherwise.
    bool hasImages() const;
    /// /// Returns true if the patient of this context contains at least one encapsulated document and false otherwise.
    bool hasEncapsulatedDocuments() const;

private:
    Patient* m_patient;

    QStringList m_defaultSelectedStudies;
    QStringList m_defaultSelectedSeries;
};

}

#endif
