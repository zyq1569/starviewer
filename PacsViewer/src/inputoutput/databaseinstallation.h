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

#ifndef UDGDATABASEINSTALLATION_H
#define UDGDATABASEINSTALLATION_H

#include <QObject>
#include <QProgressDialog>

namespace udg {

/**
    Aquesta classe comprova que els directoris i la base de dades de la cache estiguin correctament creats si no es aquest el cas, els crea, per a que
    l'aplicacio pugui funcionar correctament
  */
class DatabaseInstallation : public QObject {
Q_OBJECT
public:
    DatabaseInstallation();
    ~DatabaseInstallation();

    /// Comprova que el directori on es guarda la base dades, les imatges i la base de dades existeixin sinó l'intenta crear.
    /// També comprova que la base de dades estigui en la revisió que necessita la compilació actual de l'starviewer i sinó la
    /// intenta actualitzar
    /// @return indica si la base de dades existeix
    bool checkStarviewerDatabase();

    /// Comprova si existeix el directori de la base de dades
    /// @return indica si el directori existeix
    bool existsDatabasePath();

    /// Comprova si existeix el directori on es guarden les imatges descarregades
    /// @return indica si el directori existeix
    bool existsLocalImagePath();

    /// Comprova si existeix el fitxer de la base de dades
    /// @return indica si el directori existeix
    bool existsDatabaseFile();

    /// Esborra la base de dades actual i torna a crear-ne una de nova
    bool reinstallDatabase();

    /// Reinstal·lar la base de dades i esborra les imatges descarregades o importades a la base de dades local. Mostra un QProgressDialog mentre es neteja
    /// la cache.
    bool removeCacheAndReinstallDatabase();

    /// Aplica els canvis a fets a la última revisió de la base de dades a la base de dades locals
    bool recreateDatabase();

    /// Retorna els errors que s'han trobat
    QString getErrorMessage();

private slots:
    /// Fa avançar la barra de progrés
    void setValueProgressBar();

private:
    /// Comprova que el directori on es guarden les imatges descarregades existeixi si no l'intenta crear
    /// @return indicat si el directori existeix o no
    bool checkLocalImagePath();

    /// Comprova que existeix el path de la base de dades i sinó existeix el crea
    bool checkDatabasePath();

    /// Comprova si la revisió de la base de dades és la necessària per l'actual compilació de l'starviewer i sinó l'intenta actualitzar
    bool checkDatabaseRevision();

    /// Retorna cert si tenim permisos d'escriptura a la base de dades, fals altrament
    bool isDatabaseFileWritable();

    /// Crea el directori per guardar les imatges de la cache
    /// @return indica si s'ha pogut crear el directori
    bool createLocalImageDir();

    /// Crea el directori per guardar la base de dades
    /// @return indica si s'ha pogut crear el directori
    bool createDatabaseDirectory();

    /// Crea la base de dades
    /// @return indica si s'ha pogut crear la base de dades
    bool createDatabaseFile();

    /// Intenta actualitzar la base de dades sinó és possible esborra la caché actual i crea una bd nova buid
    bool tryToUpgradeDatabaseIfNotRecreateDatabase();

    /// Comprova si la versió actual de la base de dades es pot actualitzar, sense necessitat d'haver d'esborrar la caché i tornar-la a crear
    bool canBeUpgradedDatabase();

    /// Aplica els canvis necessaris a la base de dades per poder funcionar amb la versió actual de Starviewer, sempre i quan la BD sigui actualitzable
    bool upgradeDatabase();

    /// Ens aplica la comanda d'actualització a la base de dades
    bool applySqlUpgradeCommandToDatabase(QString sqlUpgradeCommand);

    /// Retorna el XML a aplicar per actualitzar la base de dades
    QString getUpgradeDatabaseRevisionXmlData();

    /// Pregunta l'usuari si vol fer un Donwgrade de la base dades, això implicar esborrar la caché actuai tornar a crear la BD
    bool askToUserIfDowngradeDatabase();

private:
    /// Diàleg de progrés per les operacions costoses
    QProgressDialog *m_qprogressDialog;

    /// Missatges d'errors que s'han anat produint
    QString m_errorMessage;
};

} // End namespace udg

#endif
