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

#ifndef UDGDICOMTAGREADER_H
#define UDGDICOMTAGREADER_H

#include <QMap>
#include <QString>
// Pràcticament sempre que volguem fer servir aquesta classe farem ús del diccionari
#include "dicomdictionary.h"

class DcmDataset;
class DcmSequenceOfItems;
class DcmElement;
class DcmMetaInfo;
class DcmItem;
class OFCondition;

namespace udg {

class DICOMTag;
class DICOMAttribute;
class DICOMSequenceAttribute;
class DICOMSequenceItem;
class DICOMValueAttribute;

/**
Class in charge of reading information from DICOM files.
It can be created from a file (the most common case) or using an existing DcmDataset (for efficiency cases).
In the case of creating a DICOMTagReader from a DcmDataset it should be noted that the owner of this will be the
same DICOMTagReader and therefore the object cannot be destroyed from the outside. DICOMTagReader will do the same.
*/
class DICOMTagReader {
public:
    /// This enum indicates whether we should return the value for
    /// all tags when required, or if for "Heavy" Tags (PixelData, OverlayData)
    /// we must return them without their value, saving us from reading and loading them into memory
    enum ReturnValueOfTags { AllTags, ExcludeHeavyTags };

    DICOMTagReader();
    /// Constructor by filename.
    DICOMTagReader(const QString &filename);
    /// Constructor by filename in case you have a DcmDataset already read.
    /// That way you don't have to read it again.
    DICOMTagReader(const QString &filename, DcmDataset *dcmDataset);

    virtual ~DICOMTagReader();

    /// Name of the DICOM file to be read. Returns true if the file was successfully uploaded, otherwise false.
    bool setFile(const QString &filename);

    /// Tells us if the assigned file is valid as a DICOM file. If we do not have an assigned file it will return false.
    bool canReadFile() const;

    /// Returns the name of the file handled by the tag reader.
    QString getFileName() const;

    /// Convenience method to take advantage of an already open DcmDataset.
    /// It is assumed that dcmDataset is not null and belongs to the past file.
    /// In case you already have a file open, replace it by deleting the previous DcmDataset. Once past the owner
    /// of the DcmDataset becomes the DICOMTagReader.
    void setDcmDataset(const QString &filename, DcmDataset *dcmDataset);

    /// Returns the dcmtk Dataset used internally
    DcmDataset* getDcmDataset() const;

    /// Tells us if the tag is present in the file or not. The m_dicomData object must have been used correctly.
    virtual bool tagExists(const DICOMTag &tag) const;

    /// Returns true if this reader contains the given tag and false otherwise.
    bool hasAttribute(const DICOMTag &tag) const;

    /// Gets the value of the requested attribute and returns it to us as QString
    /// If the tag is not found an empty QString will be returned
    virtual QString getValueAttributeAsQString(const DICOMTag &tag) const;

    /// Obtains the value at the given tag and returns it as a QByteArray with binary data (not text).
    /// Returns an empty QByteArray if the tag is not found or there is an error.
    QByteArray getValueAttributeAsByteArray(const DICOMTag &tag) const;

    /// Returns a DICOM attribute that is at the first level (that is not contained in sequences)
    /// Returns null in case the tag was not found or it did not match an attribute (eg it is a sequence)
    /// It does not discriminate whether that tag can be "heavy" or not, it will load all the requested information. For example, if we ask
    /// Pixel Data will load all this information
    DICOMValueAttribute* getValueAttribute(const DICOMTag &attributeTag) const;

    /// Returns an object that includes the whole sequence. If it doesn't exist or the tag doesn't correspond to a sequence, it returns null.
    /// By default it returns OverlayData and PixelData with their values, but they are returned without value if the second parameter is ExcludeHeavyTags.
    virtual DICOMSequenceAttribute* getSequenceAttribute(const DICOMTag &sequenceTag, DICOMTagReader::ReturnValueOfTags returnValueOfTags = AllTags) const;

    /// Returns the first item of the specified sequence, or null if the sequence does not exist or is empty.
    /// By default it returns OverlayData and PixelData with their values, but they are returned without value if the second parameter is ExcludeHeavyTags.
    DICOMSequenceItem* getFirstSequenceItem(const DICOMTag &sequenceTag, ReturnValueOfTags returnValueOfTags = AllTags) const;

    /// Returns a list of DICOMAttribute that includes all the Tags of a DcmDataset (It is assumed that the dataset will be valid)
    /// By default it returns the OverlayData and PixelData tag with its value, but if we want it to be returned to us with its
    /// empty value because they weigh a lot (in case of a breast can occupy more than 80Mb of RAM the PixelData) and we will not use them,
    /// as a second parameter the enum with value ExcludeHeavyTags must be passed.
    /// The class invoking this method is responsible for deleting the DICOMAttribute list
    QList<DICOMAttribute*> getDICOMDataSet(DICOMTagReader::ReturnValueOfTags returnValueOfTags = AllTags) const;

    /// Returns a DICOMAttribute list that includes all DICOMHeader Tags
    /// The class invoking this method is responsible for deleting the DICOMAttribute list
    QList<DICOMAttribute*> getDICOMHeader() const;

private:
    /// Initializes class attributes on creation
    void initialize();

    /// Initializes the text codec according to the current dataset.
    void initializeTextCodec();

    /// Converts a DCMTK sequence to a sequence of its own.
    DICOMSequenceAttribute* convertToDICOMSequenceAttribute(DcmSequenceOfItems *dcmtkSequence, DICOMTagReader::ReturnValueOfTags returnValueOfTags) const;

    ///Converts an element from DCMTK to its own DICOMValueAttribute. If the item could not be converted, a NULL value is returned
    DICOMValueAttribute* convertToDICOMValueAttribute(DcmElement *dcmtkDICOMElement, DICOMTagReader::ReturnValueOfTags returnValueOfTags) const;

    /// Converts a dcmtk DcmItem object to a DICOMAttribute list
    QList<DICOMAttribute*> convertToDICOMAttributeQList(DcmItem *dcmItem, DICOMTagReader::ReturnValueOfTags returnValueOfTags) const;

    /// Clears the data from the last uploaded file. If we didn’t have any files uploaded it doesn’t matter.
    void deleteDataLastLoadedFile();

    ///Write the log according to the status passed by parameter for
    /// when an operation was performed with a tag and went wrong for some reason
    void logStatusForTagOperation(const DICOMTag &tag, const OFCondition &status) const;

private:
    /// Absolute path where the file from which we extract the information is located
    QString m_filename;

    /// dcmtk object through which we obtain the DICOM information
    DcmDataset *m_dicomData;
    DcmMetaInfo *m_dicomHeader;

    /// Tells us if the current file is valid
    bool m_hasValidFile;

    /// Holds sequences that have already been retrieved.
    mutable QMap<DICOMTag, DICOMSequenceAttribute*> m_sequencesCache;

    /// Text codec used to convert dataset strings to UTF-8.
    QTextCodec *m_textCodec;

};

}

#endif
