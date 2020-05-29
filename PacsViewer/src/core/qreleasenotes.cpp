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

#include "qreleasenotes.h"
#include "coresettings.h"
#include "logging.h"

#include <QCloseEvent>
#include <QUrl>
#include <QNetworkReply>
#include <QWebHistory>

namespace udg {

QReleaseNotes::QReleaseNotes(QWidget *parent)
 : QWidget(parent)
{
    setupUi(this);

    // No cal fer un metode a part per les connexions si només en tenim una
    connect(m_closePushButton, SIGNAL(clicked()), this, SLOT(close()));

    // Fer que la finestra sempre quedi davant i no es pugui fer res fins que no es tanqui
    setWindowModality(Qt::ApplicationModal);

    m_viewWebView->setContextMenuPolicy(Qt::NoContextMenu);
    m_viewWebView->history()->setMaximumItemCount(0);
}

QReleaseNotes::~QReleaseNotes()
{
}

bool QReleaseNotes::isDontShowAnymoreChecked()
{
    return m_dontShowCheckBox->isChecked();
}

void QReleaseNotes::setDontShowVisible(bool visible)
{
    m_dontShowCheckBox->setVisible(visible);
}

void QReleaseNotes::showIfUrlLoadsSuccessfully(const QUrl &url)
{
    connect(m_viewWebView->page()->networkAccessManager(), SIGNAL(finished(QNetworkReply*)), this, SLOT(loadFinished(QNetworkReply*)));
    m_viewWebView->setUrl(url);
}

void QReleaseNotes::closeEvent(QCloseEvent *event)
{
    Settings settings;
    // Si el 'Don't show on future releases' esta marcat
    if (isDontShowAnymoreChecked())
    {
        // Modificar els settings per que no es mostrin mai més
        settings.setValue(CoreSettings::NeverShowNewVersionReleaseNotes, true);
    }
    // I tancar la finestra
    event->accept();
}

void QReleaseNotes::loadFinished(QNetworkReply *reply)
{
    // Desconectar el manager
    disconnect(m_viewWebView->page()->networkAccessManager(), SIGNAL(finished(QNetworkReply*)), this, SLOT(loadFinished(QNetworkReply*)));

    if (reply->error() == QNetworkReply::NoError)
    {
        // Si no hi ha hagut error, mostrar
        show();
    }
    else
    {
        ERROR_LOG(QString("Error en carregar la url, tipus ") + QString::number(reply->error())+
                  QString(": ") + reply->errorString());
    }
    reply->deleteLater();
}

}; // End namespace udg
