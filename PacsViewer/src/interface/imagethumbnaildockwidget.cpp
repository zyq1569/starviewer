#include "imagethumbnaildockwidget.h"

#include <QSize>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDir>
#include <QVBoxLayout>
#include <QPalette>
const int g_DockWidgetwith = 133;
const QSize IMAGE_SIZE(96,99);
const QSize ITEM_SIZE(109,120);

ImageThumbnailDockWidget::ImageThumbnailDockWidget(const QString &title, QWidget *parent,
                                                   Qt::WindowFlags flags ):QDockWidget(title, parent, flags)
{
//    m_background.setColor(QPalette::Background,Qt::black);
//    setAutoFillBackground(true);
    setPalette(m_background);
    m_ImagelistWidge = new QListWidget(this);
    m_ImagelistWidge->setIconSize(IMAGE_SIZE);
    m_ImagelistWidge->setResizeMode(QListView::Adjust);
    m_ImagelistWidge->setViewMode(QListView::IconMode);
    m_ImagelistWidge->setMovement(QListView::Static);
    m_ImagelistWidge->setSelectionRectVisible(true);
    m_ImagelistWidge->setSpacing(0);
    m_ImagelistWidge->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);//always show!!?? set

//    QPalette palette;
//    palette.setColor(QPalette::Background, Qt::red);
//    m_ImagelistWidge->setPalette(palette);

    m_mainlayout = new QHBoxLayout;
    m_mainlayout->addWidget(m_ImagelistWidge);
    m_mainlayout->setMargin(0);
    setLayout(m_mainlayout);
    m_ImagelistWidge->show();
    setMinimumWidth(g_DockWidgetwith);
    setMaximumWidth(g_DockWidgetwith);

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
}
void ImageThumbnailDockWidget::addPatientsThumbmailList()
{

}

ImageThumbnailDockWidget::~ImageThumbnailDockWidget()
{

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
    //    QPainter painter(this);
    //    QRect rect = this->rect();

    //    QDockWidget *dw = qobject_cast<QDockWidget*>(parentWidget());
    //    Q_ASSERT(dw);

    //    if (dw->features() & QDockWidget::DockWidgetVerticalTitleBar) {
    //        QSize s = rect.size();
    //        s.transpose();
    //        rect.setSize(s);

    //        painter.translate(rect.left(), rect.top() + rect.width());
    //        painter.rotate(-90);
    //        painter.translate(-rect.left(), -rect.top());
    //    }

    //    painter.drawPixmap(rect.topLeft(), leftPm);
    //    painter.drawPixmap(rect.topRight() - QPoint(rightPm.width() - 1, 0), rightPm);
    //    QBrush brush(centerPm);
    //    painter.fillRect(rect.left() + leftPm.width(), rect.top(),
    //                        rect.width() - leftPm.width() - rightPm.width(),
    //                       centerPm.height(), centerPm);
    QRect rect = this->rect();
    m_ImagelistWidge->setGeometry(rect.left(),rect.top(),rect.right(),rect.bottom());
#ifdef QT_DEBUG
    /// look ! PatientBrowserMenuExtendedItem
    ///void PatientBrowserMenu::placeAdditionalInfoWidget()
    static bool init = false;
    if (!init)
    {
        init = true;
        QString path = "F:\\\log\\image";
        QDir dir(path);
        if (dir.exists())
        {
            dir.setFilter(QDir::Files | QDir::NoSymLinks);
            QStringList filters;
            filters<<"*.png"<<"*.jpg";
            dir.setNameFilters(filters);
            static QStringList m_imgList;
            m_imgList = dir.entryList();
            if (m_imgList.count()>0)
            {
                for (int i=0; i<m_imgList.count(); i++)
                {
                    QPixmap pixmap(path +"\\"+m_imgList.at(i));
                    QListWidgetItem *item = new QListWidgetItem(QIcon(pixmap.scaled(IMAGE_SIZE)),m_imgList.at(i));
                    item->setSizeHint(ITEM_SIZE);
                    m_ImagelistWidge->insertItem(i,item);
                }
            }
        }
        //connect(m_ImagelistWidge, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slot_itemClicked(QListWidgetItem*)));
    }
#endif
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
