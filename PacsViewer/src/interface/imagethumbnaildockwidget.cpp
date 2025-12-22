#include "imagethumbnaildockwidget.h"
#include "volume.h"
#include "anatomicalplane.h"
#include "series.h"
#include "study.h"
#include "image.h"
#include "qapplicationmainwindow.h"
#include "patient.h"
#include "VolumeRepository.h"
#include "ExtensionWorkspace.h"
#include "extensionmediatorfactory.h"

//#include <QPainter>
#include <QSize>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDir>
#include <QVBoxLayout>
#include <QPalette>
#include <QMessageBox>
namespace udg {

const int g_DockWidgetwith = 120;
const QSize IMAGE_SIZE(96,96);
const QSize ITEM_SIZE(109,140);

ImageThumbnailDockWidget::ImageThumbnailDockWidget(const QString &title,QApplicationMainWindow *mainApp, QWidget *parent,
                                                   Qt::WindowFlags flags ):QDockWidget(title, parent, flags)
{
    m_mainApp = mainApp;

    //m_background.setColor(QPalette::Background,Qt::black);
    //setAutoFillBackground(true);
    //setPalette(m_background);
    m_ImagelistWidge = new QListWidget(this);
    m_ImagelistWidge->setIconSize(IMAGE_SIZE);
    m_ImagelistWidge->setResizeMode(QListView::Adjust);
    m_ImagelistWidge->setViewMode(QListView::IconMode);
    m_ImagelistWidge->setMovement(QListView::Static);
    m_ImagelistWidge->setSelectionRectVisible(true);
    m_ImagelistWidge->setSpacing(0);
    m_ImagelistWidge->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);//always show!!?? set
    m_ImagelistWidge->setWordWrap(true);
	m_ImagelistWidge->setAutoScroll(true);
	//m_ImagelistWidge->setItemWidget()
    //QPalette palette;
    //palette.setColor(QPalette::Background, Qt::red);
    //m_ImagelistWidge->setPalette(palette);
	//setWidget(m_ImagelistWidge);
    //m_mainlayout = new QHBoxLayout;
    //m_mainlayout->addWidget(m_ImagelistWidge);
    //m_mainlayout->setMargin(0);
	//this->setContentsMargins(0, 0, 0, 0);
    //setLayout(m_mainlayout);
	m_ImagelistWidge->setItemAlignment(Qt::AlignCenter);
    setMinimumWidth(g_DockWidgetwith);
    setMaximumWidth(g_DockWidgetwith);

    m_lastExtension = 0;
	m_ImagelistWidge->setStyleSheet("border: 0px; background-color:lightgray");
	//this->setStyleSheet("border: 0px; background-color:lightgray");
}

void ImageThumbnailDockWidget::clearThumbmailList()
{
    if (m_ImagelistWidge)
    {
        int itemcount = m_ImagelistWidge->count();
        if (itemcount > 0)
        {
            for(int index=0;index<itemcount;index++)
            {
                QListWidgetItem *item =m_ImagelistWidge->takeItem(0);// auto change /first item!!!
                delete item;
            }

            //update
            QRect rect = this->rect();
            m_ImagelistWidge->setGeometry(rect.left()+1,rect.top()+1,rect.right()-1,rect.bottom()-1);
        }
    }
    m_patientsList.clear();
}
void ImageThumbnailDockWidget::addPatientsThumbmailList(QList<Patient*> patientsList)
{
    clearThumbmailList();
    m_patientsList<<patientsList;
    QString caption;
    QString label;
    // For each series of the study we will extract its label and identifier
    QList<QPair<QString, QString> > itemsList;
    QList<QPair<QString, QString> > fusionItemsList;
    foreach(Patient *patient, m_patientsList)
    {
        foreach (Study *study, patient->getStudies())
        {
            // We extract the caption from the study
            caption = tr("Study %1 %2 [%3] %4")
                    .arg(study->getDate().toString(Qt::ISODate))
                    .arg(study->getTimeAsString())
                    .arg(study->getModalitiesAsSingleString())
                    .arg(study->getDescription());

            //// For each series of the study we will extract its label and identifier
            //QList<QPair<QString, QString> > itemsList;
            //QList<QPair<QString, QString> > fusionItemsList;
            foreach (Series *series, study->getViewableSeries())
            {
                label = tr("Se:%1 %3%4%5(%6 Ims)")//tr("Series %1: %2%3%4%5(%6 Images)")
                        .arg(series->getSeriesNumber().trimmed())
                        //.arg(series->getProtocolName().trimmed())
                        .arg(series->getDescription().trimmed())
                        .arg(series->getBodyPartExamined())
                        .arg(series->getViewPosition())
                        .arg(series->getNumberOfImages());

                int volumeNumber = 1;
                foreach (Volume *volume, series->getVolumesList())
                {
                    QPair<QString, QString> itemPair;
                    // Label
                    if (series->getNumberOfVolumes() > 1)
                    {
                        itemPair.first = label + " (" + QString::number(volumeNumber) + ")";
                    }
                    else
                    {
                        itemPair.first = label;
                    }
                    volumeNumber++;
                    // Identifier !!!!!!
                    itemPair.second = QString::number(volume->getIdentifier().getValue());
                    // We add the pair to the list
                    itemsList << itemPair;
                    if (series->getModality() == "CT" && !series->isCTLocalizer())
                    {
                        AnatomicalPlane acquisitionPlane = volume->getAcquisitionPlane();

                        if (acquisitionPlane != AnatomicalPlane::NotAvailable)
                        {
                            int zIndex = volume->getCorrespondingOrthogonalPlane(acquisitionPlane).getZIndex();
                            double margin = series->getImages().first()->getSliceThickness() * 5;

                            double range1[2];
                            range1[0] = volume->getImages().first()->getImagePositionPatient()[zIndex];
                            range1[1] = volume->getImages().last()->getImagePositionPatient()[zIndex];

                            foreach (Series * secondSeries, study->getViewableSeries())
                            {
                                if ((secondSeries->getModality() == "PT" || secondSeries->getModality() == "NM") && series->getFrameOfReferenceUID() == secondSeries->getFrameOfReferenceUID())
                                {
                                    foreach (Volume *secondVolume, secondSeries->getVolumesList())
                                    {
                                        if (secondVolume->getAcquisitionPlane() == acquisitionPlane)
                                        {
                                            double range2[2];
                                            range2[0] = secondVolume->getImages().first()->getImagePositionPatient()[zIndex];
                                            range2[1] = secondVolume->getImages().last()->getImagePositionPatient()[zIndex];

                                            if ((range1[0] + margin > range2[0] && range1[1] - margin < range2[1]) || (range2[0] + margin > range1[0] && range2[1] - margin < range1[1]))
                                            {
                                                QPair<QString, QString> itemPair;
                                                // Label
                                                itemPair.first = QString("%1 + %2").arg(series->getProtocolName().trimmed() + series->getDescription().trimmed()).arg(secondSeries->getProtocolName().trimmed() + secondSeries->getDescription().trimmed());
                                                // Identifier
                                                itemPair.second = QString("%1+%2").arg(volume->getIdentifier().getValue()).arg(secondVolume->getIdentifier().getValue());
                                                //We add the pair to the list
                                                fusionItemsList << itemPair;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            itemsList << fusionItemsList;
            // We add the series grouped by study
            //m_patientBrowserList->addItemsGroup(caption, itemsList << fusionItemsList);
        }
    }
    typedef QPair<QString, QString> DefPair;
    int index = 0;
    foreach (DefPair itemPair, itemsList)
    {
        ///--------------------------------------------------------------------------------
        /// item
        Identifier id(itemPair.second.toInt());
        QPixmap pixmap(VolumeRepository::getRepository()->getVolume(id)->getThumbnail());
        QListWidgetItem *item = new QListWidgetItem(QIcon(pixmap.scaled(IMAGE_SIZE)),itemPair.first);
        item->setSizeHint(ITEM_SIZE);
        item->setWhatsThis(itemPair.second); // set Volume ( QString id = item->whatsThis())->updateActiveItemView
        item->setToolTip(itemPair.first);
        m_ImagelistWidge->insertItem(index++,item);

		//https://blog.csdn.net/abcd552191868/article/details/110147743
		//QWidget *itemWidge = new QWidget(m_ImagelistWidge);
		//QLabel *lblPicture = new QLabel("");
		//QListWidgetItem *listItem = new QListWidgetItem(m_ImagelistWidge);
		//QGridLayout *itemLayout = new QGridLayout;
		//QHBoxLayout *itemContentLayout = new QHBoxLayout;
		//lblPicture->setPixmap((pixmap.scaled(IMAGE_SIZE)));
		//itemLayout->addWidget(lblPicture);
		//itemWidge->setLayout(itemLayout);
		////itemWidge->setSizeHint(ITEM_SIZE);
		//itemWidge->setWhatsThis(itemPair.second); // set Volume ( QString id = item->whatsThis())->updateActiveItemView
		//itemWidge->setToolTip(itemPair.first);
		//m_ImagelistWidge->setItemWidget(item,itemWidge);

    }
    //????-------->  m_mainApp->getExtensionWorkspace()->addApplication(extension, requestedExtensionLabel, who);
    connect(m_ImagelistWidge, SIGNAL(itemDoubleClicked(QListWidgetItem *)),this, SLOT(updateActiveItemView(QListWidgetItem*)));
}

ImageThumbnailDockWidget::~ImageThumbnailDockWidget()
{
    clearThumbmailList();
}

void ImageThumbnailDockWidget::updateActiveItemView(QListWidgetItem *item)
{
    QWidget* widget = m_mainApp->currentWidgetOfExtensionWorkspace();
    QString className = widget->metaObject()->className();
    QString str = className.section("::",1,1);
    QString id = item->whatsThis();
    ExtensionMediator *mediator = ExtensionMediatorFactory::instance()->create(str);
    if (mediator)
    {
        if (widget)
        {
			Volume *volume = VolumeRepository::getRepository()->getVolume(Identifier(id.toInt()));
			ExtensionWorkspace *extensionWorkspace = m_mainApp->getExtensionWorkspace();
			int extensionIndex = extensionWorkspace->currentIndex();
			if (extensionWorkspace->tabText(extensionIndex).contains("3D-Viewer"))
			{
				if (!volume)
				{
					QMessageBox::warning(0, "3D-Viewer", ("3D-Viewer: No image is selected!!"));
					delete mediator;
					return ;
				}
				if (!volume->is3Dimage())
				{
					QMessageBox::warning(0, "3D-Viewer", ("The selected item : 3D-Viewer fail!!! images < 5 or SliceThickness = 0.0"));
					delete mediator;
					return;
				}

				extensionWorkspace->setTabText(extensionIndex, "3D-Viewer#Series:" + volume->getSeries()->getSeriesNumber());
			}
			else if (extensionWorkspace->tabText(extensionIndex).contains("MPR"))
			{
				if (!volume)
				{
					QMessageBox::warning(0, "MPR-3D-Viewer", ("MPR-3D-Viewer: No image is selected!!"));
					delete mediator;
					return;
				}
				if (!volume->is3Dimage())
				{
					QMessageBox::warning(0, "MPR-3D-Viewer", ("The selected item : MPR-3D-Viewer fail!!! images < 5 or SliceThickness = 0.0"));
					delete mediator;
					return;
				}

				//extensionWorkspace->setTabText(extensionIndex, "3D-Viewer#Series:" + volume->getSeries()->getSeriesNumber());
			}
            mediator->executionCommand(widget, volume);
        }
        else
        {
            QMessageBox::warning(NULL, "extension NULL!", id);
        }
        delete mediator;
    }

}

void  ImageThumbnailDockWidget::refreshTab(int index)
{
    //QMessageBox::about(NULL, "refreshTab", QString(index));
}

//QSize ImageThumbnailDockWidget::minimumSizeHint() const
//{
//    QDockWidget *dw = qobject_cast<QDockWidget*>(parentWidget());
//    Q_ASSERT(dw);

//    QSize result(m_pushpin.width(),m_pushpin.height());
//    if (dw->features() & QDockWidget::DockWidgetVerticalTitleBar)
//    {
//        result.transpose();
//    }

//    return result;
//}

void ImageThumbnailDockWidget::paintEvent(QPaintEvent*)
{
    QRect rect = this->rect();
    m_ImagelistWidge->setGeometry(rect.left(),rect.top(),rect.right(),rect.bottom());

	//QPainter painter(this);
	//painter.setRenderHint(QPainter::Antialiasing);
	//QRect bottonRect(rect.right() - 15, rect.bottom() / 2 - 30, rect.right(), rect.bottom() / 2 + 30);
	//bottonRect.setTopLeft(QPoint(rect.right() - 15, rect.bottom() / 2 - 30));
	//bottonRect.setBottomRight(QPoint(rect.right(), rect.bottom() / 2 + 30));
	//QColor color;
	//color.setRed(200);
	//QPen blackPen = QPen(color, 2, Qt::SolidLine);//QPen(QColor("#2a2c2f"), 2, Qt::SolidLine);
	//painter.setPen(blackPen);
	//painter.drawRect(bottonRect);
}

void ImageThumbnailDockWidget::mouseReleaseEvent(QMouseEvent *event)
{
    //    QPoint pos = event->pos();

    //    QRect rect = this->rect();

    //    QDockWidget *dw = qobject_cast<QDockWidget*>(parentWidget());
    //    Q_ASSERT(dw);

    //    if (dw->features() & QDockWidget::DockWidgetVerticalTitleBar) {
    //        QPoint p = pos;
    //        pos.setX(rect.left() + rect.bottom() - p.y());
    //        pos.setY(rect.top() + p.x() - rect.left());

    //        QSize s = rect.size();
    //        s.transpose();
    //        rect.setSize(s);
    //    }

    //    const int buttonRight = 7;
    //    const int buttonWidth = 20;
    //    int right = rect.right() - pos.x();
    //    int button = (right - buttonRight)/buttonWidth;
    //    switch (button) {
    //        case 0:
    //            event->accept();
    //            dw->close();
    //            break;
    //        case 1:
    //            event->accept();
    //            dw->setFloating(!dw->isFloating());
    //            break;
    //        case 2: {
    //            event->accept();
    //            QDockWidget::DockWidgetFeatures features = dw->features();
    //            if (features & QDockWidget::DockWidgetVerticalTitleBar)
    //                features &= ~QDockWidget::DockWidgetVerticalTitleBar;
    //            else
    //                features |= QDockWidget::DockWidgetVerticalTitleBar;
    //            dw->setFeatures(features);
    //            break;
    //        }
    //        default:
    //            event->ignore();
    //            break;
    //    }
}

void ImageThumbnailDockWidget::updateMask()
{
    /*
 * QDockWidget *dw = qobject_cast<QDockWidget*>(parent());
    Q_ASSERT(dw);

    QRect rect = dw->rect();
    QPixmap bitmap(dw->size());

    {
        QPainter painter(&bitmap);

        // initialize to transparent
        painter.fillRect(rect, Qt::color0);

        QRect contents = rect;
        contents.setTopLeft(geometry().bottomLeft());
        contents.setRight(geometry().right());
        contents.setBottom(contents.bottom()-y());
        painter.fillRect(contents, Qt::color1);

        // let's paint the titlebar
        QRect titleRect = this->geometry();

        if (dw->features() & QDockWidget::DockWidgetVerticalTitleBar) {
            QSize s = rect.size();
            s.transpose();
            rect.setSize(s);

            QSize s2 = size();
            s2.transpose();
            titleRect.setSize(s2);

            painter.translate(rect.left(), rect.top() + rect.width());
            painter.rotate(-90);
            painter.translate(-rect.left(), -rect.top());
        }

        contents.setTopLeft(titleRect.bottomLeft());
        contents.setRight(titleRect.right());
        contents.setBottom(rect.bottom()-y());

        QRect rect = titleRect;

        painter.drawPixmap(rect.topLeft(), leftPm.mask());
        painter.fillRect(rect.left() + leftPm.width(), rect.top(),
            rect.width() - leftPm.width() - rightPm.width(),
            centerPm.height(), Qt::color1);
        painter.drawPixmap(rect.topRight() - QPoint(rightPm.width() - 1, 0), rightPm.mask());

        painter.fillRect(contents, Qt::color1);
    }

    dw->setMask(bitmap);
*/
}

void ImageThumbnailDockWidget::mainAppclearThumbnail()
{
    clearThumbmailList();
}

void ImageThumbnailDockWidget::addViewerExtension(QWidget *widget, QString caption, const QString &widgetIdentifier)
{
    m_lastExtension = widget;
    m_lastWidgetIdentifier = widgetIdentifier;
    //INFO_LOG("ImageThumbnailDockWidget switch:"+caption);
    //this->setCurrentIndex(this->indexOf(application));
    // We add the extension to the list of active extensions
    m_activeExtensions.insert(widget, widgetIdentifier);
}

}   // end namespace udg
