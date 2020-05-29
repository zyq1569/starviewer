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

#include "localdatabaseimagedal.h"

#include <QString>
#include <QVector2D>

#include <sqlite3.h>

#include "study.h"
#include "series.h"
#include "image.h"
#include "databaseconnection.h"
#include "dicommask.h"
#include "localdatabasemanager.h"
#include "imageorientation.h"
#include "localdatabasedisplayshutterdal.h"
#include "dicomsource.h"
#include "localdatabasepacsretrievedimagesdal.h"
#include "dicomformattedvaluesconverter.h"
#include "dicomvaluerepresentationconverter.h"
#include "localdatabasevoilutdal.h"

namespace udg {

LocalDatabaseImageDAL::LocalDatabaseImageDAL(DatabaseConnection *dbConnection)
 : LocalDatabaseBaseDAL(dbConnection)
{
}

void LocalDatabaseImageDAL::insert(Image *newImage)
{
    m_lastSqliteError = sqlite3_exec(m_dbConnection->getConnection(), buildSqlInsert(newImage).toUtf8().constData(), 0, 0, 0);

    if (getLastError() != SQLITE_OK)
    {
        logError(buildSqlInsert(newImage));
    }
}

void LocalDatabaseImageDAL::del(const DicomMask &imageMaskToDelete)
{
    m_lastSqliteError = sqlite3_exec(m_dbConnection->getConnection(), buildSqlDelete(imageMaskToDelete).toUtf8().constData(), 0, 0, 0);

    if (getLastError() != SQLITE_OK)
    {
        logError(buildSqlDelete(imageMaskToDelete));
    }
}

void LocalDatabaseImageDAL::update(Image *imageToUpdate)
{
    m_lastSqliteError = sqlite3_exec(m_dbConnection->getConnection(), buildSqlUpdate(imageToUpdate).toUtf8().constData(), 0, 0, 0);

    if (getLastError() != SQLITE_OK)
    {
        logError(buildSqlUpdate(imageToUpdate));
    }
}

QList<Image*> LocalDatabaseImageDAL::query(const DicomMask &imageMask)
{
    int columns;
    int rows;
    char **reply = NULL;
    char **error = NULL;
    QList<Image*> imageList;

    m_lastSqliteError = sqlite3_get_table(m_dbConnection->getConnection(),
                                          buildSqlSelect(imageMask).toUtf8().constData(),
                                          &reply, &rows, &columns, error);
    if (getLastError() != SQLITE_OK)
    {
        logError (buildSqlSelect(imageMask));
        return imageList;
    }

    LocalDatabaseDisplayShutterDAL shutterDAL(m_dbConnection);
    LocalDatabaseVoiLutDAL voiLutDal(m_dbConnection);

    // index = 1 ignorem les capçaleres
    for (int index = 1; index <= rows; index++)
    {
        Image *newImage = fillImage(reply, index, columns);
            
        // Obtenim els shutters de l'imatge actual
        DicomMask mask;
        mask.setSOPInstanceUID(newImage->getSOPInstanceUID());
        mask.setImageNumber(QString::number(newImage->getFrameNumber()));
        newImage->setDisplayShutters(shutterDAL.query(mask));

        // Get VOI LUTs
        QList<VoiLut> voiLuts = voiLutDal.query(mask);

        foreach (const VoiLut &voiLut, voiLuts)
        {
            newImage->addVoiLut(voiLut);
        }
        
        imageList << newImage;
    }

    sqlite3_free_table(reply);

    return imageList;
}

int LocalDatabaseImageDAL::count(const DicomMask &imageMaskToCount)
{
    int columns;
    int rows;
    char **reply = NULL;
    char **error = NULL;

    m_lastSqliteError = sqlite3_get_table(m_dbConnection->getConnection(),
                                          buildSqlSelectCountImages(imageMaskToCount).toUtf8().constData(),
                                          &reply, &rows, &columns, error);

    if (getLastError() != SQLITE_OK)
    {
        logError (buildSqlSelectCountImages(imageMaskToCount));
        return -1;
    }

    int numberOfImages = QString(reply[1]).toInt();
    sqlite3_free_table(reply);

    return numberOfImages;
}

Image* LocalDatabaseImageDAL::fillImage(char **reply, int row, int columns)
{
    Image *image = new Image();

    image->setSOPInstanceUID(reply[0 + row * columns]);
    image->setFrameNumber(QString(reply[1 + row * columns]).toInt());
    image->setInstanceNumber(reply[4 + row * columns]);
    ImageOrientation imageOrientation;
    imageOrientation.setDICOMFormattedImageOrientation((reply[5 + row * columns]));
    image->setImageOrientationPatient(imageOrientation);
    
    PatientOrientation patientOrientation;
    patientOrientation.setDICOMFormattedPatientOrientation(reply[6 + row * columns]);
    image->setPatientOrientation(patientOrientation);
    
    image->setPixelSpacing(getPixelSpacingAsDouble(reply[7 + row * columns])[0], getPixelSpacingAsDouble(reply[7 + row * columns])[1]);
    image->setSliceThickness(QString(reply[8 + row * columns]).toDouble());
    image->setImagePositionPatient(getPatientPositionAsDouble(reply[9 + row * columns]));
    image->setSamplesPerPixel(QString(reply[10 + row * columns]).toInt());
    image->setRows(QString(reply[11 + row * columns]).toInt());
    image->setColumns(QString(reply[12 + row * columns]).toInt());
    image->setBitsAllocated(QString(reply[13 + row * columns]).toInt());
    image->setBitsStored(QString(reply[14 + row * columns]).toInt());
    image->setPixelRepresentation(QString(reply[15 + row * columns]).toInt());
    image->setRescaleSlope(QString(reply[16 + row * columns]).toDouble());
    QList<WindowLevel> windowLevelList = DICOMFormattedValuesConverter::parseWindowLevelValues(reply[17 + row * columns], reply[18 + row * columns],
                                                                                               convertToQString(reply[19 + row * columns]));
    QList<VoiLut> voiLutList;
    foreach (const WindowLevel &windowLevel, windowLevelList)
    {
        voiLutList.append(windowLevel);
    }
    image->setVoiLutList(voiLutList);
    image->setSliceLocation(reply[20 + row * columns]);
    image->setRescaleIntercept(QString(reply[21 + row * columns]).toDouble());
    image->setPhotometricInterpretation(reply[22 + row * columns]);
    image->setImageType(reply[23 + row * columns]);
    image->setViewPosition(reply[24 + row * columns]);
    // ImageLaterality sempre és un Char
    image->setImageLaterality(QChar(reply[25 + row * columns][0]));
    image->setViewCodeMeaning(convertToQString(reply[26 + row * columns]));
    image->setPhaseNumber(QString(reply[27 + row * columns]).toInt());
    image->setImageTime(reply[28 + row * columns]);
    image->setVolumeNumberInSeries(QString(reply [29 + row * columns]).toInt());
    image->setOrderNumberInVolume(QString(reply [30 + row * columns]).toInt());
    image->setRetrievedDate(QDate().fromString(reply[31 + row * columns], "yyyyMMdd"));
    image->setRetrievedTime(QTime().fromString(reply[32 + row * columns], "hhmmss"));
    image->setNumberOfOverlays(QString(reply[34 + row * columns]).toUShort());
    image->setDICOMSource(getImageDICOMSourceByIDPACSInDatabase(reply[35 + row * columns]));
    QVector2D imagerPixelSpacing = getImagerPixelSpacingAs2DVector(reply[36 + row * columns]);
    image->setImagerPixelSpacing(imagerPixelSpacing.x(), imagerPixelSpacing.y());
    image->setEstimatedRadiographicMagnificationFactor(QString(reply[37 + row * columns]).toDouble());
    image->setTransferSyntaxUID(reply[38 + row * columns]);

    // TODO argghh!!! Això només hauria d'estar en un únic lloc, no aquí i en retrieveimages.cpp
    image->setPath(LocalDatabaseManager::getCachePath() + reply[2 + row * columns] + "/" + reply[3 + row * columns] + "/" + reply[0 + row * columns]);

    return image;
}

QString LocalDatabaseImageDAL::buildSqlSelect(const DicomMask &imageMaskToSelect)
{
    QString selectSentence = "Select SOPInstanceUID, FrameNumber, StudyInstanceUID, SeriesInstanceUID, InstanceNumber,"
                                    "ImageOrientationPatient, PatientOrientation, PixelSpacing, SliceThickness,"
                                    "PatientPosition, SamplesPerPixel, Rows, Columns, BitsAllocated, BitsStored,"
                                    "PixelRepresentation, RescaleSlope, WindowLevelWidth, WindowLevelCenter,"
                                    "WindowLevelExplanations, SliceLocation, RescaleIntercept,"
                                    "PhotometricInterpretation, ImageType, ViewPosition,"
                                    "ImageLaterality, ViewCodeMeaning , PhaseNumber, ImageTime,  VolumeNumberInSeries,"
                                    "OrderNumberInVolume, RetrievedDate, RetrievedTime, State, NumberOfOverlays, RetrievedPACSID,"
                                    "ImagerPixelSpacing, EstimatedRadiographicMagnificationFactor, TransferSyntaxUID "
                            "from Image ";

    QString orderSentence = " order by VolumeNumberInSeries, OrderNumberInVolume";

    return selectSentence + buildWhereSentence(imageMaskToSelect) + orderSentence;
}

QString LocalDatabaseImageDAL::buildSqlSelectCountImages(const DicomMask &imageMaskToSelect)
{
    QString selectSentence = "Select count(*) from Image ";

    return selectSentence + buildWhereSentence(imageMaskToSelect);
}

QString LocalDatabaseImageDAL::buildSqlInsert(Image *newImage)
{
    QString windowWidth, windowCenter, windowExplanation;
    getWindowLevelInformationAsQString(newImage, windowWidth, windowCenter, windowExplanation);
    
    QString insertSentence = QString("Insert into Image (SOPInstanceUID, FrameNumber, StudyInstanceUID, SeriesInstanceUID, InstanceNumber,"
                                             "ImageOrientationPatient, PatientOrientation, PixelSpacing, SliceThickness,"
                                             "PatientPosition, SamplesPerPixel, Rows, Columns, BitsAllocated, BitsStored,"
                                             "PixelRepresentation, RescaleSlope, WindowLevelWidth, WindowLevelCenter,"
                                             "WindowLevelExplanations, SliceLocation,"
                                             "RescaleIntercept, PhotometricInterpretation, ImageType, ViewPosition,"
                                             "ImageLaterality, ViewCodeMeaning, PhaseNumber, ImageTime, VolumeNumberInSeries,"
                                             "OrderNumberInVolume, RetrievedDate, RetrievedTime, State, NumberOfOverlays, RetrievedPACSID,"
                                             "ImagerPixelSpacing, EstimatedRadiographicMagnificationFactor, TransferSyntaxUID) "
                                     "values ('%1', %2, '%3', '%4', '%5', "
                                             "'%6', '%7', '%8', %9,"
                                             "'%10', %11, %12, %13, %14, %15, "
                                             "%16, %17,'%18', '%19', "
                                             "'%20', '%21', "
                                             "%22, '%23', '%24', '%25',"
                                             "'%26',  '%27', %28, '%29', %30,"
                                             "%31, '%32', '%33', %34, %35, %36, '%37', %38, '%39')")
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(newImage->getSOPInstanceUID()))
                            .arg(newImage->getFrameNumber())
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(newImage->getParentSeries()->getParentStudy()->getInstanceUID()))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(newImage->getParentSeries()->getInstanceUID()))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(newImage->getInstanceNumber()))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(newImage->getImageOrientationPatient().getDICOMFormattedImageOrientation()))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(newImage->getPatientOrientation().getDICOMFormattedPatientOrientation()))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(getPixelSpacingAsQString(newImage)))
                            .arg(newImage->getSliceThickness())
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(getPatientPositionAsQString(newImage)))
                            .arg(newImage->getSamplesPerPixel())
                            .arg(newImage->getRows())
                            .arg(newImage->getColumns())
                            .arg(newImage->getBitsAllocated())
                            .arg(newImage->getBitsStored())
                            .arg(newImage->getPixelRepresentation())
                            .arg(newImage->getRescaleSlope())
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(windowWidth))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(windowCenter))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(windowExplanation))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(newImage->getSliceLocation()))
                            .arg(newImage->getRescaleIntercept())
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(newImage->getPhotometricInterpretation().getAsQString()))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(newImage->getImageType()))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(newImage->getViewPosition()))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(newImage->getImageLaterality()))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(newImage->getViewCodeMeaning()))
                            .arg(newImage->getPhaseNumber())
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(newImage->getImageTime()))
                            .arg (newImage->getVolumeNumberInSeries())
                            .arg(newImage->getOrderNumberInVolume())
                            .arg(newImage->getRetrievedDate().toString("yyyyMMdd"))
                            .arg(newImage->getRetrievedTime().toString("hhmmss"))
                            .arg(0)
                            .arg(newImage->getNumberOfOverlays())
                            .arg(getIDPACSInDatabaseFromDICOMSource(newImage->getDICOMSource()))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(getImagerPixelSpacingAsQString(newImage)))
                            .arg(newImage->getEstimatedRadiographicMagnificationFactor())
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(newImage->getTransferSyntaxUID()));

    return insertSentence;
}

QString LocalDatabaseImageDAL::buildSqlUpdate(Image *imageToUpdate)
{
    QString windowWidth, windowCenter, windowExplanation;
    getWindowLevelInformationAsQString(imageToUpdate, windowWidth, windowCenter, windowExplanation);
    
    QString updateSentence = QString("Update Image set StudyInstanceUID = '%1',"
                                              "SeriesInstanceUID = '%2',"
                                              "InstanceNumber = '%3',"
                                              "ImageOrientationPatient = '%4',"
                                              "PatientOrientation = '%5',"
                                              "PixelSpacing = '%6',"
                                              "SliceThickness = '%7',"
                                              "PatientPosition = '%8',"
                                              "SamplesPerPixel = '%9',"
                                              "Rows = '%10',"
                                              "Columns = '%11',"
                                              "BitsAllocated = '%12',"
                                              "BitsStored = '%13',"
                                              "PixelRepresentation = '%14',"
                                              "RescaleSlope = '%15',"
                                              "WindowLevelWidth = '%16',"
                                              "WindowLevelCenter = '%17',"
                                              "WindowLevelExplanations = '%18',"
                                              "SliceLocation = '%19',"
                                              "RescaleIntercept = '%20', "
                                              "PhotometricInterpretation = '%21', "
                                              "ImageType = '%22', "
                                              "ViewPosition = '%23', "
                                              "ImageLaterality = '%24', "
                                              "ViewCodeMeaning = '%25', "
                                              "PhaseNumber = %26, "
                                              "ImageTime = '%27', "
                                              "VolumeNumberInSeries = %28, "
                                              "OrderNumberInVolume = '%29', "
                                              "RetrievedDate = '%30', "
                                              "RetrievedTime = '%31', "
                                              "State = %32, "
                                              "NumberOfOverlays = %33, "
                                              "RetrievedPACSID = %34, "
                                              "ImagerPixelSpacing = '%37', "
                                              "EstimatedRadiographicMagnificationFactor = %38, "
                                              "TransferSyntaxUID = '%39' "
                                     "Where SOPInstanceUID = '%35' And "
                                           "FrameNumber = %36")
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(imageToUpdate->getParentSeries()->getParentStudy()->getInstanceUID()))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(imageToUpdate->getParentSeries()->getInstanceUID()))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(imageToUpdate->getInstanceNumber()))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(imageToUpdate->getImageOrientationPatient().getDICOMFormattedImageOrientation()))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(imageToUpdate->getPatientOrientation().getDICOMFormattedPatientOrientation()))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(getPixelSpacingAsQString(imageToUpdate)))
                            .arg(imageToUpdate->getSliceThickness())
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(getPatientPositionAsQString(imageToUpdate)))
                            .arg(imageToUpdate->getSamplesPerPixel())
                            .arg(imageToUpdate->getRows())
                            .arg(imageToUpdate->getColumns())
                            .arg(imageToUpdate->getBitsAllocated())
                            .arg(imageToUpdate->getBitsStored())
                            .arg(imageToUpdate->getPixelRepresentation())
                            .arg(imageToUpdate->getRescaleSlope())
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(windowWidth))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(windowCenter))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(windowExplanation))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(imageToUpdate->getSliceLocation()))
                            .arg(imageToUpdate->getRescaleIntercept())
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(imageToUpdate->getPhotometricInterpretation().getAsQString()))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(imageToUpdate->getImageType()))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(imageToUpdate->getViewPosition()))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(imageToUpdate->getImageLaterality()))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(imageToUpdate->getViewCodeMeaning()))
                            .arg (imageToUpdate->getPhaseNumber())
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(imageToUpdate->getImageTime()))
                            .arg (imageToUpdate->getVolumeNumberInSeries())
                            .arg(imageToUpdate->getOrderNumberInVolume())
                            .arg(imageToUpdate->getRetrievedDate().toString("yyyyMMdd"))
                            .arg(imageToUpdate->getRetrievedTime().toString("hhmmss"))
                            .arg(0)
                            .arg(imageToUpdate->getNumberOfOverlays())
                            .arg(getIDPACSInDatabaseFromDICOMSource(imageToUpdate->getDICOMSource()))
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(imageToUpdate->getSOPInstanceUID()))
                            .arg(imageToUpdate->getFrameNumber())
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(getImagerPixelSpacingAsQString(imageToUpdate)))
                            .arg(imageToUpdate->getEstimatedRadiographicMagnificationFactor())
                            .arg(DatabaseConnection::formatTextToValidSQLSyntax(imageToUpdate->getTransferSyntaxUID()));

    return updateSentence;
}

QString LocalDatabaseImageDAL::buildSqlDelete(const DicomMask &imageMaskToDelete)
{
    return "delete from Image " + buildWhereSentence(imageMaskToDelete);
}

QString LocalDatabaseImageDAL::buildWhereSentence(const DicomMask &imageMask)
{
    QString whereSentence = "";

    if (!imageMask.getStudyInstanceUID().isEmpty())
    {
        whereSentence = QString("where StudyInstanceUID = '%1'").arg(DatabaseConnection::formatTextToValidSQLSyntax(imageMask.getStudyInstanceUID()));
    }

    if (!imageMask.getSeriesInstanceUID().isEmpty())
    {
        if (whereSentence.isEmpty())
        {
            whereSentence = "where";
        }
        else
        {
            whereSentence += " and ";
        }

        whereSentence += QString(" SeriesInstanceUID = '%1'").arg(DatabaseConnection::formatTextToValidSQLSyntax(imageMask.getSeriesInstanceUID()));
    }

    if (!imageMask.getSOPInstanceUID().isEmpty())
    {
        if (whereSentence.isEmpty())
        {
            whereSentence = "where";
        }
        else
        {
            whereSentence += " and ";
        }

        whereSentence += QString(" SOPInstanceUID = '%1'").arg(DatabaseConnection::formatTextToValidSQLSyntax(imageMask.getSOPInstanceUID()));
    }

    return whereSentence;
}

QString LocalDatabaseImageDAL::getPixelSpacingAsQString(Image *newImage)
{
    QString imagePixelSpacing = "";
    QString value;
    // TODO Add method to PixelSpacing2D getAsDICOMString? or maybe in a helper class
    imagePixelSpacing += value.setNum(newImage->getPixelSpacing().x(), 'g', 10) + "\\";
    imagePixelSpacing += value.setNum(newImage->getPixelSpacing().y(), 'g', 10);

    return imagePixelSpacing;
}

double* LocalDatabaseImageDAL::getPixelSpacingAsDouble(const QString &pixelSpacing)
{
    QStringList list = pixelSpacing.split("\\");

    if (list.size() == 2)
    {
        m_pixelSpacing[0] = list.at(0).toDouble();
        m_pixelSpacing[1] = list.at(1).toDouble();
    }
    else
    {
        m_pixelSpacing[0] = 0;
        m_pixelSpacing[1] = 0;
    }

    return m_pixelSpacing;
}

QString LocalDatabaseImageDAL::getImagerPixelSpacingAsQString(Image *newImage) const
{
    PixelSpacing2D imagerPixelSpacing = newImage->getImagerPixelSpacing();
    // TODO Add method to PixelSpacing2D getAsDICOMString? or maybe in a helper class
    return QString("%1\\%2").arg(imagerPixelSpacing.x(), 0, 'g', 10).arg(imagerPixelSpacing.y(), 0, 'g', 10);
}

QVector2D LocalDatabaseImageDAL::getImagerPixelSpacingAs2DVector(const QString &imagerPixelSpacing) const
{
    return DICOMValueRepresentationConverter::decimalStringTo2DDoubleVector(imagerPixelSpacing);
}

double* LocalDatabaseImageDAL::getPatientPositionAsDouble(const QString &patientPosition)
{
    QStringList list = patientPosition.split("\\");

    for (int index = 0; index < 3; index++)
    {
        if (list.size() == 3)
        {
            m_patientPosition[index] = list.at(index).toDouble();
        }
        else
        {
            m_patientPosition[index] = 0;
        }
    }

    return m_patientPosition;
}

QString LocalDatabaseImageDAL::getPatientPositionAsQString(Image *newImage)
{
    QString patientPosition = "";
    QString value;

    patientPosition += value.setNum(newImage->getImagePositionPatient()[0], 'g', 10) + "\\";
    patientPosition += value.setNum(newImage->getImagePositionPatient()[1], 'g', 10) + "\\";
    patientPosition += value.setNum(newImage->getImagePositionPatient()[2], 'g', 10);

    return patientPosition;
}

void LocalDatabaseImageDAL::getWindowLevelInformationAsQString(Image *newImage, QString &windowWidth, QString &windowCenter, QString &explanation)
{
    windowWidth.clear();
    windowCenter.clear();
    explanation.clear();
    
    QString value;
    WindowLevel windowLevel;
    for (int index = 0; index < newImage->getNumberOfVoiLuts(); ++index)
    {
        const VoiLut &voiLut = newImage->getVoiLut(index);

        if (voiLut.isWindowLevel())
        {
            windowLevel = voiLut.getWindowLevel();
            windowWidth += value.setNum(windowLevel.getWidth(), 'g', 10) + "\\";
            windowCenter += value.setNum(windowLevel.getCenter(), 'g', 10) + "\\";
            explanation += windowLevel.getName() + "\\";
        }
    }

    // Treiem l'últim "\\" afegit
    windowWidth = windowWidth.left(windowWidth.length() - 1);
    windowCenter = windowCenter.left(windowCenter.length() - 1);
    explanation = explanation.left(explanation.length() - 1);
}

QString LocalDatabaseImageDAL::getIDPACSInDatabaseFromDICOMSource(DICOMSource DICOMSourceRetrievedImage)
{
    if (DICOMSourceRetrievedImage.getRetrievePACS().count() == 0)
    {
        return "null";
    }

    return getIDPACSInDatabase(DICOMSourceRetrievedImage.getRetrievePACS().at(0));
}

QString LocalDatabaseImageDAL::getIDPACSInDatabase(PacsDevice pacsDevice)
{
    QString keyPacsDevice = pacsDevice.getAddress() + QString().setNum(pacsDevice.getQueryRetrieveServicePort());

    if (m_PACSIDCache.contains(keyPacsDevice))
    {
        return m_PACSIDCache[keyPacsDevice];
    }

    LocalDatabasePACSRetrievedImagesDAL localDatabasePACSRetrievedImagesDAL(m_dbConnection);
    PacsDevice pacsDeviceRetrievedFromDatabase = localDatabasePACSRetrievedImagesDAL.query(pacsDevice.getAETitle(), pacsDevice.getAddress(),
                                                                                pacsDevice.getQueryRetrieveServicePort());
    if (!pacsDeviceRetrievedFromDatabase.getID().isEmpty())
    {
        //El pacs ja està inserit a la base de dades
        m_PACSIDCache[keyPacsDevice] = pacsDeviceRetrievedFromDatabase.getID();
        return pacsDeviceRetrievedFromDatabase.getID();
    }

    //No hem trobat el PACS l'inserir a la base de dades
    qlonglong PACSDInDatabase = localDatabasePACSRetrievedImagesDAL.insert(pacsDevice);

    if (localDatabasePACSRetrievedImagesDAL.getLastError() == SQLITE_OK)
    {
        m_PACSIDCache[keyPacsDevice] = QString().setNum(PACSDInDatabase);
        return m_PACSIDCache[keyPacsDevice];
    }

    return "null";
}

DICOMSource LocalDatabaseImageDAL::getImageDICOMSourceByIDPACSInDatabase(const QString &retrievedPACSID)
{
    DICOMSource imageDICOMSource;

    if (retrievedPACSID == "null")
    {
        // La imatge no té de quin PACS s'ha descarregat
        return imageDICOMSource;
    }

    PacsDevice pacsDevice = getPACSDeviceByIDPACSInDatabase(retrievedPACSID.toInt());

    if (!pacsDevice.getID().isEmpty())
    {
        imageDICOMSource.addRetrievePACS(pacsDevice);
    }

    return imageDICOMSource;
}

PacsDevice LocalDatabaseImageDAL::getPACSDeviceByIDPACSInDatabase(int IDPACSInDatabase)
{
    if (m_PACSDeviceCacheByIDPACSInDatabase.contains(IDPACSInDatabase))
    {
        return m_PACSDeviceCacheByIDPACSInDatabase[IDPACSInDatabase];
    }

    //Sinó el trobem a la caché el recuperem de la base de dades
    LocalDatabasePACSRetrievedImagesDAL localDatabasePACSRetrievedImagesDAL(m_dbConnection);
    PacsDevice pacsDevice = localDatabasePACSRetrievedImagesDAL.query(IDPACSInDatabase);

    if (!pacsDevice.getID().isEmpty())
    {
        m_PACSDeviceCacheByIDPACSInDatabase[IDPACSInDatabase] = pacsDevice;
    }

    return pacsDevice;
}

}
