#ifndef IMAGETHUMBNAILDOCKWIDGET_H
#define IMAGETHUMBNAILDOCKWIDGET_H

class QListWidget;
class QVBoxLayout;
class QHBoxLayout;
class QListWidgetItem;

#include <QDockWidget>
#include <QMap>

namespace udg {

class QApplicationMainWindow;
class Patient;
class Volume;
class ImageThumbnailDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    ImageThumbnailDockWidget(const QString &title, QApplicationMainWindow *mainApp,QWidget *parent = nullptr,
                             Qt::WindowFlags flags = Qt::WindowFlags());
    ~ImageThumbnailDockWidget();

public:
    //QSize minimumSizeHint() const override;
    void clearThumbmailList();
    void addPatientsThumbmailList(QList<Patient*> patientsList);
    void addViewerExtension(QWidget *widget, QString caption, const QString &widgetIdentifier);


protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

public slots:
    void updateMask();
    void mainAppclearThumbnail();


signals:
    /// Signal that is emitted when the mouse enters the widget and notifies us of its identifier
    void isActive(QString);

    /// This signal is emitted when an item is chosen and notifies us of its identifier
    void selectedItem(QString);

    /// Signal that notifies the volume corresponding to the chosen item
    void selectedVolume(Volume *);

private slots:
    /// Update views related to the active item (the one the mouse hovers over)
    void updateActiveItemView(QListWidgetItem *item);

    void refreshTab(int index);

private:
    QPalette m_background;
    const QPixmap m_pushpin;
    QListWidget *m_ImagelistWidge;
    QHBoxLayout *m_mainlayout;
    QList<Patient*> m_patientsList;
private:
    /// Pointer to the main application
    QApplicationMainWindow *m_mainApp;
    QWidget *m_lastExtension;
    ///Map containing the extensions we have open, associated with their name
    QMap<QWidget *, QString> m_activeExtensions;
    QString m_lastWidgetIdentifier;
};

};  // end namespace udg
#endif // IMAGETHUMBNAILDOCKWIDGET_H
