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

#ifndef STATUS
#define STATUS

#include <QString>

class OFCondition;

namespace udg {

/**
    Aquesta classe proporciona el control d'errors, dels mètodes invocats.
    Aquesta classe, està basada seguin el patró Abstract Factory, el que es tracta és de separar el control d'errors del dctmk, de control
    d'errors que utilitzarem nosaltres en la nostre aplicació, aixi si mai el control d'errors de dcmtk mai canvia, només haurem de modificar aquesta classe
    i mai haurem de tocar les classes de la nostra aplicació que utilitzi el tractament d'errors.
    En aquesta classe encapsulem el tractament d'errors.
    Per englobar el tractament d'errors de tota l'aplicació hi podem tenir dos objectes d'error el OFCondition de dcmtk, i un altre que es propi format per un
    QString que descriu l'error,un boolea que descriu si l'estat és bo, i un enter que descriu el número de l'error, d'aquesta manera, amb aquesta classe també
    podem encapsular els errors que no siguin de dctmk, en una sola classe per fer més fàcil la gestió dels errors, ja que per l'aplicació, només existirà
    l'objecte status que controla l'error. Independentment si és un error dcmtk o provinent d'altres fonts com la caché. Només podem inserir un dels dos tipus
    d'error!
  */
class Status {

public:
    Status();

    /// Retorna un text descrivint l'error o l'exit del procés
    /// @return text descriptiu de l'exit o l'error de la crida
    QString text() const;

    /// Per contruir l'objecte status
    /// @param OFCodition resultant d'una crida
    /// @return retorna l'objecte status
    Status setStatus(const OFCondition);

    /// Crea un objecte Status
    /// @param descripció de l'status
    /// @param indica si s'ha finalitzat amb èxit
    /// @param número d'erro en cas d'haver-ni
    /// @return retorna l'objecte status
    Status setStatus(QString, bool, int);

    /// Retorna si l'èxit de l'element cridat
    /// @return boolea indicant l'exit de la crida
    bool good() const;

    /// Retorna el codi d'estat
    /// @return codi d'estat
    int code() const;

private:
    QString m_descText;
    bool m_success;
    int m_numberError;
};
}; // end namespace udg
#endif
