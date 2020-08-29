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

#ifndef UDGHANGINGPROTOCOL_H
#define UDGHANGINGPROTOCOL_H

#include <QMap>
#include <QRegExp>

namespace udg {

class HangingProtocolLayout;
class HangingProtocolMask;
class HangingProtocolImageSet;
class HangingProtocolDisplaySet;

class HangingProtocol {

public:
    HangingProtocol();

    /// Creates a deep copy of the given hanging protocol.
    HangingProtocol(const HangingProtocol &hangingProtocol);

    ~HangingProtocol();

    ///Put the name on the hanging protocol
    void setName(const QString &name);

    ///Definition of the different levels that a Hanging Protocol can have
    enum HangingProtocolLevel { Manufacturer, Site, UserGroup, SingleUser };

    /// Get the layout
    HangingProtocolLayout* getHangingProtocolLayout() const;

    ///Get the mascara
    HangingProtocolMask* getHangingProtocolMask() const;

    /// Assign the number of screens
    void setNumberOfScreens(int screens);

    ///Assigns the protocols that the hanging protocol handles
    void setProtocolsList(const QStringList &protocols);

    ///Assign window positions
    void setDisplayEnvironmentSpatialPositionList(const QStringList &positions);

    ///Add an image set
    void addImageSet(HangingProtocolImageSet *imageSet);

    /// Afegeix un display set
    void addDisplaySet(HangingProtocolDisplaySet *displaySet);

    /// Obtenir el nom del hanging protocol
    QString getName() const;

    ///Gets the total number of image sets
    int getNumberOfImageSets() const;

    /// Gets the total number of image sets
    int getNumberOfDisplaySets() const;

    /// Gets the list of image sets
    QList<HangingProtocolImageSet*> getImageSets() const;

    ///Gets the list of display sets
    QList<HangingProtocolDisplaySet*> getDisplaySets() const;

    /// Get image set with identifier "identifier"
    HangingProtocolImageSet* getImageSet(int identifier) const;

    /// Get the display set with "identifier"
    HangingProtocolDisplaySet* getDisplaySet(int identifier) const;

    ///Assigns / Gets the regular expression of the name of the institutions to which the images must belong
    void setInstitutionsRegularExpression(const QRegExp &institutionRegularExpression);
    QRegExp getInstitutionsRegularExpression() const;

    ///Method for displaying values
    void show() const;

    /// Put the identifier in the hanging protocol
    void setIdentifier(int id);

    ///Obtain the hanging protocol identifier
    int getIdentifier() const;

    /// Method for comparing hanging protocols
    bool isBetterThan(const HangingProtocol *hangingToCompare) const;

    /// Returns whether the method is strict or not
    bool isStrict() const;

    ///Assign whether the method is strict or not
    void setStrictness(bool strictness);

    /// Returns if the hanging protocol must have all different series
    bool getAllDifferent() const;

    ///Assigns whether the hanging protocol should have all different series
    void setAllDifferent(bool allDifferent);

    ///Assigns the icon type to represent it
    void setIconType(const QString &iconType);

    ///Gets the icon type to represent it
    QString getIconType() const;

    /// Sets the number of priors.
    void setNumberOfPriors(int numberOfPriors);

    /// Returns the number of priors.
    int getNumberOfPriors() const;

    /// Assigns a priority to the hanging protocol
    void setPriority(double priority);

    /// Returns the priority of the hanging protocol
    double getPriority() const;

    /// Compare whether two hanging protocols are the same
    bool compareTo(const HangingProtocol &hangingProtocol) const;

    /// Returns the number of ImageSets that have a series assigned
    int countFilledImageSets() const;
    /// Return the number of filled ImageSets that its AbstractPriorValue is different to 0
    int countFilledImageSetsWithPriors() const;

    /// Returns the number of DisplaySets that have a series or image assigned
    int countFilledDisplaySets() const;
private:
    //Private copy assignment operator so it can't be used accidentally
    HangingProtocol& operator=(const HangingProtocol&);

private:
    /// Identificador
    int m_identifier;

    /// Name of the hanging protocol
    QString m_name;

    ///Definition of layouts
    HangingProtocolLayout *m_layout;

    /// Definition of the mask
    HangingProtocolMask *m_mask;

    QRegExp m_institutionsRegularExpression;

    /// Map from identifier to image set.
    QMap<int, HangingProtocolImageSet*> m_imageSets;

    /// Map from identifier to display set.
    QMap<int, HangingProtocolDisplaySet*> m_displaySets;

    /// Boolea indicating whether it is strict or not. If it is strict it means that to be correct all image sets must be assigned.
    bool m_strictness;

    /// Boolea which indicates whether the series should all be different from each other.
    bool m_allDifferent;

    ///Indicates the type of icon to represent the hanging protocol
    QString m_iconType;

    /// The number of priors in this hanging protocol (based on DICOM Number of Priors Referenced (0072,0014)).
    int m_numberOfPriors;

    /// Priority of this hanging protocol (default: 1).
    double m_priority;
};

}

#endif
