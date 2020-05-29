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

#ifndef UDGAPPLICATIONUPDATECHECKER_H
#define UDGAPPLICATIONUPDATECHECKER_H

#include <QObject>
class QTimer;
class QUrl;
class QNetworkReply;
class QNetworkAccessManager;

namespace udg {

/**
    Aquesta classe fa una crida al webservice per comprobar si hi ha una versió disponible de starviewer més nova que la que tenim instal·lada.
  */
class ApplicationUpdateChecker : public QObject {
Q_OBJECT

public:
    /// Constructor per defecte
    ApplicationUpdateChecker(QObject *parent = 0);
    /// Destructor
    ~ApplicationUpdateChecker();
    
    /// Retorna l'url, en forma de QString, de les notes de la nova versió. El resultat és correcte sempre que isNewVersionAvailable sigui cert.
    QString getReleaseNotesUrl() const;
    /// Retorna l'url, en forma de QString, la nova versió. El resultat és correcte sempre que isNewVersionAvailable sigui cert.
    QString getVersion() const;
    /// Retorna si hi ha una versió disponible més nova de la que tenim instal·lada.
    bool isNewVersionAvailable() const;

    /// Retorna si no hi ha hagut error
    bool isOnlineCheckOk() const;
    /// Retorna la descripció de l'error. Si el test ha acabat bé, aquest valor no té sentit.
    QString getErrorDescription() const;

    /// Permet especificar el temps que tarda en fer timout. (15 segons per defecte)
    void setTimeout(int milliseconds);
    /// Retorna el temps definit del timout en milisegons
    int getTimout() const;

    /// Retorna si s'està fent una comprovació online en el moment de la crida.
    /// Aquest mètode només té sentit si s'ha de cridar més d'una vegada el mètode (slot) checkForUpdates sobre el mateix objecte.
    bool isChecking() const;

public slots:
    /// Fer la crida al servidor per obtenir si hi ha una nova versió
    void checkForUpdates();

signals:
    /// Senyal per indicar que s'ha acabat de carregar
    void checkFinished();

private:
    /// Genera la url per fer la crida al servidor i obtenir la versió
    QString createWebServiceUrl();
    /// Assigna el proxy per defecte, si n'hi ha, a un QNetworkAccessManager, si no n'hi ha el busca.
    void setProxy(const QUrl &url);

    /// Posa l'atribut finished a true i emet la senyal de checkFinished
    void setCheckFinished();

    /// Comprova que la resposta del servidor web no sigui un error i crida el metode parseJSON
    void parseWebServiceReply(QNetworkReply *reply);
    /// Interpreta la resposta del servidor en format JSON i emplena els atributs corresponents.
    void parseJSON(const QString &json);

protected:
    virtual void performOnlinePetition(const QUrl &url);
    virtual QString readReplyData(QNetworkReply *reply);

protected slots:
    /// Tracta la resposta del webservice obtenint la versió i la url de les notes de la nova versió
    void checkForUpdatesReply(QNetworkReply *reply);
    
    /// Emula la funció de timeout que el QNetworkAccessManager no té
    void checkForUpdatesReplyTimeout();

private:
    /// Indica si ha acabat de carregar-ho tot
    bool m_checkFinished;

    /// Timer per fer el timeout de la resposta del servidor
    QTimer *m_timeoutTimer;

    /// Gestiona les connexions http
    QNetworkAccessManager *m_manager;

    /// La última versió que hi ha al server
    QString m_checkedVersion;
    /// La url de les notes
    QString m_releaseNotesURL;
    /// Si hi ha o no una nova versió
    bool m_updateAvailable;

    /// Si hi ha algun error de connexió amb el servidor
    bool m_checkOk;
    QString m_errorDescription;

    /// Defineix si s'està esperant resposta del servidor
    bool m_isChecking;
};

} // End namespace udg

#endif // UDGAPPLICATIONUPDATECHECKER_H
