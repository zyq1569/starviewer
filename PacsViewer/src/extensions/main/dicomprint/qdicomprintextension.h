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


#ifndef UDGQDICOMPRINTEXTENSION_H
#define UDGQDICOMPRINTEXTENSION_H

#include "ui_qdicomprintextensionbase.h"
#include "dicomprint.h"

#include <QTimer>
#include <QList>
#include <QPair>

namespace udg {

class QDicomPrinterConfigurationWidget;
class Study;
class Image;
class DicomPrintPage;
class DicomPrintJob;
class Volume;
class ToolManager;
class DICOMPrintPresentationStateImage;

class QDicomPrintExtension : public QWidget, private::Ui::QDicomPrintExtensionBase {
Q_OBJECT

public:
    QDicomPrintExtension(QWidget *parent = 0);

public slots:
    /// Li assigna el volum que s'ha d'imprimir
    void setInput(Volume *input);
    /// Actualitzem tots els elements que estan lligats al Volume que estem visualitzant.
    void updateInput();

private slots:
    // Actualitza la informació que mostrem de la impressora
    void selectedDicomPrinterChanged(int indexOfSelectedDicomPrinter);
    /// Omple el combobox amb les impressores i mostra com a seleccionada la que esta marca com a impressora per defecte
    void fillSelectedDicomPrinterComboBox();
    /// Activa/desactiva el fram que conté poder escollir el interval i des de quina imatge fins quina imprimir en funció del mode de selecció
    /// d'imatges escollit
    void imageSelectionModeChanged();
    /// Slot que s'executa quan canviem de valor a través del Slider l'interval d'imatges a imprimir
    void m_intervalImagesSliderValueChanged(int value);
    /// Slot que s'executa quan canviem de valor a través del Slider a partir de quina imatge imprimir
    void m_fromImageSliderValueChanged();
    /// Slot que s'executa quan canviem de valor a través del Slider fins quina imatge imprimir
    void m_toImageSliderValueChanged();
    /// Slot que s'exectua quan el LineEdit per especificar el valor del interval d'imatges a imprimir és editat, per traspassar el seu valor al Slider
    void m_intervalImagesLineEditTextEdited(const QString &text);
    /// Slot que s'exectua quan el LineEdit per especificar a partir de quina imatges s'haa imprimir és editat, per traspassar el seu valor al Slider
    void m_fromImageLineEditTextEdited(const QString &text);
    /// Slot que s'exectua quan el LineEdit per especificar fins quina imatge s'ha d'imprimir és editat, per traspassar el seu valor al Slider
    void m_toImageLineEditTextEdited(const QString &text);
    /// Actualitza label indicant el número de pàgines dicom print que s'imprimiran
    void updateNumberOfDicomPrintPagesToPrint();

    /// Envia a imprimir les imatges seleccionades a la impressora seleccionada
    void print();

    /// Amaga el frame que indica que s'han enviat a imprimir correctament les imatges
    void timeoutTimer();

    /// Afegeix les imatges selccionades com a grup d'imatges per imprimir i les mostra al ThumbnailsPreview
    void addSelectedImagesToGroupedDICOMImagesToPrint();

    /// Esborra els grups d'imatges seleccionades per imprimir
    void clearDICOMImagesGroupedToPrint();

    /// Esborra el grups d'imatges per imprimir seleccionades en el control ThumbnailsPreview
    void removeGroupedDICOMImagesToPrintSelectedInThumbnailsPreview();

private:
    /// Crea les connexions
    void createConnections();

    /// Crea inputValidators pels lineEdit de la selecció d'imatges
    void configureInputValidator();

    /// Inicialitza les tools que volem tenir activades al viewer
    void initializeViewerTools();

    /// Reinicia i configura els controls de selecció d'imatges en funció dels nombre d'imatges
    void resetAndUpdateSelectionImagesValue();

    /// Retorna el DicomPrintJob que s'ha d'enviar a imprimir en funció de la impressora i imatges seleccionades
    DicomPrintJob getDicomPrintJobToPrint();

    /// Retorna una llista de pàgines per imprimir
    QList<DicomPrintPage> getDicomPrintPageListToPrint();

    /// Retorna les imatges s'han d'enviar a imprimir en funció de lo definit a la selecció d'imatges
    QList<Image*> getSelectedImagesToAddToPrint() const;

    /// Retorna la impressora seleccionada
    DicomPrinter getSelectedDicomPrinter() const;

    /// Indica amb la selecció actua el número de pàgines a imprimir, sinó l'ha pogut calcu·lar correctament retorna 0
    int getNumberOfPagesToPrint() const;

    /// Ens retorna un DicomPrintPage amb els paràmetres d'impressió omplerts a partir d'una impressora. No afegeix les imatges ni número de pàgina
    DicomPrintPage fillDicomPrintPagePrintSettings(const DicomPrinter &dicomPrinter) const;

    /// Mostra per pantalla els errors que s'han produït alhora d'imprimir
    /// Degut a només podem tenir una pàgina per FilmSession degut a limitacions de dcmtk fa que haguem d'imprimir pàgina per pàgina
    /// per això ens podem trobar que la primera pàgina s'imprimeixi bé, i les restants no, per això passem el paràmetre printedSomePage per indica que només
    /// algunes de les pàgines han fallat
    void showDicomPrintError(DicomPrint::DicomPrintError error, bool printedSomePage);

    /// Comprova si se suporta el format de la sèrie i actualitza la interfície segons convingui.
    void updateVolumeSupport();

    /// Activa/desactiva els controls de configuració d'impressió i el botó d'impressió
    void setEnabledPrintControls(bool enabled);

    ///Activa/desactiva els controls per afegir imatges a imprimir
    void setEnabledAddImagesToPrintControls(bool enabled);

    /// Ens afegeix anotacions a la pàgina a imprimir amb informació de l'estudi, com nom de pacient,
    /// Data i hora estudi, Institució, ID Estudi i descripció,...
    void addAnnotationsToDicomPrintPage(DicomPrintPage *dicomPrintPage, Image *imageToPrint);

    /// Retorna la descripcio pel thumbnail que mostra un rang d'imatges seleccionades per imprimir
    QString getThumbnailPreviewDescriptionOfSelectedGroupedDICOMImagesToPrint() const;

    /// Treu del grups d'imatges per imprimir el grup d'imatges amb l'ID passat per parametre
    void removeGroupedDICOMImagesToPrint(int IDGroup);

    /// Retorna les imatges a imprimir
    QList<QPair<Image*, DICOMPrintPresentationStateImage> > getImagesToPrint() const;

    /// Retorna el PresentationState a aplicar a les imatges seleccionades per imprimir
    DICOMPrintPresentationStateImage getDICOMPrintPresentationStateImageForCurrentSelectedImages() const;

private:
    /// Contenidor d'objectes associats a l'estudi que serveix per facilitar la intercomunicació
    struct GroupedDICOMImagesToPrint
    {
        QList<Image*> imagesToPrint;
        DICOMPrintPresentationStateImage dicomPrintPresentationStateImage;
        int ID;
    };

    QList<GroupedDICOMImagesToPrint> m_groupedDICOMImagesToPrintList;
    int m_lastIDGroupedDICOMImagesToPrint;

    /// Gestor de tools pel viewer
    ToolManager *m_toolManager;

    QTimer *m_qTimer;

    QDicomPrinterConfigurationWidget *m_qDicomPrinterConfigurationWidgetProof;
};

} // end namespace udg.

#endif
