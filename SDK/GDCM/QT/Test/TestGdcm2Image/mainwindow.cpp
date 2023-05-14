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
}

MainWindow::~MainWindow()
{
    delete ui;
}

//未压缩：
//Implicit VR Little Endian:
//Default Transfer Syntax for DICOM               1.2.840.10008.1.2
//Explicit VR Little Endian                       1.2.840.10008.1.2.1
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

bool ConvertToFormat_RGB888(gdcm::Image const & gimage, char *buffer, QImage* &imageQt)
{
    const unsigned int* dimension = gimage.GetDimensions();
    unsigned int dimX = dimension[0];
    unsigned int dimY = dimension[1];
    if (!gimage.GetBuffer(buffer))
    {
        return  false;
    }
    if (gimage.IsLossy())
    {
        printf("IsLossy\n");
    }
    int row = gimage.GetRows();
    int col = gimage.GetColumns();
    printf("row: %d col: %d\n", row, col);
    // Let's start with the easy case:
    if( gimage.GetPhotometricInterpretation() == gdcm::PhotometricInterpretation::RGB )
    {
        if( gimage.GetPixelFormat() != gdcm::PixelFormat::UINT8 )
        {
            return false;
        }
        unsigned char *ubuffer = (unsigned char*)buffer;
        // QImage::Format_RGB888  13  The image is stored using a 24-bit RGB format (8-8-8).
        imageQt = new QImage((unsigned char *)ubuffer, dimX, dimY, 3*dimX, QImage::Format_RGB888);
    }
    else if( gimage.GetPhotometricInterpretation() == gdcm::PhotometricInterpretation::MONOCHROME2 )
    {
        if( gimage.GetPixelFormat() == gdcm::PixelFormat::UINT8 )
        {
            // We need to copy each individual 8bits into R / G and B:
            unsigned char *ubuffer = new unsigned char[dimX*dimY*3];
            unsigned char *pubuffer = ubuffer;
            for(unsigned int i = 0; i < dimX*dimY; i++)
            {
                *pubuffer++ = *buffer;
                *pubuffer++ = *buffer;
                *pubuffer++ = *buffer++;
                //printf("Pixel: %d ",*pubuffer);
            }

            imageQt = new QImage(ubuffer, dimX, dimY, QImage::Format_RGB888);
        }
        else if( gimage.GetPixelFormat() == gdcm::PixelFormat::INT16 )
        {
            // We need to copy each individual 16bits into R / G and B (truncate value)
            short *buffer16 = (short*)buffer;
            unsigned char *ubuffer = new unsigned char[dimX*dimY*3];
            unsigned char *pubuffer = ubuffer;
            for(unsigned int i = 0; i < dimX*dimY; i++)
            {
                // Scalar Range of gdcmData/012345.002.050.dcm is [0,192], we could simply do:
                // *pubuffer++ = *buffer16;
                // *pubuffer++ = *buffer16;
                // *pubuffer++ = *buffer16;
                // instead do it right:
                *pubuffer++ = (unsigned char)std::min(255, (32768 + *buffer16) / 255);
                *pubuffer++ = (unsigned char)std::min(255, (32768 + *buffer16) / 255);
                *pubuffer++ = (unsigned char)std::min(255, (32768 + *buffer16) / 255);
                //printf("Pixel: %s ",(*buffer16));
                buffer16++;
            }

            imageQt = new QImage(ubuffer, dimX, dimY, QImage::Format_RGB888);
        }
        else
        {
            std::cerr << "Pixel Format is: " << gimage.GetPixelFormat() << std::endl;
            return false;
        }
    }
    else
    {
        std::cerr << "Unhandled PhotometricInterpretation: " << gimage.GetPhotometricInterpretation() << std::endl;
        return false;
    }

    return true;
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
const QString PreviewNotAvailableText(QObject::tr("Preview image not available"));

DcmObject* decompressImage(DcmFileFormat &fileformat, QString imageFileName)
{
    DcmObject *dcmimage = NULL;

    // JPEG parameters
    E_DecompressionColorSpaceConversion opt_decompCSconversion = EDC_photometricInterpretation;
    E_UIDCreation opt_uidcreation = EUC_default;
    E_PlanarConfiguration opt_planarconfig = EPC_default;
    OFBool opt_predictor6WorkaroundEnable = OFFalse;
    OFBool opt_cornellWorkaroundEnable = OFFalse;
    OFBool opt_forceSingleFragmentPerFrame = OFFalse;
    // register global decompression codecs
    DJDecoderRegistration::registerCodecs(
                opt_decompCSconversion,
                opt_uidcreation,
                opt_planarconfig,
                opt_predictor6WorkaroundEnable,
                opt_cornellWorkaroundEnable,
                opt_forceSingleFragmentPerFrame);
    OFCondition error = EC_Normal;

    const char *opt_ifname = imageFileName.toLatin1().data();
    //const char *opt_ofname = (imageFileName+"_djpeg").toLatin1().data();
    QString output = imageFileName+"_djpeg.dcm";
    E_TransferSyntax opt_ixfer = EXS_JPEGProcess14;//EXS_Unknown;
    E_FileReadMode opt_readMode = ERM_dataset;//ERM_autoDetect;
    error = fileformat.loadFile(imageFileName.toLatin1().data(), opt_ixfer, EGL_noChange, DCM_MaxReadLength, opt_readMode);
    if (error.bad())
    {
        //OFLOG_FATAL(dcmdjpegLogger, error.text() << ": reading file: " <<  opt_ifname);
        // deregister global decompression codecs
        DJDecoderRegistration::cleanup();
        return dcmimage;
    }

    DcmDataset *dataset = fileformat.getDataset();

    //OFLOG_INFO(dcmdjpegLogger, "decompressing file");

    E_TransferSyntax opt_oxfer = EXS_LittleEndianImplicit;
    //    if (cmd.findOption("--write-xfer-little")) opt_oxfer = EXS_LittleEndianExplicit;
    //    if (cmd.findOption("--write-xfer-big")) opt_oxfer = EXS_BigEndianExplicit;
    //    if (cmd.findOption("--write-xfer-implicit")) opt_oxfer = EXS_LittleEndianImplicit;
    DcmXfer opt_oxferSyn(opt_oxfer);
    DcmXfer original_xfer(dataset->getOriginalXfer());

    error = dataset->chooseRepresentation(opt_oxfer, NULL);
    if (error.bad())
    {
        printf("%s decompressing file: %s", error.text(),opt_ifname);
        if (error == EJ_UnsupportedColorConversion)
            printf( "Try --conv-never to disable color space conversion");
        else if (error == EC_CannotChangeRepresentation)
            printf( "Input transfer syntax %s not supported", original_xfer.getXferName());
        // deregister global decompression codecs
        DJDecoderRegistration::cleanup();
        return dcmimage;
    }

    if (! dataset->canWriteXfer(opt_oxfer))
    {
        printf("no conversion to transfer syntax %s possible" , opt_oxferSyn.getXferName());
        // deregister global decompression codecs
        DJDecoderRegistration::cleanup();
        return dcmimage;
    }

    printf("creating output file %s",  output.toLatin1().data()/*opt_ofname*/);

    fileformat.loadAllDataIntoMemory();
    // deregister global decompression codecs
    DJDecoderRegistration::cleanup();
    dcmimage = fileformat.clone();
    return  dcmimage;
}

bool decoderDcm(const QString &imageFileName, int resolution = 96)
{
    // JPEG parameters
    E_DecompressionColorSpaceConversion opt_decompCSconversion = EDC_photometricInterpretation;
    E_UIDCreation opt_uidcreation = EUC_default;
    E_PlanarConfiguration opt_planarconfig = EPC_default;
    OFBool opt_predictor6WorkaroundEnable = OFFalse;
    OFBool opt_cornellWorkaroundEnable = OFFalse;
    OFBool opt_forceSingleFragmentPerFrame = OFFalse;
    // register global decompression codecs
    DJDecoderRegistration::registerCodecs(opt_decompCSconversion,  opt_uidcreation,  opt_planarconfig,
                                          opt_predictor6WorkaroundEnable,  opt_cornellWorkaroundEnable, opt_forceSingleFragmentPerFrame);
    OFCondition error = EC_Normal;

    DcmFileFormat fileformat;
    const char *opt_ifname = imageFileName.toLatin1().data();
    //const char *opt_ofname = (imageFileName+"_djpeg").toLatin1().data();
    QString output = imageFileName+"_djpeg.dcm";
    E_TransferSyntax opt_ixfer = EXS_JPEGProcess14;//EXS_Unknown;
    E_FileReadMode opt_readMode = ERM_dataset;//ERM_autoDetect;
    error = fileformat.loadFile(imageFileName.toLatin1().data(), opt_ixfer, EGL_noChange, DCM_MaxReadLength, opt_readMode);
    if (error.bad())
    {
        //OFLOG_FATAL(dcmdjpegLogger, error.text() << ": reading file: " <<  opt_ifname);
        return false;
    }

    DcmDataset *dataset = fileformat.getDataset();

    //OFLOG_INFO(dcmdjpegLogger, "decompressing file");

    //    EXS_Unknown -1
    //    EXS_LittleEndianImplicit  0
    //    EXS_BigEndianImplicit     1
    //    EXS_LittleEndianExplicit  2
    E_TransferSyntax opt_oxfer = EXS_LittleEndianImplicit;
    DcmXfer opt_oxferSyn(opt_oxfer);
    DcmXfer original_xfer(dataset->getOriginalXfer());

    error = dataset->chooseRepresentation(opt_oxfer, NULL);
    if (error.bad())
    {
        printf("%s decompressing file: %s", error.text(),opt_ifname);
        if (error == EJ_UnsupportedColorConversion)
            printf( "Try --conv-never to disable color space conversion");
        else if (error == EC_CannotChangeRepresentation)
            printf( "Input transfer syntax %s not supported", original_xfer.getXferName());
        return false;
    }

    if (! dataset->canWriteXfer(opt_oxfer))
    {
        printf("no conversion to transfer syntax %s possible" , opt_oxferSyn.getXferName());
        return false;
    }

    printf("creating output file %s",  output.toLatin1().data()/*opt_ofname*/);

    fileformat.loadAllDataIntoMemory();
    E_EncodingType opt_oenctype = EET_ExplicitLength;
    E_GrpLenEncoding opt_oglenc = EGL_recalcGL;
    E_PaddingEncoding opt_opadenc = EPD_noChange;
    OFCmdUnsignedInt opt_filepad = 0;
    OFCmdUnsignedInt opt_itempad = 0;
    E_FileWriteMode opt_writeMode = EWM_createNewMeta;
    error = fileformat.saveFile(/*opt_ofname*/ output.toLatin1().data(), opt_oxfer, opt_oenctype, opt_oglenc, opt_opadenc, OFstatic_cast(Uint32, opt_filepad),
                                OFstatic_cast(Uint32, opt_itempad), opt_writeMode);
    if (error != EC_Normal)
    {
        printf("%s : writing file: %s", error.text() , /*opt_ofname*/ output.toLatin1().data());
        return false;
    }

    printf("conversion successful \n");

    // deregister global decompression codecs
    DJDecoderRegistration::cleanup();
    return  true;
}

QImage createQImage(/*DicomImage *dicomImage*/const QString &imageFileName, int resolution = 256)
{
    QImage image;
    DcmObject *obj = NULL;

    DicomImage *dicomImage = NULL;

    DcmFileFormat fileformat;
    DicomImage reader(imageFileName.toLatin1().data());
    E_TransferSyntax opt_ixfer = EXS_LittleEndianImplicit;//EXS_LittleEndianExplicit;//EXS_JPEGProcess14;//EXS_Unknown;
    //E_FileReadMode opt_readMode = ERM_dataset;//ERM_autoDetect;
    OFCondition error = EC_Normal;
    error = fileformat.loadFile(imageFileName.toLatin1().data()/*, opt_ixfer, EGL_noChange, DCM_MaxReadLength, opt_readMode*/);
    if (error.bad())
    {
        //return image;
    }

    DcmXfer original_xfer(fileformat.getDataset()->getOriginalXfer());
    if (!original_xfer.isEncapsulated())
    {
        dicomImage = &reader;
    }
    else
    {
        obj =  decompressImage(fileformat,imageFileName);
        //DicomImage reader(obj, opt_ixfer);
        dicomImage = new DicomImage(obj,opt_ixfer);
    }

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
    if (obj)
    {
        delete  obj;
        obj = NULL;
        delete dicomImage;
        dicomImage = NULL;
    }
    return thumbnail;
}


QImage* GetQimage(QString input)
{
    QImage *imageQt = NULL;
    gdcm::ImageReader ir;
    ir.SetFileName( input.toLatin1().data());
    if(!ir.Read())
    {
        //Read failed
        return imageQt;
    }

    std::cout<<"Getting image from ImageReader..."<<std::endl;

    const gdcm::Image &gimage = ir.GetImage();
    std::vector<char> vbuffer;
    vbuffer.resize( gimage.GetBufferLength() );
    char *buffer = &vbuffer[0];


    if( !ConvertToFormat_RGB888( gimage, buffer, imageQt ) )
    {
        return NULL/*imageQt*/;
    }

    return imageQt;
}

void MainWindow::on_gdcm2Image_clicked()
{
    QFileInfo info(m_dcmpath);
    if (info.isFile())
    {
        QString dcmfilename = m_dcmpath;
        QString pngfilename = m_dcmpath+".png";
        QImage image =  createQImage(dcmfilename);
        image.save(pngfilename+"dcmtk.png");
        ui->lbshowimage->clear();
        ui->lbshowimage->setPixmap(QPixmap(pngfilename+"dcmtk.png"));

    }else
    {
        QMessageBox::warning(this,"warning!","the dcm not exit!");
    }

}

void MainWindow::on_pBdcmpath_clicked()
{
    m_dcmpath = QFileDialog::getOpenFileName(this,tr("open a file."),"",tr("dcm file(*.dcm);All files(*.*)"));
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
    QFileInfo info(m_dcmpath);
    if (info.isFile())
    {
        QString dcmfilename = m_dcmpath;
        if (!decoderDcm(dcmfilename))
        {
            QMessageBox::warning(this,"warning!","decoderDcm fail!");
        }
        else
        {
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
