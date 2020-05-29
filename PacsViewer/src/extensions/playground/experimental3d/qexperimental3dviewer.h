#ifndef UDGQEXPERIMENTAL3DVIEWER_H
#define UDGQEXPERIMENTAL3DVIEWER_H

#include "qviewer.h"

#include "vector3.h"

namespace udg {

class Experimental3DVolume;

/**
    Visualitzador de l'extensió Experimental 3D.
 */
class QExperimental3DViewer : public QViewer {
Q_OBJECT
public:
    QExperimental3DViewer(QWidget *parent = 0);
    virtual ~QExperimental3DViewer();

    /// Indiquem les dades d'entrada.
    virtual void setInput(Volume *volume);

    /// Assigna el volum.
    void setVolume(Experimental3DVolume *volume);
    /// Elimina el volum actual de la visualització.
    void removeCurrentVolume();

    /// Retorna el color de fons actual.
    QColor getBackgroundColor();
    /// Posa el color de fons a \a color.
    void setBackgroundColor(QColor color);

    /// Actualitza la taula d'ombreig (necessari per fer servir el voxel shader d'il·luminació directa).
    void updateShadingTable();

    /// Retorna els paràmetres de la càmera.
    void getCamera(Vector3 &position, Vector3 &focus, Vector3 &up);
    /// Estableix els paràmetres de la càmera.
    void setCamera(const Vector3 &position, const Vector3 &focus, const Vector3 &up);

    void screenshot(const QString &fileName);

public slots:
    /// Força l'execució de la visualització.
    virtual void render();

    /// Crida que reinicia a l'estat inicial el visor.
    virtual void reset();

    /// Li indiquem quina vista volem del volum: XYPlane, YZPlane o XZPlane.
    virtual void resetView(const OrthogonalPlane &view);

protected:
    void getCurrentRenderedItemBounds(double bounds[6]);

private:
    /// El volum.
    Experimental3DVolume *m_volume;

};

} // namespace udg

#endif // UDGQEXPERIMENTAL3DVIEWER_H
