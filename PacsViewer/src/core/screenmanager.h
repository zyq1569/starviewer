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

#ifndef UDGSCREENMANAGER_H
#define UDGSCREENMANAGER_H

#include "dynamicmatrix.h"
#include "screenlayout.h"

class QWidget;
class QDesktopWidget;
class QRect;

namespace udg {

/**
    Aquesta classe s'utilitza per dues funcionalitats principals, maximitzar una finestra quan hi ha múltiples pantalles,
    i moure una finestra d'una pantalla a una altra.
    Aquesta també s'encarrega de calcular quin és el layout de pantalles (ScreenLayout) al sistema.
    El seu funcionament bàsic és a través dels mètodes maximize i moveToDesktop, als que se li passa la finestra a tractar.
  */
class ScreenManager {
public:
    /// Constructor per defecte.
    ScreenManager();
    
    /// Ens retorna el layout actual
    ScreenLayout getScreenLayout() const;
    
    /// Mètode que maximitza la finestra passada per paràmetres a tantes pantalles com sigui possible per tal de que es vegi bé.
    void maximize(QWidget *window);
    /// Returns true if the given window is maximized to more than one screen and false otherwise (including when it's simply maximized or not maximized).
    bool isMaximizedToMultipleScreens(QWidget *window);
    /// Mètode per canviar la finestra especificada per el primer paràmetre a la pantalla amb l'identificador del segon paràmetre.
    void moveToDesktop(QWidget *window, int idDesktop);
    /// Mètode per moure la finestra a la pantalla anterior segons l'identificador de pantalla
    void moveToPreviousDesktop(QWidget *window);
    /// Mètode per moure la finestra a la següent pantalla segons l'identificador de pantalla
    void moveToNextDesktop(QWidget *window);

    ///Restaura la finestra a l'estat d'abans d'estar minimitzada
    void restoreFromMinimized(QWidget *window);
    
    /// Retorna a quina pantalla es troba el widget/coordenada
    int getScreenID(QWidget *window) const;
    int getScreenID(const QPoint &point) const;

    /// Calcula la matriu de distribució de les pantalles.
    /// Utilitza la classe DynamicMatrix per crear-se l'estructura de dades per representar la distribució de
    /// les pantalles a l'espai.
    DynamicMatrix computeScreenMatrix(QWidget *window) const;

private:
    /// Prepara l'screen layout segons la configuració actual
    void setupCurrentScreenLayout();
    
    /// Retorna si la finestra passada per paràmetre cap dins la pantalla.
    bool doesItFitInto(QWidget *window, int IdDesktop);
    /// Modifica el tamany de la finestra passada per paràmtre per tal de que càpiga a la pantalla amb id IdDesktop.
    /// A més, centra la finestra.
    void fitInto(QWidget *window, int IdDesktop);

    /// Retorna el punt de més adalt a l'esquerra segons la geometria disponible.
    /// (tenint en compte la barra de tasques)
    QPoint getTopLeft(const DynamicMatrix &dynamicMatrix) const;
    /// Retorna el punt de més aball a la dreta segons la geometria disponible.
    /// (tenint en compte la barra de tasques)
    QPoint getBottomRight(const DynamicMatrix &dynamicMatrix) const;

    /// Returns the geometry that should be applied to the given window to maximize it to multiple screens.
    /// If the window can't be maximized to multiple screens returns a null QRect.
    QRect getGeometryToMaximizeToMulipleScreens(QWidget *window);

private:
    /// Gestor d'escriptori on es fan les crides referents al múltiples escriptoris o pantalles
    QDesktopWidget *m_applicationDesktop;

    /// Distribució de pantalles
    ScreenLayout m_screenLayout;
};

} // End namespace udg

#endif // UDGSCREENMANAGER_H
