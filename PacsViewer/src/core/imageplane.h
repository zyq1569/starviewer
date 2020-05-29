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

#ifndef UDGIMAGEPLANE_H
#define UDGIMAGEPLANE_H

#include <QList>
#include <QVector>

#include "imageorientation.h"
#include "pixelspacing2d.h"

namespace udg {

class Image;
/**
    Classe que defineix un pla d'imatge. El pla consta de vectors de direcció de X i Y, normal, origen, espaiat i nombre de files i columnes per definir un pla
    tancat i finit com és el de la imatge. Està definit en coordenades de món.
  */
class ImagePlane {
public:
    ImagePlane();

    ImagePlane(ImagePlane *imagePlane);

    ~ImagePlane();

    void setImageOrientation(const ImageOrientation &imageOrientation);
    ImageOrientation getImageOrientation() const;

    /// TODO Mètodes de transició. Es mantenen en la primera fase de refactoring d'ImageOrientation
    /// Amb el temps hauríem de prescindir d'aquests i accedir a través dels que ens proporciona ImageOrientation
    void getRowDirectionVector(double vector[3]);
    void getColumnDirectionVector(double vector[3]);
    void getNormalVector(double vector[3]);

    void setOrigin(double origin[3]);
    void setOrigin(double x, double y, double z);
    void getOrigin(double origin[3]);

    void setSpacing(const PixelSpacing2D &spacing);
    PixelSpacing2D getSpacing() const;

    void setThickness(double thickness);
    double getThickness() const;

    void setRows(int rows);
    void setColumns(int columns);

    int getRows() const;
    int getColumns() const;

    double getRowLength() const;
    double getColumnLength() const;

    /// Omple les dades del pla a partir d'un objecte Image
    /// @param image objecte Image
    /// @return Cert si l'objecte Image és vàlid per omplir les dades, fals altrament
    bool fillFromImage(const Image *image);

    bool operator ==(const ImagePlane &imagePlane);
    bool operator !=(const ImagePlane &imagePlane);

    /// Ens retorna una llista amb 4 punts que defineix els bounds del pla.
    /// Tenint en compte que la coordenada d'origen s'assumeix que és al centre de la llesca ( és a dir, enmig +/- thickness )
    /// Podem obtenir els bounds respecte el centre, thickness per amunt o thickness per avall
    /// L'ordre dels punts retornats és el següent:
    /// TLHC, TRHC, BRHC, BLHC,
    /// On:
    /// TLHC == TopLeftHandCorner == Origen
    /// TRHC == TopRightHandCorner
    /// BRHC == BottomRightHandCorner
    /// BLHC == BottomLeftHandCorner
    /// @param location defineix quins bounds volem, 0: Central, 1: Upper (+thickness/2), 2: Lower (-thickness/2)
    QList<QVector<double> > getBounds(int location);
    QList<QVector<double> > getCentralBounds();
    QList<QVector<double> > getUpperBounds();
    QList<QVector<double> > getLowerBounds();

    /// Retorna un volcat d'informació de l'objecte en format d'string
    QString toString(bool verbose = false);

    /// Ens dóna els punts d'intersecció entre el pla localitzador passat per paràmetre i el pla
    /// @param planeToIntersect pla que volem fer intersectar amb el pla
    /// @param firstIntersectionPoint[] primer punt d'intersecció trobat ( si n'hi ha )
    /// @param secondIntersectionPoint[] segon punt d'intersecció trobat ( si n'hi ha )
    /// @param bounds Which plane bounds to take to make the intersection computing. 0 = upper, 1 = lower, 2 = central, upper by default.
    /// @return el nombre d'interseccions trobades
    int getIntersections(ImagePlane *planeToIntersect, double firstIntersectionPoint[3], double secondIntersectionPoint[3], int bounds = 0);

    /// Returns the distance from the given point to the current plane
    double getDistanceToPoint(double point[3]);
    
    /// Computes the projection of pointToProject on this image plane and sets it on projectedPoint parameter
    void projectPoint(const double pointToProject[3], double projectedPoint[3], bool vtkReconstructionHack = false);
    
    /// Quan es modifica algun dels vectors directors del pla es modifica el centre.
    void updateCenter();

    void getCenter(double center[3]);

    /// Assigna un nou centre al pla i per tant modifica l'origen, els dos vectors de direcció (row i column) i el vector normal.
    void setCenter(double x, double y, double z);
    void setCenter(double center[3]);

    /// Trasllada el pla en la direcció de la normal en la distància especificada.
    /// Valors negatius mouen el pla en la direcció oposada.
    void push(double distance);

private:
    /// Orientació del pla imatge
    ImageOrientation m_imageOrientation;

    /// Origen del pla
    double m_origin[3];

    /// Espaiat de les X i les Y
    PixelSpacing2D m_spacing;

    /// Files i columnes
    int m_rows, m_columns;

    /// Gruix del pla
    double m_thickness;

    /// Centre del pla
    double m_center[3];
};

}

#endif
