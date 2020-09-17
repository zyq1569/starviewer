#ifndef IMAGETHUMBNAILDOCKWIDGET_H
#define IMAGETHUMBNAILDOCKWIDGET_H

class QListWidget;
class QVBoxLayout;
class QHBoxLayout;
class QListWidgetItem;

#include <QDockWidget>

namespace udg {

class QApplicationMainWindow;
class Patient;

class ImageThumbnailDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    ImageThumbnailDockWidget(const QString &title, QApplicationMainWindow *mainApp,QWidget *parent = nullptr,
                             Qt::WindowFlags flags = Qt::WindowFlags());
    ~ImageThumbnailDockWidget();

public:
    //QSize sizeHint() const override
    //{
    //return minimumSizeHint();
    //}
    //QSize minimumSizeHint() const override;
    void clearThumbmailList();
    void addPatientsThumbmailList(QList<Patient*> patientsList);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

public slots:
    void updateMask();

signals:
    /// Signal that is emitted when the mouse enters the widget and notifies us of its identifier
    void isActive(QString);

    /// This signal is emitted when an item is chosen and notifies us of its identifier
    void selectedItem(QString);

private slots:
    /// Update views related to the active item (the one the mouse hovers over)
    void updateActiveItemView(QListWidgetItem *item);

private:
    QPalette m_background;
    const QPixmap m_pushpin;
    QListWidget *m_ImagelistWidge;
    QHBoxLayout *m_mainlayout;
    QList<Patient*> m_patientsList;
private:
    /// Pointer to the main application
    QApplicationMainWindow *m_mainApp;
};

};  // end namespace udg
#endif // IMAGETHUMBNAILDOCKWIDGET_H
