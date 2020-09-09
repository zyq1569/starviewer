#ifndef IMAGETHUMBNAILDOCKWIDGET_H
#define IMAGETHUMBNAILDOCKWIDGET_H

#include <QDockWidget>

class ImageThumbnailDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    ImageThumbnailDockWidget(const QString &title, QWidget *parent = nullptr,
                             Qt::WindowFlags flags = Qt::WindowFlags());
    ~ImageThumbnailDockWidget();
private:
    QPalette m_background;
};

#endif // IMAGETHUMBNAILDOCKWIDGET_H
