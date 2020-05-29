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

#ifndef QCOLORTRANSFERFUNCTIONGRAPHICALVIEW_H
#define QCOLORTRANSFERFUNCTIONGRAPHICALVIEW_H

#include <QGraphicsView>

namespace udg {

class ColorTransferFunction;

/**
    Vista gràfica d'una funció de transferència de color.
  */
class QColorTransferFunctionGraphicalView : public QGraphicsView {
Q_OBJECT

public:
    /// Constructor.
    explicit QColorTransferFunctionGraphicalView(QWidget *parent = 0);

    /// Assigna la funció de transferència de color.
    void setColorTransferFunction(const ColorTransferFunction &colorTransferFunction);
    /// Demana que s'actualitzi el fons després de moure el ratolí
    /// \TODO Es podria fer privat i el node com a classe aniuada privada.
    void requestBackgroundUpdate();
    // void fitInView();    // per ajustar el zoom automàticament

signals:
    /// S'emet quan s'afegeix un node.
    void nodeAdded(double x, const QColor &color);
    /// S'emet quan s'esborra un node.
    void nodeRemoved(double x);
    /// S'emet quan es mou un node.
    void nodeMoved(double origin, double destination);
    /// S'emet quan es mou més d'un node.
    void nodesMoved(const QList<double> &origins, double offset);
    /// S'emet quan es canvia el color d'un node.
    void nodeChangedColor(double x, const QColor &color);

protected:
    ///@{
    /// Gestió d'esdeveniments. \todo Explicar una mica què fem a cadascun.
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    ///@}

private:
    Q_DISABLE_COPY(QColorTransferFunctionGraphicalView)

    /// Actualitza el gradient del fons.
    void updateBackground();
    /// Afegeix un nou node a x.
    void addNode(double x);
    /// Esborra un node a x si existeix.
    void removeNode(double x);
    /// Fa els preparatius per començar a moure nodes.
    void beginMoveNodes();
    /// Finalitza el moviment de nodes.
    void endMoveNodes();
    /// Canvia el color del node a x si existeix.
    void changeNodeColor(double x);

private:
    /// Nivell de zoom actual.
    double m_zoom;
    /// Indica si s'ha d'actualitzar el fons després de moure el ratolí.
    bool m_backgroundUpdateRequested;

};

} // End namespace udg

#endif // QCOLORTRANSFERFUNCTIONGRAPHICALVIEW_H
