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

#ifndef UDGDICOMTAG_H
#define UDGDICOMTAG_H

#include <QString>

namespace udg {

/**
Class to represent a DICOM tag.
A tag consists of integers, the group, and the element. Ex: (0020,0022)
*/
class DICOMTag {

public:
    DICOMTag();
    DICOMTag(unsigned int group, unsigned int element);
    DICOMTag(DICOMTag *tag);

    ~DICOMTag();

    ///Assign / Obtain the group value
    void setGroup(unsigned int group);
    unsigned int getGroup() const;

    /// Assign / Obtain the item value
    void setElement(unsigned int element);
    unsigned int getElement() const;

    /// Assign / Get tag name.
    void setName(const QString &name);
    QString getName() const;

    /// Returns the tag as text.
    QString getKeyAsQString() const;

    /// Returns true if group and element are equal.
    bool operator==(const DICOMTag &tag) const;
    /// Returns true if group or element are different.
    bool operator!=(const DICOMTag &tag) const;
    /// Returns true if this' group is smaller than tag's group or if groups
    /// are equal and this' element is smaller than tag's element.
    bool operator<(const DICOMTag &tag) const;

private:
    unsigned int m_group;
    unsigned int m_element;
    QString m_name;
};

}

#endif // DICOMTAG_H
