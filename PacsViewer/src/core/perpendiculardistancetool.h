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

#ifndef UDGPERPENDICULARDISTANCETOOL_H
#define UDGPERPENDICULARDISTANCETOOL_H

#include "genericdistancetool.h"

namespace udg {

/**
    Eina per fer mesures TA-GT.

    El funcionament és el següent:
    - Primer es dibuixa una línia.
    - Un cop dibuixada la línia es traça una línia perpendicular a la primera línia segons la posició del mouse.
    - Un cop acabada la segona línia, es traça una tercera línia paral·lela a l'anterior.
    - Un cop fixada la línia paral·lela, apareix la distància entre les línies paral·leles.

    Per marcar cadascun dels punts es farà amb un clic o doble-clic amb el botó esquerre del ratolí.
    Un cop marcat el primer punt, es mostrarà una línia que unirà el primer punt marcat amb el punt on es trobi el punter del ratolí.
    Un cop marcat el segon punt, es mostrarà una línia perpendicular a la primera que unirà aquesta amb el punt on es trobi el punter del ratolí.
    Un cop marcat el tercer punt, es mostrarà una línia perpendicular a la primera i paral·lela a la segona que unirà aquesta amb el punt on es trobi el punter
    del ratolí.
    Mentre l'usuari no hagi marcat l'últim punt, no es mostrarà cap informació respecte a la distància en aquell moment.

    Un cop marcat l'últim punt, la mesura es mostrarà en mil·límetres si existeix informació d'espaiat i en píxels altrament.

    Un cop acabada la mesura, es poden annotar successivament tantes mesures com es desitgi.

    Les mesures anotades només apareixeran sobre les imatges sobre les que s'han realitzat, però no s'esborren si canviem d'imatge.
    Si canviem d'imatge però tornem a mostrar-la més endavant les anotacions fetes sobre aquesta anteriorment tornaran a aparèixer.

    Quan es canviï l'input del visor, les anotacions fetes fins aquell moment s'esborraran.
    Quan es desactivi l'eina, les anotacions fetes fins aquell moment es mantindran.

    TODO Canviar el nom de la classe.
  */
class PerpendicularDistanceTool : public GenericDistanceTool {

Q_OBJECT

public:

    PerpendicularDistanceTool(QViewer *viewer, QObject *parent = 0);
    ~PerpendicularDistanceTool();

protected:
    void handleLeftButtonPress();
    void handleMouseMove();
    void abortDrawing();

private:

    /// Possibles estats de l'eina.
    enum State { NotDrawing, DrawingReferenceLine, DrawingFirstPerpendicularLine, DrawingSecondPerpendicularLine };

private:

    /// Decideix què s'ha de fer quan es rep un clic.
    void handleClick();
    /// Afegeix el primer punt, per començar a dibuixar la línia de referència.
    void addFirstPoint();
    /// Afegeix el segon punt, per acabar de dibuixar la línia de referència i començar la primera perpendicular.
    void addSecondPoint();
    /// Afegeix el tercer punt, per acabar de dibuixar la primera línia perpendicular i començar la segona.
    void addThirdPoint();
    /// Afegeix el quart i últim punt, per acabar de dibuixar la segona línia perpendicular i calcular la distància.
    void addFourthPoint();
    /// Actualitza la línia de referència perquè vagi del primer punt a la posició actual del ratolí.
    void updateReferenceLine();
    /// Actualitza la primera línia perpendicular perquè sigui perpendicular a la línia de referència i vagi des d'aquesta a la posició actual del ratolí (o al
    /// punt més proper de manera que encara toqui amb la línia de referència).
    void updateFirstPerpendicularLine();
    /// Actualitza la segona línia perpendicular perquè sigui perpendicular a la línia de referència i pel mateix costat que la primera línia perpendicular i
    /// vagi des d'aquesta a la posició actual del ratolí (o al punt més proper de manera que encara toqui amb la línia de referència).
    void updateSecondPerpendicularLine();
    /// Actualitza la línia de referència i refresca la visualització.
    void updateReferenceLineAndRender();
    /// Actualitza la primera línia perpendicular i refresca la visualització.
    void updateFirstPerpendicularLineAndRender();
    /// Actualitza la segona línia perpendicular i refresca la visualització.
    void updateSecondPerpendicularLineAndRender();
    /// Calcula la distància i la dibuixa.
    void drawDistance();
    /// Dibuixa la línia de distància.
    void drawDistanceLine();

    /// Equalitza la profunditat dels elements que formen l'anotació TA-GT final.
    void equalizeDepth();

private slots:

    /// Torna l'eina al seu estat inicial.
    void reset();

private:

    /// La línia de referència, per fer després dues línies perpendiculars a aquesta.
    QPointer<DrawerLine> m_referenceLine;
    /// La primera línia perpendicular a la de referència, que és un extrem de la distància.
    QPointer<DrawerLine> m_firstPerpendicularLine;
    /// La segona línia perpendicular a la de referència, que és l'altre extrem de la distància.
    QPointer<DrawerLine> m_secondPerpendicularLine;

    /// Estat de l'eina.
    State m_state;

};

}

#endif
