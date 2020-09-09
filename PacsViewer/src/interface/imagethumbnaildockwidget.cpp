#include "imagethumbnaildockwidget.h"

ImageThumbnailDockWidget::ImageThumbnailDockWidget(const QString &title, QWidget *parent,
                          Qt::WindowFlags flags ):QDockWidget(title, parent, flags)
{

    m_background.setColor(QPalette::Background,Qt::black);
    setAutoFillBackground(true);
    setPalette(m_background);
}
ImageThumbnailDockWidget::~ImageThumbnailDockWidget()
{

}
