/*************************************************************************************
  Copyright (C) 2014 Laboratori de Gràfics i Imatge, Universitat de Girona &
  Institut de Diagnòstic per la Imatge.
  Girona 2014. All rights reserved.
  http://starviewer.udg.edu

  This file is part of the Starviewer (Medical Imaging Software) open source project.
  It is subject to the license terms in the LICENSE file found in the top-level
  directory of this distribution and at http://starviewer.udg.edu/license. No part of
  the Starviewer (Medical Imaging Software) open source project, including this file,
  may be copied, modified, propagated, or distributed except according to the
  terms contained in the LICENSE file.
 *************************************************************************************/

#ifndef UDGDICOMPRINTPAGE_H
#define UDGDICOMPRINTPAGE_H

#include <QString>
#include <QMap>
#include <QPair>

#include "dicomprintpresentationstateimage.h"

namespace udg {

class Image;

/**
    Implementació per la Interfície DicomPrintPage que defineix una pàgina amb imatges per imprimir en impressores Dicom en DCMTK.
	Support for the DicomPrintPage interface that defines a page with images to print on Dicom printers in DCMTK.
  */
class DicomPrintPage {
public:
    /// Especifica/Retorna la mida sobre de la placa sobre el que s'imprimirà
    void setFilmSize(const QString &filmSize);
    QString getFilmSize() const;

    /// Especifica/Retorna layout de la placa sobre el que s'imprimirà
    void setFilmLayout(const QString &filmLayout);
    QString getFilmLayout() const;

    /// Retorna el número de columnes/files del layout
    int getFilmLayoutColumns();
    int getFilmLayoutRows();

    /// Especifica/Retorna l'orientació (Horitzontal/Vertical) de la placa en que s'imprimirà
    void setFilmOrientation(const QString &filmOrientation);
    QString getFilmOrientation() const;

    /// Especifica/retorna el Magnification amb le que s'imprimirà
    void setMagnificationType(const QString &magnificationType);
    QString getMagnificationType() const;

    /// Especifica/retorna la densitat màxima amb la que s'imprimirà
    void setMaxDensity(ushort maxDensity);
    ushort getMaxDensity();

    /// Especifica/retorna la densitat mínima amb la que s'imprimirà
    void setMinDensity(ushort minDensity);
    ushort getMinDensity();

    /// Especifica si s'imprimirà les imatges amb trim o sense
    void setTrim(bool trim);
    bool getTrim();

    /// Especifica/retorna la densitat amb la que s'imprimirà la línia de separació entre les imatges
    void setBorderDensity(const QString &borderDensity);
    QString getBorderDensity() const;

    /// Especifica/retorna la densitat amb que s'imprimiran els image box sense imatge
    void setEmptyImageDensity(const QString &emptyImageDensity);
    QString getEmptyImageDensity() const;

    /// Especifica/Retorna el smoothing type per defecte amb la que s'imprimirà
    void setSmoothingType(const QString &smoothingType);
    QString getSmoothingType() const;

    /// Especifica/retorna la polaritat per defecte que s'utilitzarà per imprimir
    // Aquest paràmetre a nivell dicom s'aplica per imatge no per pàgina
    void setPolarity(const QString &polarity);
    QString getPolarity() const;

    /// Especifica retorna la informació del tag Configuration Information
    void setConfigurationInformation(const QString &configurationInformation);
    QString getConfigurationInformation() const;

    /// Ens afegeix una anotació a la pàgina en la posició indicada. El text de l'anotació no pot fer més de 64 caràcters, si fa més de 64 caràcters el trunca
    /// Si s'afegeix una anotació en una posició que ja hi havia una altra notació aquesta se sobre escriu, i si el text és buit no afegeix l'anotació
    /// La majoria d'impressores només tenen 6 posicions per les anotacions Agfa, Kodak, Sony, Codonics. Tenir en compte que totes les impressores les posicions
    /// de les anotacions són de la 1 a la 6 i Sony és de la 0 a la 5.
	//It adds an annotation to the page in the indicated position.The text of the annotation cannot be longer than 64 characters; 
	//if it is more than 64 characters, it is truncated.If an annotation is added in a position that already had another notation, 
	//it is overwritten, and if the text is empty, it does not add the annotation.Most printers only have 6 positions for Agfa, 
	//Kodak, Sony, Codonics annotations.Note that all printers' positions of the annotations are from 1 to 6 and Sony is from 0 to 5.
	//它会向页面的指定位置添加注释。注释的文本不能超过 64 个字符，如果超过 64 个字符，则将被截断
	//如果在已存在其他表示法的位置添加注释，则会覆盖该注释，如果文本为空，则不会添加注释
	//大多数打印机只有 6 个位置用于 Agfa、Kodak、Sony、Codonics 注释。请注意，所有打印机位置
	//的注释从 1 到 6，而 Sony 是从 0 到 5。
    void addAnnotation(int position, const QString &annotationText);

    /// Retorna les anotacions que ens han indicat que s'han d'imprimir amb la pàgina
    QMap<int, QString> getPageAnnotations();

    /// Especifica/retorna el número de pàgina
    void setPageNumber(int pageNumber);
    int getPageNumber();

    /// Especifica/retorna les imatges que s'han d'imprimir en aquesta pàgina. Les imatges s'imprimiren segons l'ordre d'inserció a la llista
    void setImagesToPrint(QList<QPair<Image*, DICOMPrintPresentationStateImage> > imagesToPrint);
    QList<QPair<Image*, DICOMPrintPresentationStateImage> > getImagesToPrint();

private:
    QString m_filmSize;
    QString m_filmLayout;
    QString m_filmOrientation;
    QString m_magnificationType;
    bool m_trim;
    QString m_borderDensity;
    QString m_emptyImageDensity;
    QString m_smoothingType;
    QString m_polarity;
    ushort m_minDensity, m_maxDensity;
    QString m_configurationInformation;
    QMap<int, QString> m_pageAnnotations;
    QString m_annotationDisplayFormatID;
    int m_pageNumber;
    QList<QPair<Image*, DICOMPrintPresentationStateImage> > m_imagesToPrint;
};
};
#endif
