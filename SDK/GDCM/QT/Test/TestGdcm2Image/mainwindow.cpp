#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>
////
/// \brief MainWindow::MainWindow
/// \param parent
#include <QImage>
#include <QImageWriter>
#include "gdcmImageReader.h"
///
/// \brief MainWindow::MainWindow
/// \param parent
//#include "series.h"
//#include "image.h"
//#include "logging.h"
//#include "dicomtagreader.h"
// We use dcmtk for scaling dicom images
#include <dcmimage.h>
#include <ofbmanip.h>
#include <dcdatset.h>


#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcuid.h"       /* for dcmtk version name */
#include "dcmtk/dcmjpeg/djdecode.h"    /* for dcmjpeg decoders */
#include "dcmtk/dcmjpeg/dipijpeg.h"    /* for dcmimage JPEG plugin */

// Needed to support color images
#include <diregist.h>
///
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_TransferSyntax = 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

//未压缩：
//Implicit VR Little Endian:
//Default Transfer Syntax for DICOM               1.2.840.10008.1.2
//Explicit VR Little Endian                       1.2.840.10008.1.2.1
//Deflated Explicit VR Little Endian              1.2.840.10008.1.2.1.99
//Explicit VR Big Endian                          1.2.840.10008.1.2.2

//无损压缩：
//JPEG Lossless :
//Default  Transfer Syntax for Lossless JPEG ImageCompression               1.2.840.10008.1.2.4.70
//JPEG 2000 Image Compression(Lossless Only)                                1.2.840.10008.1.2.4.90
//RLE Lossless                                   1.2.840.10008.1.2.5
//有损压缩：
//Default Transfer Syntax for Lossy JPEG 12 Bit Image Compression           1.2.840.10008.1.2.4.51
//LOSSY(8 bit)                                 1.2.840.10008.1.2.4.50
//动态：
//MPEG2 image Compression                     1.2.840.10008.1.2.4.100

//TRANSFER SYNTAXES
//dcmdjpeg supports the following transfer syntaxes for input (dcmfile-in):
//LittleEndianImplicitTransferSyntax             1.2.840.10008.1.2
//LittleEndianExplicitTransferSyntax             1.2.840.10008.1.2.1
//DeflatedExplicitVRLittleEndianTransferSyntax   1.2.840.10008.1.2.1.99 (*)
//BigEndianExplicitTransferSyntax                1.2.840.10008.1.2.2
//JPEGProcess1TransferSyntax                     1.2.840.10008.1.2.4.50
//JPEGProcess2_4TransferSyntax                   1.2.840.10008.1.2.4.51
//JPEGProcess6_8TransferSyntax                   1.2.840.10008.1.2.4.53
//JPEGProcess10_12TransferSyntax                 1.2.840.10008.1.2.4.55
//JPEGProcess14TransferSyntax                    1.2.840.10008.1.2.4.57
//JPEGProcess14SV1TransferSyntax                 1.2.840.10008.1.2.4.70
//(*) if compiled with zlib support enabled
//dcmdjpeg supports the following transfer syntaxes for output (dcmfile-out):
//LittleEndianImplicitTransferSyntax             1.2.840.10008.1.2
//LittleEndianExplicitTransferSyntax             1.2.840.10008.1.2.1
//BigEndianExplicitTransferSyntax                1.2.840.10008.1.2.2


//dcmdjpeg supports the following transfer syntaxes for input (dcmfile-in):

//LittleEndianImplicitTransferSyntax             1.2.840.10008.1.2
//LittleEndianExplicitTransferSyntax             1.2.840.10008.1.2.1
//DeflatedExplicitVRLittleEndianTransferSyntax   1.2.840.10008.1.2.1.99 (*)
//BigEndianExplicitTransferSyntax                1.2.840.10008.1.2.2
//JPEGProcess1TransferSyntax                     1.2.840.10008.1.2.4.50
//JPEGProcess2_4TransferSyntax                   1.2.840.10008.1.2.4.51
//JPEGProcess6_8TransferSyntax                   1.2.840.10008.1.2.4.53
//JPEGProcess10_12TransferSyntax                 1.2.840.10008.1.2.4.55
//JPEGProcess14TransferSyntax                    1.2.840.10008.1.2.4.57
//JPEGProcess14SV1TransferSyntax                 1.2.840.10008.1.2.4.70
//(*) if compiled with zlib support enabled

//dcmdjpeg supports the following transfer syntaxes for output (dcmfile-out):

//LittleEndianImplicitTransferSyntax             1.2.840.10008.1.2
//LittleEndianExplicitTransferSyntax             1.2.840.10008.1.2.1
//BigEndianExplicitTransferSyntax                1.2.840.10008.1.2.2


//dcmcjpeg supports the following transfer syntaxes for input (dcmfile-in):

//LittleEndianImplicitTransferSyntax             1.2.840.10008.1.2
//LittleEndianExplicitTransferSyntax             1.2.840.10008.1.2.1
//DeflatedExplicitVRLittleEndianTransferSyntax   1.2.840.10008.1.2.1.99 (*)
//BigEndianExplicitTransferSyntax                1.2.840.10008.1.2.2
//JPEGProcess1TransferSyntax                     1.2.840.10008.1.2.4.50
//JPEGProcess2_4TransferSyntax                   1.2.840.10008.1.2.4.51
//JPEGProcess6_8TransferSyntax                   1.2.840.10008.1.2.4.53
//JPEGProcess10_12TransferSyntax                 1.2.840.10008.1.2.4.55
//JPEGProcess14TransferSyntax                    1.2.840.10008.1.2.4.57
//JPEGProcess14SV1TransferSyntax                 1.2.840.10008.1.2.4.70
//(*) if compiled with zlib support enabled

//dcmcjpeg supports the following transfer syntaxes for output (dcmfile-out):

//JPEGProcess1TransferSyntax                     1.2.840.10008.1.2.4.50
//JPEGProcess2_4TransferSyntax                   1.2.840.10008.1.2.4.51
//JPEGProcess6_8TransferSyntax                   1.2.840.10008.1.2.4.53
//JPEGProcess10_12TransferSyntax                 1.2.840.10008.1.2.4.55
//JPEGProcess14TransferSyntax                    1.2.840.10008.1.2.4.57
//JPEGProcess14SV1TransferSyntax                 1.2.840.10008.1.2.4.70


///
//未压缩：
//Transfer Syntax UID Transfer Syntax name
//Implicit VR Endian: Default Transfer Syntax for DICOM(默认，使用广)          1.2.840.10008.1.2
// Explicit VR Little Endian                                                   1.2.840.10008.1.2.1
//Deflated Explicit VR Little Endian                                           1.2.840.10008.1.2.1.99
// Explicit VR Big Endian                                                      1.2.840.10008.1.2.2
//压缩---JPEG和JPEG2000（有损和无损）
//有损：值中带Lossy
//Transfer Syntax UID Transfer Syntax name
//1.2.840.10008.1.2.4.50 JPEG Baseline (Process 1):
//Default Transfer Syntax for Lossy JPEG 8-bit Image Compression
//1.2.840.10008.1.2.4.51 JPEG Baseline (Processes 2 & 4):
//Default Transfer Syntax for Lossy JPEG 12-bit Image Compression
//(Process 4 only)
//1.2.840.10008.1.2.4.52 JPEG Extended (Processes 3 & 5)
//1.2.840.10008.1.2.4.53 JPEG Spectral Selection, Nonhierarchical (Processes 6 & 8)
//1.2.840.10008.1.2.4.54 JPEG Spectral Selection, Nonhierarchical (Processes 7 & 9)
//1.2.840.10008.1.2.4.55 JPEG Full Progression, Nonhierarchical (Processes 10 & 12)
//1.2.840.10008.1.2.4.56 JPEG Full Progression, Nonhierarchical (Processes 11 & 13)
//1.2.840.10008.1.2.4.57 JPEG Lossless, Nonhierarchical (Processes 14)
//1.2.840.10008.1.2.4.58 JPEG Lossless, Nonhierarchical (Processes 15)
//1.2.840.10008.1.2.4.59 JPEG Extended, Hierarchical (Processes 16 & 18)
//1.2.840.10008.1.2.4.60 JPEG Extended, Hierarchical (Processes 17 & 19)
//1.2.840.10008.1.2.4.61 JPEG Spectral Selection, Hierarchical (Processes 20 & 22)
//1.2.840.10008.1.2.4.62 JPEG Spectral Selection, Hierarchical (Processes 21 & 23)
//1.2.840.10008.1.2.4.63 JPEG Full Progression, Hierarchical (Processes 24 & 26)
//1.2.840.10008.1.2.4.64 JPEG Full Progression, Hierarchical (Processes 25 & 27)
//1.2.840.10008.1.2.4.65 JPEG Lossless, Nonhierarchical (Process 28)
//1.2.840.10008.1.2.4.66 JPEG Lossless, Nonhierarchical (Process 29)
//1.2.840.10008.1.2.4.70 JPEG Lossless, Nonhierarchical, First- Order Prediction
//(Processes 14 [Selection Value 1]):
//Default Transfer Syntax for Lossless JPEG Image Compression
//1.2.840.10008.1.2.4.80 JPEG-LS Lossless Image Compression
//DICOM Transfer Syntaxes
//1.2.840.10008.1.2.4.81 JPEG-LS Lossy (Near- Lossless) Image Compression
//1.2.840.10008.1.2.4.90 JPEG 2000 Image Compression (Lossless Only)
//1.2.840.10008.1.2.4.91 JPEG 2000 Image Compression
//1.2.840.10008.1.2.4.92 JPEG 2000 Part 2 Multicomponent Image Compression (Lossless Only)
//1.2.840.10008.1.2.4.93 JPEG 2000 Part 2 Multicomponent Image Compression
//Transfer Syntax UID Transfer Syntax name
//1.2.840.10008.1.2.4.94 JPIP Referenced
//1.2.840.10008.1.2.4.95 JPIP Referenced Deflate
//1.2.840.10008.1.2.5 RLE Lossless
//1.2.840.10008.1.2.6.1 RFC 2557 MIME Encapsulation
//Transfer Syntax UID Transfer Syntax name
//1.2.840.10008.1.2.4.100 MPEG2 Main Profile Main Level
//1.2.840.10008.1.2.4.102 MPEG-4 AVC/H.264 High Profile / Level 4.1
//1.2.840.10008.1.2.4.103 MPEG-4 AVC/H.264 BD-compatible High Profile / Level 4.1
///dcmdjpeg.cc dcmcjpeg.cc
///
/// https://dicomlibrary.com/dicom/transfer-syntax/
const QString PreviewNotAvailableText(QObject::tr("Preview image not available"));

QImage makeEmptyThumbnailWithCustomText(const QString &text, int resolution = 96)
{
    QImage thumbnail;

    thumbnail = QImage(resolution, resolution, QImage::Format_RGB32);
    thumbnail.fill(Qt::black);

    QPainter painter(&thumbnail);
    painter.setPen(Qt::white);
    painter.drawText(0, 0, resolution, resolution, Qt::AlignCenter | Qt::TextWordWrap, text);

    return thumbnail;
}

DcmDataset decompressImage( const DcmDataset *olddataset, E_TransferSyntax opt_oxfer = EXS_LittleEndianImplicit, QString derror = "")
{
    DcmFileFormat fileformat;
    // JPEG parameters
    E_DecompressionColorSpaceConversion opt_decompCSconversion = EDC_photometricInterpretation;
    E_UIDCreation opt_uidcreation = EUC_default;
    E_PlanarConfiguration opt_planarconfig = EPC_default;
    OFBool opt_predictor6WorkaroundEnable = OFFalse;
    OFBool opt_cornellWorkaroundEnable = OFFalse;
    //OFBool opt_forceSingleFragmentPerFrame = OFFalse;
    // register global decompression codecs
    DJDecoderRegistration::registerCodecs(
                opt_decompCSconversion,
                opt_uidcreation,
                opt_planarconfig,
                opt_predictor6WorkaroundEnable,
                opt_cornellWorkaroundEnable);
    OFCondition error = EC_Normal;

    //DcmDataset *dataset = new DcmDataset(*olddataset);
    DcmDataset dataset(*olddataset);
    //OFLOG_INFO(dcmdjpegLogger, "decompressing file");

    DcmXfer opt_oxferSyn(opt_oxfer);
    DcmXfer original_xfer(dataset.getOriginalXfer());

    error = dataset.chooseRepresentation(opt_oxfer, NULL);
    if (error.bad())
    {
        //ERROR_LOG(QString( error.text()) + " decompressing file: " + opt_ifname);
        derror = error.text();
        if (error == EJ_UnsupportedColorConversion)
        {
            //ERROR_LOG( "Try --conv-never to disable color space conversion");
            derror +=  "Try --conv-never to disable color space conversion";
        }
        else if (error == EC_CannotChangeRepresentation)
        {
            //ERROR_LOG( QString("Input transfer syntax ") +  original_xfer.getXferName() + "not supported");
            derror +=  QString("Input transfer syntax ") +  original_xfer.getXferName() + "not supported";
        }
        // deregister global decompression codecs
        DJDecoderRegistration::cleanup();
        return dataset;
    }

    if (!dataset.canWriteXfer(opt_oxfer))
    {
        // ERROR_LOG(QString ("no conversion to transfer syntax") + opt_oxferSyn.getXferName() + "possible");
        // deregister global decompression codecs
        DJDecoderRegistration::cleanup();
        QMessageBox::warning(NULL,"warning!","conversion to transfer syntax fail!" + QString(opt_oxferSyn.getXferName()));
        return dataset;
    }

    // deregister global decompression codecs
    DJDecoderRegistration::cleanup();

    return  dataset;
}

QPixmap convertToQPixmap(DicomImage *dicomImage)
{
    Q_ASSERT(dicomImage);

    // The following code creates a PGM or PPM image in memory and we load this buffer directly into the QImage
    // Based on the code of http://forum.dcmtk.org/viewtopic.php?t=120&highlight=qpixmap
    int bytesPerComponent;
    QString imageFormat;
    QString imageHeader;
    if (dicomImage->isMonochrome())
    {
        imageHeader = "P5";
        bytesPerComponent = 1;
        imageFormat = "PGM";
    }
    else
    {
        imageHeader = "P6";
        bytesPerComponent = 3;
        imageFormat = "PPM";
    }

    // Create PGM/PPM header
    const int width = (int)(dicomImage->getWidth());
    const int height = (int)(dicomImage->getHeight());
    imageHeader += QString("\n%1 %2\n255\n").arg(width).arg(height);

    // QPixmap in which we will load the data buffer
    QPixmap thumbnail;
    // Create output buffer for DicomImage class
    const int offset = imageHeader.size();
    const unsigned int length = (width * height) * bytesPerComponent + offset;
    Uint8 *buffer = new Uint8[length];
    if (buffer != NULL)
    {
        // Copy PGM/PPM header to buffer
        OFBitmanipTemplate<Uint8>::copyMem((const Uint8*)imageHeader.toLatin1().data(), buffer, offset);
        if (dicomImage->getOutputData((void*)(buffer + offset), length, 8))
        {
            if (!thumbnail.loadFromData((const unsigned char*)buffer, length, imageFormat.toLatin1()))
            {
                // DEBUG_LOG("Buffer upload to thumbnail failed :(");
            }
            //thumbnail.save("E:/test.png");
        }
        // Delete temporary pixel buffer
        delete[] buffer;
    }
    else
    {
        //DEBUG_LOG("Insufficient memory to create thumbnail buffer!");
    }

    return thumbnail;
}

QImage dcm2QImage(DicomImage *dicomImage, int transferSyntax = 0, int resolution = 256)
{
    QImage image;
    QImage thumbnail;
    bool ok = false;
    if (dicomImage == NULL)
    {
        ok = false;
        //DEBUG_LOG("Insufficient memory to load DICOM image when making thumbnail or null pointer");
    }
    else if (dicomImage->getStatus() == EIS_Normal)
    {
        dicomImage->hideAllOverlays();
        dicomImage->setMinMaxWindow(1);
        // We scale the image
        DicomImage *scaledImage;
        // We climb the biggest corner
        unsigned long width, height;
        if (dicomImage->getWidth() < dicomImage->getHeight())
        {
            width = 0;
            height = resolution;
        }
        else
        {
            width = resolution;
            height = 0;
        }
        scaledImage = dicomImage->createScaledImage(width, height, 1, 1);
        if (scaledImage == NULL)
        {
            ok = false;
            //DEBUG_LOG("Scaled image returned as null");
        }
        else if (scaledImage->getStatus() == EIS_Normal)
        {
            QPixmap pixmap = convertToQPixmap(scaledImage);
            if (pixmap.isNull())
            {
                //DEBUG_LOG("Could not convert DicomImage to QImage. A Preview not available thumbnail is created.");
                ok = false;
            }
            else
            {
                // The smallest side will be of "resolution" size.
                pixmap = pixmap.scaled(resolution,resolution, Qt::AspectRatioMode::KeepAspectRatioByExpanding, Qt::TransformationMode::SmoothTransformation);
                // By cropping the longer side, a squared image is made.
                int width = pixmap.width();
                int height = pixmap.height();
                if (width > height) // heigth == resolution
                {
                    pixmap = pixmap.copy((width-resolution) / 2, 0, height, height);
                }
                else if (height > width) // width == resolution
                {
                    pixmap = pixmap.copy(0, (height-resolution) / 2, width, width);
                }
                else
                {
                    // A perfect square, nothing to do
                }
                thumbnail = pixmap.toImage();
                ok = true;
            }

            // DicomImage must be deleted to avoid memory leaks
            delete scaledImage;
        }
        else
        {
            ok = false;
            //DEBUG_LOG(QString("The scaled image has errors. Error: %1 ").arg(DicomImage::getString(scaledImage->getStatus())));
        }
    }
    else
    {
        ok = false;
        //DEBUG_LOG(QString("Error loading the DicomImage. Error: %1 ").arg(DicomImage::getString(dicomImage->getStatus())));
    }

    //If we were unable to generate the thumbnail, we create a blank one
    if (!ok)
    {
        thumbnail = makeEmptyThumbnailWithCustomText(PreviewNotAvailableText);
    }

    return thumbnail;
}

void MainWindow::on_dcm2Image_clicked()
{
    m_TransferSyntax = ui->cbmTransferSyntax->currentIndex();
    QFileInfo info(m_dcmpath);
    if (info.isFile())
    {
        QString dcmfilename = m_dcmpath;
        QString pngfilename = m_dcmpath+".png";
        ///------------------------------------------------------------
        DicomImage *dicomImage = NULL;
        DicomImage *decodeImage = NULL;
        DcmFileFormat fileformat;
        DicomImage reader(dcmfilename.toLatin1().data());
        E_TransferSyntax opt_ixfer = EXS_LittleEndianImplicit;
        OFCondition error = EC_Normal;
        error = fileformat.loadFile(dcmfilename.toLatin1().data()/*, opt_ixfer, EGL_noChange, DCM_MaxReadLength, opt_readMode*/);
        if (error.bad())
        {
            QMessageBox::warning(this,"warning!"," loadFile dcm fail!");
            return;
        }
        DcmXfer original_xfer(fileformat.getDataset()->getOriginalXfer());
        if (!original_xfer.isEncapsulated())
        {
            dicomImage = &reader;
        }
        else
        {
            switch(m_TransferSyntax-1)
            {
            case 1:
                opt_ixfer = EXS_LittleEndianImplicit;
                break;
            case 2:
                opt_ixfer = EXS_BigEndianImplicit;
                break;
            case 3:
                opt_ixfer = EXS_LittleEndianExplicit;
                break;
            default:
                break;
            }
            DcmDataset newdataset(decompressImage(fileformat.getDataset(), opt_ixfer));
            if (!newdataset.isEmpty())
            {
                dicomImage = new DicomImage(&newdataset, newdataset.getCurrentXfer());
                decodeImage = dicomImage;
            }
        }
        ///------------------------------------------------------------------------
        QImage image = dcm2QImage(dicomImage,opt_ixfer);

        image.save(pngfilename+"dcmtk.png");
        ui->lbshowimage->clear();
        ui->lbshowimage->setPixmap(QPixmap(pngfilename+"dcmtk.png"));
        if (decodeImage)
        {
            delete  decodeImage;
            decodeImage = NULL;
        }
    }else
    {
        QMessageBox::warning(this,"warning!","the dcm not exit!");
    }

}

void MainWindow::on_pBdcmpath_clicked()
{
    m_dcmpath = QFileDialog::getOpenFileName(this,tr("open a file."),"",tr("dcm file(*.dcm);All files(*.*)"/*,nullptr,QFileDialog::DontUseNativeDialog*/));
    if (m_dcmpath.isEmpty())
    {
        QMessageBox::warning(this,"warning!","Failed to open the dcm");
    }
    else
    {
        ui->dcmpath->setText(m_dcmpath);
        ui->pngpath->setText(m_dcmpath+"dcmtk.png");
    }
}


void MainWindow::on_decoder_clicked()
{
    m_TransferSyntax = ui->cbmTransferSyntax->currentIndex();
    QFileInfo info(m_dcmpath);
    if (info.isFile())
    {
        QString dcmfilename = m_dcmpath;
        DcmFileFormat fileformat;
        fileformat.loadFile(dcmfilename.toLatin1().data());
        DcmXfer original_xfer(fileformat.getDataset()->getOriginalXfer());
        if (!original_xfer.isEncapsulated())
        {
            QMessageBox::warning(this,"warning!","is not JPEG-compressed DICOM file!");
            return;
        }
        DcmDataset newdataset(decompressImage(fileformat.getDataset()));

        if (newdataset.isEmpty())
        {
            QMessageBox::warning(this,"warning!","decoderDcm fail!");
        }
        else
        {
            QString newfilepath = dcmfilename+"_d_LI.dcm";
            DcmFileFormat newDcmFile(&newdataset);
            fileformat.loadAllDataIntoMemory();
            E_EncodingType opt_oenctype = EET_ExplicitLength;
            E_GrpLenEncoding opt_oglenc = EGL_recalcGL;
            E_PaddingEncoding opt_opadenc = EPD_noChange;
            OFCmdUnsignedInt opt_filepad = 0;
            OFCmdUnsignedInt opt_itempad = 0;
            E_FileWriteMode opt_writeMode = EWM_createNewMeta;
            OFCondition error = EC_Normal;

            E_TransferSyntax opt_ixfer = EXS_LittleEndianImplicit;
            switch(m_TransferSyntax)
            {
            case 1:
                opt_ixfer = EXS_LittleEndianImplicit;
//                newfilepath = dcmfilename+"_d_LI.dcm";
                break;
            case 2:
                opt_ixfer = EXS_BigEndianImplicit;
                newfilepath = dcmfilename+"_d_BI.dcm";
                break;
            case 3:
                opt_ixfer = EXS_LittleEndianExplicit;
                newfilepath = dcmfilename+"_d_LE.dcm";
                break;
            default:
                break;
            }
            error = newDcmFile.saveFile( newfilepath.toLatin1().data(), opt_ixfer, opt_oenctype, opt_oglenc, opt_opadenc, OFstatic_cast(Uint32, opt_filepad),
                                         OFstatic_cast(Uint32, opt_itempad), opt_writeMode);
            if (error != EC_Normal)
            {
                //OFLOG_FATAL(dcmdjpegLogger, error.text() << ": writing file: " <<  opt_ofname);
                QMessageBox::warning(this,"warning!","decoderDcm save fail!" + newfilepath);
                return ;
            }
            QMessageBox::information(this,"ok!","decoderDcm ok!");
        }

    }else
    {
        QMessageBox::warning(this,"warning!","the dcm not exit!");
    }
}

void MainWindow::on_pBshowimage_clicked()
{
    QString pngfile = ui->pngpath->text();
    QFileInfo info(pngfile);
    if (info.isFile())
    {
        ui->lbshowimage->clear();
        ui->lbshowimage->setPixmap(QPixmap(pngfile));
    }
    else
    {
        QMessageBox::warning(this,"warning!","file not exit!");
    }
}

void MainWindow::on_cbmTransferSyntax_currentIndexChanged(int index)
{
    m_TransferSyntax = ui->cbmTransferSyntax->currentIndex();
}
