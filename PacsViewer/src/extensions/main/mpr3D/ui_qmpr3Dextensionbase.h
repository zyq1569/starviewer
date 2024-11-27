/********************************************************************************
** Form generated from reading UI file 'qmpr3Dextensionbase.ui'
**
** Created by: Qt User Interface Compiler version 5.12.12
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QMPR3DEXTENSIONBASE_H
#define UI_QMPR3DEXTENSIONBASE_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "QVTKOpenGLWidget.h"
#include "qvoilutcombobox.h"

QT_BEGIN_NAMESPACE

class Ui_QMPR3DExtensionBase
{
public:
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout;
    QToolButton *m_slicingToolButton;
    QToolButton *m_zoomToolButton;
    QToolButton *m_ROIToolButton;
    QToolButton *m_distanceToolButton;
    QToolButton *m_angleToolButton;
    QToolButton *m_eraserToolButton;
    QToolButton *m_screenShotToolButton;
    QToolButton *m_screenshotsExporterToolButton;
    QToolButton *m_viewerInformationToolButton;
    QToolButton *m_voxelInformationToolButton;
    QLabel *m_thickSlabLabel;
    QSlider *m_thickSlabSlider;
    QDoubleSpinBox *m_thickSlabSpinBox;
    QToolButton *m_mipToolButton;
    QToolButton *m_horizontalLayoutToolButton;
    QPushButton *m_Reset;
    QLabel *m_phasesAlertLabel;
    QSpacerItem *spacerItem;
    QHBoxLayout *hboxLayout1;
    QLabel *label;
    udg::QVoiLutComboBox *m_voiLutComboBox;
    QSplitter *m_horizontalSplitter;
    QSplitter *m_verticalSplitter;
    QWidget *layout4;
    QVBoxLayout *vboxLayout1;
    QHBoxLayout *hboxLayout2;
    QVTKOpenGLWidget *m_axial2DView;
    QWidget *layout9;
    QVBoxLayout *vboxLayout2;
    QVTKOpenGLWidget *m_sagital2DView;
    QVTKOpenGLWidget *m_coronal2DView;

    void setupUi(QWidget *QMPR3DExtensionBase)
    {
        if (QMPR3DExtensionBase->objectName().isEmpty())
            QMPR3DExtensionBase->setObjectName(QString::fromUtf8("QMPR3DExtensionBase"));
        QMPR3DExtensionBase->resize(1156, 541);
        vboxLayout = new QVBoxLayout(QMPR3DExtensionBase);
        vboxLayout->setSpacing(1);
        vboxLayout->setContentsMargins(11, 11, 11, 11);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        vboxLayout->setContentsMargins(0, 0, 0, 0);
        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        m_slicingToolButton = new QToolButton(QMPR3DExtensionBase);
        m_slicingToolButton->setObjectName(QString::fromUtf8("m_slicingToolButton"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/icons/layer-previous.svg"), QSize(), QIcon::Normal, QIcon::Off);
        m_slicingToolButton->setIcon(icon);
        m_slicingToolButton->setIconSize(QSize(32, 32));
        m_slicingToolButton->setCheckable(true);
        m_slicingToolButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        m_slicingToolButton->setAutoRaise(true);

        hboxLayout->addWidget(m_slicingToolButton);

        m_zoomToolButton = new QToolButton(QMPR3DExtensionBase);
        m_zoomToolButton->setObjectName(QString::fromUtf8("m_zoomToolButton"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/images/icons/edit-find.svg"), QSize(), QIcon::Normal, QIcon::Off);
        m_zoomToolButton->setIcon(icon1);
        m_zoomToolButton->setIconSize(QSize(32, 32));
        m_zoomToolButton->setCheckable(true);
        m_zoomToolButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        m_zoomToolButton->setAutoRaise(true);

        hboxLayout->addWidget(m_zoomToolButton);

        m_ROIToolButton = new QToolButton(QMPR3DExtensionBase);
        m_ROIToolButton->setObjectName(QString::fromUtf8("m_ROIToolButton"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/images/icons/draw-polyline.svg"), QSize(), QIcon::Normal, QIcon::Off);
        m_ROIToolButton->setIcon(icon2);
        m_ROIToolButton->setIconSize(QSize(32, 32));
        m_ROIToolButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        m_ROIToolButton->setAutoRaise(true);

        hboxLayout->addWidget(m_ROIToolButton);

        m_distanceToolButton = new QToolButton(QMPR3DExtensionBase);
        m_distanceToolButton->setObjectName(QString::fromUtf8("m_distanceToolButton"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/images/icons/tool-measure.svg"), QSize(), QIcon::Normal, QIcon::Off);
        m_distanceToolButton->setIcon(icon3);
        m_distanceToolButton->setIconSize(QSize(32, 32));
        m_distanceToolButton->setCheckable(true);
        m_distanceToolButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        m_distanceToolButton->setAutoRaise(true);

        hboxLayout->addWidget(m_distanceToolButton);

        m_angleToolButton = new QToolButton(QMPR3DExtensionBase);
        m_angleToolButton->setObjectName(QString::fromUtf8("m_angleToolButton"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/images/icons/measure-angle.svg"), QSize(), QIcon::Normal, QIcon::Off);
        m_angleToolButton->setIcon(icon4);
        m_angleToolButton->setIconSize(QSize(32, 32));
        m_angleToolButton->setCheckable(true);
        m_angleToolButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        m_angleToolButton->setAutoRaise(true);

        hboxLayout->addWidget(m_angleToolButton);

        m_eraserToolButton = new QToolButton(QMPR3DExtensionBase);
        m_eraserToolButton->setObjectName(QString::fromUtf8("m_eraserToolButton"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/images/icons/draw-eraser.svg"), QSize(), QIcon::Normal, QIcon::Off);
        m_eraserToolButton->setIcon(icon5);
        m_eraserToolButton->setIconSize(QSize(32, 32));
        m_eraserToolButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        m_eraserToolButton->setAutoRaise(true);

        hboxLayout->addWidget(m_eraserToolButton);

        m_screenShotToolButton = new QToolButton(QMPR3DExtensionBase);
        m_screenShotToolButton->setObjectName(QString::fromUtf8("m_screenShotToolButton"));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/images/icons/camera-photo.svg"), QSize(), QIcon::Normal, QIcon::Off);
        m_screenShotToolButton->setIcon(icon6);
        m_screenShotToolButton->setIconSize(QSize(32, 32));
        m_screenShotToolButton->setAutoRaise(true);

        hboxLayout->addWidget(m_screenShotToolButton);

        m_screenshotsExporterToolButton = new QToolButton(QMPR3DExtensionBase);
        m_screenshotsExporterToolButton->setObjectName(QString::fromUtf8("m_screenshotsExporterToolButton"));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/images/icons/upload-media.svg"), QSize(), QIcon::Normal, QIcon::Off);
        m_screenshotsExporterToolButton->setIcon(icon7);
        m_screenshotsExporterToolButton->setIconSize(QSize(32, 32));
        m_screenshotsExporterToolButton->setAutoRaise(true);

        hboxLayout->addWidget(m_screenshotsExporterToolButton);

        m_viewerInformationToolButton = new QToolButton(QMPR3DExtensionBase);
        m_viewerInformationToolButton->setObjectName(QString::fromUtf8("m_viewerInformationToolButton"));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/images/icons/annotations.svg"), QSize(), QIcon::Normal, QIcon::Off);
        m_viewerInformationToolButton->setIcon(icon8);
        m_viewerInformationToolButton->setIconSize(QSize(32, 32));
        m_viewerInformationToolButton->setCheckable(true);
        m_viewerInformationToolButton->setChecked(true);
        m_viewerInformationToolButton->setAutoRaise(true);

        hboxLayout->addWidget(m_viewerInformationToolButton);

        m_voxelInformationToolButton = new QToolButton(QMPR3DExtensionBase);
        m_voxelInformationToolButton->setObjectName(QString::fromUtf8("m_voxelInformationToolButton"));
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/images/icons/edit-select.svg"), QSize(), QIcon::Normal, QIcon::Off);
        m_voxelInformationToolButton->setIcon(icon9);
        m_voxelInformationToolButton->setIconSize(QSize(32, 32));
        m_voxelInformationToolButton->setCheckable(true);
        m_voxelInformationToolButton->setAutoRaise(true);

        hboxLayout->addWidget(m_voxelInformationToolButton);

        m_thickSlabLabel = new QLabel(QMPR3DExtensionBase);
        m_thickSlabLabel->setObjectName(QString::fromUtf8("m_thickSlabLabel"));

        hboxLayout->addWidget(m_thickSlabLabel);

        m_thickSlabSlider = new QSlider(QMPR3DExtensionBase);
        m_thickSlabSlider->setObjectName(QString::fromUtf8("m_thickSlabSlider"));
        m_thickSlabSlider->setOrientation(Qt::Horizontal);
        m_thickSlabSlider->setTickPosition(QSlider::NoTicks);
        m_thickSlabSlider->setTickInterval(0);

        hboxLayout->addWidget(m_thickSlabSlider);

        m_thickSlabSpinBox = new QDoubleSpinBox(QMPR3DExtensionBase);
        m_thickSlabSpinBox->setObjectName(QString::fromUtf8("m_thickSlabSpinBox"));

        hboxLayout->addWidget(m_thickSlabSpinBox);

        m_mipToolButton = new QToolButton(QMPR3DExtensionBase);
        m_mipToolButton->setObjectName(QString::fromUtf8("m_mipToolButton"));
        m_mipToolButton->setIconSize(QSize(32, 32));
        m_mipToolButton->setCheckable(true);
        m_mipToolButton->setAutoRaise(true);

        hboxLayout->addWidget(m_mipToolButton);

        m_horizontalLayoutToolButton = new QToolButton(QMPR3DExtensionBase);
        m_horizontalLayoutToolButton->setObjectName(QString::fromUtf8("m_horizontalLayoutToolButton"));
        m_horizontalLayoutToolButton->setIconSize(QSize(32, 32));
        m_horizontalLayoutToolButton->setCheckable(false);
        m_horizontalLayoutToolButton->setAutoRaise(true);

        hboxLayout->addWidget(m_horizontalLayoutToolButton);

        m_Reset = new QPushButton(QMPR3DExtensionBase);
        m_Reset->setObjectName(QString::fromUtf8("m_Reset"));

        hboxLayout->addWidget(m_Reset);

        m_phasesAlertLabel = new QLabel(QMPR3DExtensionBase);
        m_phasesAlertLabel->setObjectName(QString::fromUtf8("m_phasesAlertLabel"));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        m_phasesAlertLabel->setFont(font);
        m_phasesAlertLabel->setFrameShape(QFrame::NoFrame);

        hboxLayout->addWidget(m_phasesAlertLabel);

        spacerItem = new QSpacerItem(91, 50, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        hboxLayout1 = new QHBoxLayout();
        hboxLayout1->setSpacing(6);
        hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
        hboxLayout1->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(QMPR3DExtensionBase);
        label->setObjectName(QString::fromUtf8("label"));

        hboxLayout1->addWidget(label);

        m_voiLutComboBox = new udg::QVoiLutComboBox(QMPR3DExtensionBase);
        m_voiLutComboBox->setObjectName(QString::fromUtf8("m_voiLutComboBox"));

        hboxLayout1->addWidget(m_voiLutComboBox);


        hboxLayout->addLayout(hboxLayout1);


        vboxLayout->addLayout(hboxLayout);

        m_horizontalSplitter = new QSplitter(QMPR3DExtensionBase);
        m_horizontalSplitter->setObjectName(QString::fromUtf8("m_horizontalSplitter"));
        m_horizontalSplitter->setOrientation(Qt::Horizontal);
        m_verticalSplitter = new QSplitter(m_horizontalSplitter);
        m_verticalSplitter->setObjectName(QString::fromUtf8("m_verticalSplitter"));
        m_verticalSplitter->setOrientation(Qt::Vertical);
        layout4 = new QWidget(m_verticalSplitter);
        layout4->setObjectName(QString::fromUtf8("layout4"));
        vboxLayout1 = new QVBoxLayout(layout4);
        vboxLayout1->setSpacing(6);
        vboxLayout1->setContentsMargins(11, 11, 11, 11);
        vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
        vboxLayout1->setContentsMargins(0, 0, 0, 0);
        hboxLayout2 = new QHBoxLayout();
        hboxLayout2->setSpacing(6);
        hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
        hboxLayout2->setContentsMargins(0, 0, 0, 0);

        vboxLayout1->addLayout(hboxLayout2);

        m_axial2DView = new QVTKOpenGLWidget(layout4);
        m_axial2DView->setObjectName(QString::fromUtf8("m_axial2DView"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(m_axial2DView->sizePolicy().hasHeightForWidth());
        m_axial2DView->setSizePolicy(sizePolicy);

        vboxLayout1->addWidget(m_axial2DView);

        m_verticalSplitter->addWidget(layout4);
        layout9 = new QWidget(m_verticalSplitter);
        layout9->setObjectName(QString::fromUtf8("layout9"));
        vboxLayout2 = new QVBoxLayout(layout9);
        vboxLayout2->setSpacing(6);
        vboxLayout2->setContentsMargins(11, 11, 11, 11);
        vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
        vboxLayout2->setContentsMargins(0, 0, 0, 0);
        m_sagital2DView = new QVTKOpenGLWidget(layout9);
        m_sagital2DView->setObjectName(QString::fromUtf8("m_sagital2DView"));
        sizePolicy.setHeightForWidth(m_sagital2DView->sizePolicy().hasHeightForWidth());
        m_sagital2DView->setSizePolicy(sizePolicy);

        vboxLayout2->addWidget(m_sagital2DView);

        m_verticalSplitter->addWidget(layout9);
        m_horizontalSplitter->addWidget(m_verticalSplitter);
        m_coronal2DView = new QVTKOpenGLWidget(m_horizontalSplitter);
        m_coronal2DView->setObjectName(QString::fromUtf8("m_coronal2DView"));
        sizePolicy.setHeightForWidth(m_coronal2DView->sizePolicy().hasHeightForWidth());
        m_coronal2DView->setSizePolicy(sizePolicy);
        m_horizontalSplitter->addWidget(m_coronal2DView);

        vboxLayout->addWidget(m_horizontalSplitter);


        retranslateUi(QMPR3DExtensionBase);

        QMetaObject::connectSlotsByName(QMPR3DExtensionBase);
    } // setupUi

    void retranslateUi(QWidget *QMPR3DExtensionBase)
    {
        QMPR3DExtensionBase->setWindowTitle(QApplication::translate("QMPR3DExtensionBase", "MPR", nullptr));
        m_slicingToolButton->setText(QApplication::translate("QMPR3DExtensionBase", "Slicing", nullptr));
        m_zoomToolButton->setText(QApplication::translate("QMPR3DExtensionBase", "Angle", nullptr));
        m_ROIToolButton->setText(QApplication::translate("QMPR3DExtensionBase", "Angle", nullptr));
        m_distanceToolButton->setText(QApplication::translate("QMPR3DExtensionBase", "Angle", nullptr));
        m_angleToolButton->setText(QApplication::translate("QMPR3DExtensionBase", "Angle", nullptr));
        m_eraserToolButton->setText(QApplication::translate("QMPR3DExtensionBase", "Angle", nullptr));
        m_screenShotToolButton->setText(QApplication::translate("QMPR3DExtensionBase", "...", nullptr));
        m_screenshotsExporterToolButton->setText(QApplication::translate("QMPR3DExtensionBase", "...", nullptr));
        m_viewerInformationToolButton->setText(QApplication::translate("QMPR3DExtensionBase", "Zoom", nullptr));
        m_voxelInformationToolButton->setText(QApplication::translate("QMPR3DExtensionBase", "...", nullptr));
        m_thickSlabLabel->setText(QApplication::translate("QMPR3DExtensionBase", "Thick Slab (mm)", nullptr));
        m_mipToolButton->setText(QApplication::translate("QMPR3DExtensionBase", "...", nullptr));
        m_horizontalLayoutToolButton->setText(QApplication::translate("QMPR3DExtensionBase", "...", nullptr));
        m_Reset->setText(QApplication::translate("QMPR3DExtensionBase", "Reset", nullptr));
        m_phasesAlertLabel->setText(QApplication::translate("QMPR3DExtensionBase", "*The current Series has multiple phases. \n"
"Currently the MPR 2D doesn't support \n"
"Series with multiple phases so it won't work properly.", nullptr));
        label->setText(QApplication::translate("QMPR3DExtensionBase", "VOI LUT:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QMPR3DExtensionBase: public Ui_QMPR3DExtensionBase {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QMPR3DEXTENSIONBASE_H
