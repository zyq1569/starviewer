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

#include "qdicomprinterconfigurationwidget.h"
#include "dicomprinter.h"
#include "dicomprintermanager.h"
#include "starviewerapplication.h"
#include "qdicomaddprinterwidget.h"
#include "dicomprint.h"

#include <QMessageBox>

namespace udg {

// Public Methods
QDicomPrinterConfigurationWidget::QDicomPrinterConfigurationWidget()
{
    setupUi(this);

    m_listPrintersTreeWidget->setColumnHidden(0, true);

    m_addPrinterWidget = new QDicomAddPrinterWidget();

    createConnections();
    configureInputValidator();
    refreshPrinterList();
    clearPrinterSettings();

    // Afegim valor "" per si no volen especificar densitat
    m_maximumDensitySpinBox->setSpecialValueText(tr("Automatic"));
    // Afegim valor "" per si no volen especificar densitat
    m_minimumDensitySpinBox->setSpecialValueText(tr("Automatic"));

    printerSelectionChanged();
}

// Public Slots
void QDicomPrinterConfigurationWidget::printerSelectionChanged()
{
    if (m_listPrintersTreeWidget->selectedItems().count() > 0)
    {
        DicomPrinter selectedDicomPrinter = getSelectedDicomPrinter();

        this->clearPrinterSettings();

        this->setPrinterSettingsToControls(selectedDicomPrinter);
        m_qdicomPrinterBasicSettingsWidget->setDicomPrinterBasicSettings(selectedDicomPrinter);
        this->setAdvancedSettingsToControls(selectedDicomPrinter);

        m_applySettingsPushButton->setEnabled(true);
        m_printerSettingsGroupBox->setEnabled(true);
        m_qdicomPrinterBasicSettingsWidget->setEnabled(true);
        m_advancedSettingsGroupBox->setEnabled(true);
        // Perquè així comprovi si el AnnotationBox ha d'estar activat si aquest no ha canviat de valor respecte l'anterior impressora seleccionada
        // TODO: S'hauria d'intentar solventar qeu no fes falta invocar el mètode
        m_supportsAnnotationBoxYesRadioButtonToogled();
    }
    else
    {
        m_applySettingsPushButton->setEnabled(false);
        m_printerSettingsGroupBox->setEnabled(false);
        m_qdicomPrinterBasicSettingsWidget->setEnabled(false);
        m_advancedSettingsGroupBox->setEnabled(false);
    }
}

void QDicomPrinterConfigurationWidget::addPrinter()
{
    m_addPrinterWidget->clearInputs();
    m_addPrinterWidget->setVisible(true);
    // TODO:Després d'afegir una impressora s'hauria de fer signal printerSettingsChanged(), ara aprofitem el fet de que quan s'afegeix una impressora
    // com només han d'entrar AETitle, Hostname, port i descripció, llavors s'han d'acabar de complimentar les dades de la impressora per triar els
    // altres settings, fent un modificar, al fer el modificar llavors es fa l'emit del signal printerSettingsChanged()
}

bool QDicomPrinterConfigurationWidget::modifyPrinter()
{
    DicomPrinter dicomPrinter;
    DicomPrinterManager dicomPrinterManager;

    if (!m_listPrintersTreeWidget->selectedItems().isEmpty())
    {
        if (validatePrinterSettings())
        {
            this->getPrinterSettingsFromControls(dicomPrinter);
            m_qdicomPrinterBasicSettingsWidget->getDicomPrinterBasicSettings(dicomPrinter);
            this->getAdvancedSettingsFromControls(dicomPrinter);

            if (!dicomPrinterManager.updatePrinter(m_selectedPrinterId, dicomPrinter))
            {
                // Si rebem un error és que no ha trobat la impressora amb el ID passat per paràmetre
                QMessageBox::critical(this, ApplicationNameString, tr("An error has occurred. Unable to apply changes to the printer."));
            }
            else
            {
                refreshPrinterList();
                clearPrinterSettings();

                emit printerSettingsChanged();
                return true;
            }
        }
    }
    else
    {
        QMessageBox::information(this, ApplicationNameString, tr("No printer is selected to apply the changes."));
    }

    return false;
}

void QDicomPrinterConfigurationWidget::deletePrinter()
{
    DicomPrinterManager dicomPrinterManager;
    dicomPrinterManager.deletePrinter(m_selectedPrinterId);
    this->refreshPrinterList();
    clearPrinterSettings();

    emit printerSettingsChanged();
}

void QDicomPrinterConfigurationWidget::testPrinter()
{
    if (!m_listPrintersTreeWidget->selectedItems().isEmpty())
    {
        DicomPrint dicomPrint;
        DicomPrinter selectedDicomPrinter = getSelectedDicomPrinter();
        bool testIsCorrect;

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        testIsCorrect = dicomPrint.echoPrinter(selectedDicomPrinter);
        QApplication::restoreOverrideCursor();

        if (testIsCorrect)
        {
            // El test s'ha fet correctament i la impressora ha respós
            QMessageBox::information(this, ApplicationNameString, tr("Test of printer %1 is correct.").arg(selectedDicomPrinter.getAETitle()));
        }
        else
        {
            // El test ha fallat per algun motiu
            QString messageError;

            switch (dicomPrint.getLastError())
            {
                case DicomPrint::NotRespondedAsExpected:
                    messageError = tr("Printer %1 does not respond correctly.\nBe sure the hostname and AE Title are correct.")
                                 .arg(selectedDicomPrinter.getAETitle());
                    break;
                case DicomPrint::CanNotConnectToDicomPrinter:
                default:
                    messageError = tr("Printer %1 does not respond.\nBe sure the hostname and AE Title are correct.").arg(selectedDicomPrinter.getAETitle());
                    break;
            }

            QMessageBox::information(this, ApplicationNameString, messageError);
        }
    }
    else
    {
        QMessageBox::information(this, ApplicationNameString, tr("No printer is selected to test."));
    }

}

void QDicomPrinterConfigurationWidget::showNewPrinterAdded(int printerID)
{
    this->refreshPrinterList();
    this->selectPrinter(printerID);
}

void QDicomPrinterConfigurationWidget::m_magnitifacationTypeComboBoxIndexChanged(const QString &magnificationTypecomboBoxValue)
{
    // Smoothing Type només es pot escollir si el paràmetre de configuració Magnification Type té com a valor cubic
    if (magnificationTypecomboBoxValue.toUpper() == "CUBIC")
    {
        m_smoothingTypeComboBox->setEnabled(true);
    }
    else
    {
        m_smoothingTypeComboBox->setEnabled(false);
        // Treiem el valor que tenia a smoothingType
        m_smoothingTypeComboBox->setCurrentIndex(m_smoothingTypeComboBox->findText(""));
    }
}

void QDicomPrinterConfigurationWidget::m_supportsAnnotationBoxYesRadioButtonToogled()
{
    m_annotationDisplayFormatLineEdit->setEnabled(m_supportsAnnotationBoxYesRadioButton->isChecked());

    if (!m_supportsAnnotationBoxYesRadioButton->isChecked())
    {
        m_annotationDisplayFormatLineEdit->setText("");
    }
}

// Private Methods
void QDicomPrinterConfigurationWidget::createConnections()
{
    connect(m_addPrinterPushButton, SIGNAL(clicked()), SLOT(addPrinter()));
    connect(m_applySettingsPushButton, SIGNAL(clicked()), SLOT(modifyPrinter()));
    connect(m_acceptSettingsPushButton, SIGNAL(clicked()), SLOT(accept()));
    connect(m_cancelSettingsPushButton, SIGNAL(clicked()), SLOT(cancel()));
    connect(m_deletePrinterPushButton, SIGNAL(clicked()), SLOT(deletePrinter()));
    connect(m_testPrinterPushButton, SIGNAL(clicked()), SLOT(testPrinter()));

    connect(m_listPrintersTreeWidget, SIGNAL(itemSelectionChanged()), SLOT(printerSelectionChanged()));
    connect(m_addPrinterWidget, SIGNAL(newPrinterAddedSignal(int)), SLOT(showNewPrinterAdded(int)));
    connect(m_magnifactionTypeComboBox, SIGNAL(currentIndexChanged (const QString)), SLOT(m_magnitifacationTypeComboBoxIndexChanged(const QString)));
    connect(m_supportsAnnotationBoxYesRadioButton, SIGNAL(toggled(bool)), SLOT(m_supportsAnnotationBoxYesRadioButtonToogled()));
}

void QDicomPrinterConfigurationWidget::configureInputValidator()
{
    m_printerPortLineEdit->setValidator(new QIntValidator(0, 65535, m_printerPortLineEdit));
}

void QDicomPrinterConfigurationWidget::refreshPrinterList()
{
    DicomPrinterManager dicomPrinterManager;
    QList<DicomPrinter> dicomPrintersList = dicomPrinterManager.getDicomPrinterList();

    m_listPrintersTreeWidget->clear();

    foreach (DicomPrinter dicomPrinter, dicomPrintersList)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_listPrintersTreeWidget);

        item->setText(0, QString::number(m_listPrintersTreeWidget->topLevelItemCount() - 1));
        item->setText(1, dicomPrinter.getAETitle());
        item->setText(2, dicomPrinter.getHostname());
        item->setText(3, QString::number(dicomPrinter.getPort()));
        item->setText(4, dicomPrinter.getDescription());
    }
}

void QDicomPrinterConfigurationWidget::selectPrinter(int printerID)
{
    DicomPrinterManager dicomPrinterManager;
    QList<DicomPrinter> dicomPrintersList = dicomPrinterManager.getDicomPrinterList();
    QList<DicomPrinter>::iterator dicomPrintersListIterator = dicomPrintersList.begin();

    int i = 0;
    bool found = false;

    while (dicomPrintersListIterator != dicomPrintersList.end() && !found)
    {
        if (printerID == (*dicomPrintersListIterator).getID())
        {
            m_listPrintersTreeWidget->topLevelItem(i)->setSelected(true);
            found = true;
        }
        else
        {
            i++;
            dicomPrintersListIterator++;
        }

    }
}

bool QDicomPrinterConfigurationWidget::validatePrinterSettings()
{
    QString text;

    text = m_printerAetitleLineEdit->text();
    if (text.length() == 0)
    {
        QMessageBox::warning(this, ApplicationNameString, tr("AE Title field cannot be empty."));
        return false;
    }

    text = m_printerHostnameLineEdit->text();
    if (text.length() == 0)
    {
        QMessageBox::warning(this, ApplicationNameString, tr ("Hostname cannot be empty."));
        return false;
    }

    return true;
}

void QDicomPrinterConfigurationWidget::clearPrinterSettings()
{
    // Printer Settings
    m_printerAetitleLineEdit->setText("");
    m_printerDescriptionLineEdit->setText("");
    m_printerHostnameLineEdit->setText("");
    m_printerPortLineEdit->setText("");

    // Basic Settings
    m_qdicomPrinterBasicSettingsWidget->clear();

    // Advanced Settings
    m_magnifactionTypeComboBox->clear();
    m_smoothingTypeComboBox->clear();
    m_maximumDensitySpinBox->setValue(0);
    m_minimumDensitySpinBox->setValue(0);
    m_polarityComboBox->clear();
    m_borderDensityComboBox->clear();
    m_emptyDensityComboBox->clear();
    m_configurationInformationLineEdit->setText("");
    m_yesVisibleTrimRadioButton->setChecked(true);
    m_supportsAnnotationBoxNoRadioButton->setChecked(true);
    m_annotationDisplayFormatLineEdit->setText("");
    m_printerDefaultPrinterCheckBox->setChecked(false);
}

void QDicomPrinterConfigurationWidget::setPrinterSettingsToControls(DicomPrinter &printer)
{
    m_printerAetitleLineEdit->setText(printer.getAETitle());
    m_printerDescriptionLineEdit->setText(printer.getDescription());
    m_printerHostnameLineEdit->setText(printer.getHostname());
    m_printerPortLineEdit->setText(QString::number(printer.getPort()));
    m_printerDefaultPrinterCheckBox->setChecked(printer.getIsDefault());
}

void QDicomPrinterConfigurationWidget::getPrinterSettingsFromControls(DicomPrinter &printer)
{
    printer.setAETitle(m_printerAetitleLineEdit->text());
    printer.setDescription(m_printerDescriptionLineEdit->text());
    printer.setHostname(m_printerHostnameLineEdit->text());
    printer.setPort(m_printerPortLineEdit->text().toInt());
    printer.setIsDefault(m_printerDefaultPrinterCheckBox->isChecked());
}

void QDicomPrinterConfigurationWidget::setAdvancedSettingsToControls(DicomPrinter &printer)
{
    m_magnifactionTypeComboBox->addItems(printer.getAvailableMagnificationTypeValues());
    m_magnifactionTypeComboBox->setCurrentIndex(m_magnifactionTypeComboBox->findText(printer.getDefaultMagnificationType()));
    m_smoothingTypeComboBox->addItems(printer.getAvailableSmoothingTypeValues());
    m_smoothingTypeComboBox->setCurrentIndex(m_smoothingTypeComboBox->findText(printer.getDefaultSmoothingType()));
    m_maximumDensitySpinBox->setMaximum(printer.getAvailableMaxDensityValues());
    m_maximumDensitySpinBox->setValue(printer.getDefaultMaxDensity());
    m_polarityComboBox->addItems(printer.getAvailablePolarityValues());
    m_polarityComboBox->setCurrentIndex(m_polarityComboBox->findText(printer.getDefaultPolarity()));
    m_borderDensityComboBox->addItems(printer.getAvailableBorderDensityValues());
    m_borderDensityComboBox->setCurrentIndex(m_borderDensityComboBox->findText(printer.getDefaultBorderDensity()));
    m_emptyDensityComboBox->addItems(printer.getAvailableEmptyImageDensityValues());
    m_emptyDensityComboBox->setCurrentIndex(m_emptyDensityComboBox->findText(printer.getDefaultEmptyImageDensity()));
    m_minimumDensitySpinBox->setMaximum(printer.getAvailableMinDensityValues());
    m_minimumDensitySpinBox->setValue(printer.getDefaultMinDensity());
    m_yesVisibleTrimRadioButton->setChecked(printer.getDefaultTrim());
    m_noVisibleTrimRadioButton->setChecked(!printer.getDefaultTrim());
    m_supportsAnnotationBoxYesRadioButton->setChecked(printer.getSupportsAnnotationBox());
    m_supportsAnnotationBoxNoRadioButton->setChecked(!printer.getSupportsAnnotationBox());
    m_annotationDisplayFormatLineEdit->setText(printer.getAnnotationDisplayFormatID());
    m_configurationInformationLineEdit->setText(printer.getDefaultConfigurationInformation());
}

void QDicomPrinterConfigurationWidget::getAdvancedSettingsFromControls(DicomPrinter &printer)
{
    printer.setDefaultMagnificationType(m_magnifactionTypeComboBox->currentText());
    printer.setDefaultSmoothingType(m_smoothingTypeComboBox->currentText());
    printer.setDefaultMaxDensity(m_maximumDensitySpinBox->value());
    printer.setDefaultPolarity(m_polarityComboBox->currentText());
    printer.setDefaultBorderDensity(m_borderDensityComboBox->currentText());
    printer.setDefaultEmptyImageDensity(m_emptyDensityComboBox->currentText());
    printer.setDefaultMinDensity(m_minimumDensitySpinBox->value());
    printer.setDefaultTrim(m_yesVisibleTrimRadioButton->isChecked());
    printer.setSupportsAnnotationBox(m_supportsAnnotationBoxYesRadioButton->isChecked());
    printer.setAnnotationDisplayFormatID(m_annotationDisplayFormatLineEdit->text());
    printer.setDefaultConfigurationInformation(m_configurationInformationLineEdit->text());
}

DicomPrinter QDicomPrinterConfigurationWidget::getSelectedDicomPrinter()
{
    DicomPrinter selectedDicomPrinter;
    DicomPrinterManager dicomPrinterManager;
    QTreeWidgetItem *selectedItem;

    if (m_listPrintersTreeWidget->selectedItems().count() > 0)
    {
        // Seleccionem només la primera impressora seleccionada. El QTreeWidget està configurat com SingleSelection, per tant només es pot seleccionar
        // una impressora a la vegada
        selectedItem = m_listPrintersTreeWidget->selectedItems().first();
        m_selectedPrinterId = selectedItem->text(0).toInt();

        selectedDicomPrinter = dicomPrinterManager.getPrinterByID(m_selectedPrinterId);
    }

    return selectedDicomPrinter;
}

void QDicomPrinterConfigurationWidget::cancel()
{
    // Deseleccionem l'impressora si tenim alguna seleccionada i netegem els controls, per si tornen a obrir la interfície de configuració,
    // com que no es crea i es destrueix cada vegada que es fa un show, es mostraria tal com estava abans de fer el cancel d'aquesta manera
    // sempre la mostrem no té cap impressora seleccionada i els controls no tenen valor, es mostra en el seu estat inicial
    m_listPrintersTreeWidget->clearSelection();
    clearPrinterSettings();

    close();
}

void QDicomPrinterConfigurationWidget::accept()
{
    bool closeWindow = false;

    if (m_listPrintersTreeWidget->selectedItems().count() > 0)
    {
        // Si tenim una impressora seleccionada guardem possibles canvis que s'hagin fet, si es produeix algun error guardant els canvis,
        // no tanquem la finestra
        if (modifyPrinter())
        {
            closeWindow = true;
        }
    }
    else
    {
        closeWindow = true;
    }

    if (closeWindow)
    {
        // Deseleccionem l'impressora si tenim alguna seleccionada i netegem els controls, per si tornen a obrir la interfície de configuració,
        // com que no es crea i es destrueix cada vegada que es fa un show, es mostraria tal com estava abans de fer el cancel d'aquesta manera
        // sempre la mostrem no té cap impressora seleccionada i els controls no tenen valor, es mostra en el seu estat inicial

        m_listPrintersTreeWidget->clearSelection();
        clearPrinterSettings();
        close();
    }
}

}
