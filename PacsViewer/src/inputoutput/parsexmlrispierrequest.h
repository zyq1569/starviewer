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


#ifndef UDGPARSEXMLRISPIERREQUEST_H
#define UDGPARSEXMLRISPIERREQUEST_H

#include <QObject>
#include <QXmlStreamReader>

namespace udg {

class DicomMask;

/**
    Classe que s'encarrega de parsejar el XML que ens envia el RIS PIER (RIS que utilitzen des d'escriptori remot la majoria de
    centres de l'Institut Català de la Salut que fan diagnòstic per la imatge), converteix el XML en una DicomMask per poder
    descarregar l'estudi que ens indiqui el RIS. El format del XML és
         <?xml version="1.0" encoding="UTF-8"?>
             <Msg Name="OpenStudies">
                 <Param Name="AccessionNumber">00239RS00006780</Param>
             </Msg>

    En principi el format del XML sempre és igual, no hi ha cap més tag, només envien com a paràmetre per cercar l'estudi
    l'accesionm number ja que és l'únic paràmetre que els RIS coneixen d'un estudi
  */
class ParseXmlRisPIERRequest : QObject {
Q_OBJECT
public:
    /// Parseja la cadena de XML i retorna la DicomMask de l'estudi a descarregar
    DicomMask parseXml(QString xmlRisPierRequest);

    /// Retorna si s'ha produït un error alhora de parsejar el Xml
    bool error();

private:
    bool m_errorParsing;

    /// Parseja el tag Msg del Xml i retorna la DicomMask amb el accession number que ens han enviat el RIS en el Xml i indicant que cal retorni els camps
    /// Study Id, Study UID, Patient Id, Patient Name i PAtient ID al fer la query a un PACS amb aquesta màscara
    void parseTagMsg(QXmlStreamReader *xmlReader, DicomMask *mask);

    /// Parseja el tag Param del Xml i retorna la DicomMask amb el accession number que ens han enviat en el Xml
    void parseTagParam(QXmlStreamReader *xmlReader, DicomMask *mask);

    /// Degut al fet que perquè el Pacs ens retorni un determinar tag, li hem d'enviar aquell tag amb cadena buida quan fem la petició "", en aquest mètode
    /// donem valor de cadena buida a tots aquells camps que volem que ens retorni el PACS quan fer la cerca de l'estudi demanat pel RIS. Aquests tags són
    /// PatientName, PatientId, StudyUID, StudyID, els tags més bàsics per tenir informació
    void buildBasicDicomMask(DicomMask *mask);

    /// Indiquem que s'ha produït un error al parsejar el Xml
    void setErrorParsing(bool errorParsing);

};

}

#endif
