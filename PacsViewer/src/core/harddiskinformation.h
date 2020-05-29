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

#ifndef HARDDISKINFORMATION_H
#define HARDDISKINFORMATION_H

#include <QString>

namespace udg {

/**
    Classe que ens permet obtenir informació sobre un disc dur. Més concretament ens permet obtenir els Byte, MB, GB d'una partició i quins d'aquests són lliures.
    Cal tenir present que quan es vol saber l'espai lliure d'un disc dur s'ha d'especificar de quina partició. En windows s'hauria d'especificar la
    unitat (c:, d:, ...) i un directori i en linux s'ha d'especificar la ruta sensera del directori per saber la partició. Actualment aquesta classe només suporta
    els sistemes operatius basats en Unix (Linux, Mac OS X ...).
  */
class HardDiskInformation {
public:
    /// Constructor de la classe
    HardDiskInformation();

    /// Destructor de la classe
    ~HardDiskInformation();

    /// Mètode que ens serveix per saber el número de Bytes total (lliures+ocupats) que hi ha en una partició concreta. Per indicar la
    /// partició s'ha d'especificar el path absolut a un fitxer o directori que estigui en aquesta.
    ///
    /// El mètode no comprova que existeixi el path.
    ///
    /// Si hi ha qualsevol error el mètode retornarà 0.
    /// @param path Indica el path a un directori/fitxer dintre de la partició
    /// @return El nombre total de bytes que té una partició
    quint64 getTotalNumberOfBytes(const QString &path);

    /// Retorna el número de Bytes d'espai lliure que ens queden en una partició concreta i que poden ser utilitzats per l'usuari. Cal fer
    /// notar que aquest número pot ser diferent del nombre real de bytes lliures (per exemple en casos que hi hagi quotes per usuari). Per
    /// indicar la partició s'ha d'especificar el path absolut a un fitxer o directori que estigui en aquesta.
    ///
    /// El mètode no comprova que existeixi el path.
    ///
    /// Si hi ha qualsevol error el mètode retornarà 0.
    /// @param path Indica el path a un directori/fitxer dintre de la partició
    /// @return El nombre de bytes lliures que té una partició i poden ser utilitzats per l'usuari que executa el programa
    quint64 getNumberOfFreeBytes(const QString &path);

    /// Es comporta exactament igual que getTotalNumberOfBytes() però retorna MBytes en comptes de Bytes. Cal tenir en compte, però, que
    /// aquest no és un mètode per saber el número de MBytes amb absoluta precissió (per això fer servir getTotalNumberOfBytes())
    /// @param path Indica el path a un directori/fitxer dintre de la partició
    /// @return El nombre de MBytes truncats (ex.: si és 1,9MBytes reals retornarà 1Mbytes)
    quint64 getTotalNumberOfMBytes(const QString &path);

    /// Es comporta exactament igual que getNumberOfFreeBytes() però retorna MBytes en comptes de Bytes. Cal tenir en compte, però, que
    /// aquest no és un mètode per saber el número de MBytes amb absoluta precissió (per això fer servir getNumberOfFreeBytes())
    /// @param path Indica el path a un directori/fitxer dintre de la partició
    /// @return El nombre de MBytes lliures truncats (ex.: si és 1,9MBytes reals retornarà 1MByte)
    quint64 getNumberOfFreeMBytes(const QString &path);

    /// Ens retorna els Bytes que ocupa el contingut del directori donat
    static qint64 getDirectorySizeInBytes(const QString &directoryPath);

private:
    quint64 getTotalBytesPlataformEspecific(const QString &path);
    quint64 getFreeBytesPlataformEspecific(const QString &path);

    /// Loggeja l'últim error segons la plataforma
    void logLastError(const QString &additionalInformation);

};

};  // End namespace udg

#endif
