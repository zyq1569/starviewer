#ifndef IMAGETHUMBNAILDOCKWIDGET_H
#define IMAGETHUMBNAILDOCKWIDGET_H

class QListWidget;
class QVBoxLayout;
class QHBoxLayout;
#include <QDockWidget>

class ImageThumbnailDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    ImageThumbnailDockWidget(const QString &title, QWidget *parent = nullptr,
                             Qt::WindowFlags flags = Qt::WindowFlags());
    ~ImageThumbnailDockWidget();

public:
    //    QSize sizeHint() const override
    //    {
    //        return minimumSizeHint();
    //    }
    //    QSize minimumSizeHint() const override;


protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

public slots:
    void updateMask();

private:
    QPalette m_background;
    const QPixmap m_pushpin;
    QListWidget *m_ImagelistWidge;
    QHBoxLayout *m_mainlayout;
};

#endif // IMAGETHUMBNAILDOCKWIDGET_H
