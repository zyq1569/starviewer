-- IMPORTANT ATTENTION !!!!
-- EVERY TIME THIS SCRIPT IS UPDATED, REMEMBER TO GENERATE A SCRIPT TO UPDATE
-- THE EXISTING STARVIEWER USER DATABASE WITH NEW CHANGES AND UPDATING THE NUMBER
-- DATABASE REVIEW NECESSARY FOR COMPILATION OF STARVIEWER ACTUA A core / starviewerapplication.h
-- A CONSTANT StarviewerDatabaseRevisionRequired

CREATE TABLE DatabaseRevision
(
  Revision                      TEXT
); 

-- IMPORTANT !!! The revision number must be changed to a higher one each time a change is made to this file and if necessary
-- that the database is updated

INSERT INTO DatabaseRevision (Revision) VALUES ('9593');

CREATE TABLE PACSRetrievedImages
(
    ID                          INTEGER PRIMARY KEY AUTOINCREMENT,
    AETitle                     TEXT,
    Address                     TEXT,
    QueryPort                   INTEGER
);

CREATE UNIQUE INDEX IndexPacsDevice_AETitleAddressQueryPort ON PACSRetrievedImages(AETitle, Address, QueryPort);

CREATE TABLE Patient
( 
  ID                            INTEGER PRIMARY KEY AUTOINCREMENT,
  DICOMPatientId                TEXT,
  Name                          TEXT,
  BirthDate                     TEXT,
  Sex                           TEXT
);


CREATE TABLE Study
(
  InstanceUID                   TEXT PRIMARY KEY,
  PatientID                     TEXT NOT NULL,
  ID                            TEXT NOT NULL,
  PatientAge                    TEXT,
  PatientWeigth                 REAL,
  PatientHeigth                 REAL,
  Modalities                    TEXT,
  Date                          TEXT,
  Time                          TEXT,
  AccessionNumber               TEXT,
  Description                   TEXT,
  ReferringPhysicianName        TEXT,
  LastAccessDate                TEXT,
  RetrievedDate                 TEXT,
  RetrievedTime                 TEXT,
  State                         INTEGER
);

CREATE TABLE Series
(
  InstanceUID                   TEXT PRIMARY KEY,
  StudyInstanceUID              TEXT,
  Number                        TEXT,
  Modality                      TEXT,
  Date                          TEXT,
  Time                          TEXT,
  InstitutionName               TEXT,
  PatientPosition               TEXT,
  ProtocolName                  TEXT,
  Description                   TEXT,
  FrameOfReferenceUID           TEXT,
  PositionReferenceIndicator    TEXT,
  BodyPartExaminated            TEXT,
  ViewPosition                  TEXT,
  Manufacturer                  TEXT,
  Laterality                    TEXT,
  RetrievedDate                 TEXT,
  RetrievedTime                 TEXT,
  State                         INTEGER
);

CREATE INDEX  IndexSeries_StudyInstanceUID ON Series (StudyInstanceUID); 

CREATE TABLE Image
(
  SOPInstanceUID                TEXT,
  FrameNumber                   INTEGER,
  StudyInstanceUID              TEXT,
  SeriesInstanceUID             TEXT,
  InstanceNumber                TEXT,
  ImageOrientationPatient       TEXT,
  PatientOrientation            TEXT,
  PixelSpacing                  TEXT,
  SliceThickness                REAL,
  PatientPosition               TEXT,
  SamplesPerPixel               INTEGER,
  Rows                          INTEGER,
  Columns                       INTEGER,
  BitsAllocated                 INTEGER,
  BitsStored                    INTEGER,
  PixelRepresentation           INTEGER,
  RescaleSlope                  REAL,
  WindowLevelWidth              TEXT,
  WindowLevelCenter             TEXT,
  WindowLevelExplanations       TEXT,
  SliceLocation                 TEXT,
  RescaleIntercept              REAL,
  PhotometricInterpretation     TEXT,
  ImageType                     TEXT,
  ViewPosition                  TEXT,
  ImageLaterality               TEXT,
  ViewCodeMeaning               TEXT,
  PhaseNumber                   INTEGER,
  ImageTime                     TEXT,
  VolumeNumberInSeries          INTEGER,
  OrderNumberInVolume           INTEGER,
  RetrievedDate                 TEXT,
  RetrievedTime                 TEXT,
  State                         INTEGER,
  NumberOfOverlays              INTEGER,
  RetrievedPACSID               INTEGER,
  ImagerPixelSpacing            TEXT,
  EstimatedRadiographicMagnificationFactor    TEXT,
  TransferSyntaxUID             TEXT,
  PRIMARY KEY ('SOPInstanceUID', 'FrameNumber')
);

CREATE TABLE DisplayShutter
(
  ID                            INTEGER PRIMARY KEY AUTOINCREMENT,
  Shape                         TEXT,
  ShutterValue                  INTEGER,
  PointsList                    TEXT,
  ImageInstanceUID              TEXT,
  ImageFrameNumber              INTEGER,
  FOREIGN KEY (ImageInstanceUID, ImageFrameNumber) REFERENCES Image (SOPInstanceUID, FrameNumber)
);
--TODO:Check if the IndexImage_StudyInstanceUIDSeriesInstanceUID index will be used after changes made to the database
CREATE INDEX  IndexImage_StudyInstanceUIDSeriesInstanceUID ON Image (StudyInstanceUID,SeriesInstanceUID); 
CREATE INDEX  IndexImage_SOPInstanceUIDOrderNumberInVolume ON Image (SOPInstanceUID, OrderNumberInVolume); 

CREATE TABLE VoiLut
(
    ID                  INTEGER PRIMARY KEY AUTOINCREMENT,
    Lut                 BLOB,
    ImageInstanceUID    TEXT,
    ImageFrameNumber    INTEGER,
    FOREIGN KEY (ImageInstanceUID, ImageFrameNumber) REFERENCES Image (SOPInstanceUID, FrameNumber)
);

CREATE TABLE EncapsulatedDocument
(
    SOPInstanceUID                  TEXT PRIMARY KEY,
    TransferSyntaxUID               TEXT,
    InstanceNumber                  TEXT,
    DocumentTitle                   TEXT,
    MimeTypeOfEncapsulatedDocument  TEXT,
    RetrievedPacsID                 INTEGER,
    StudyInstanceUID                TEXT,
    SeriesInstanceUID               TEXT
);
