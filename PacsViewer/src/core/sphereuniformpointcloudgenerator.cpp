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

#include "sphereuniformpointcloudgenerator.h"

#include "mathtools.h"
#include "vector3.h"

namespace udg {

SphereUniformPointCloudGenerator::SphereUniformPointCloudGenerator(float radius, unsigned char depth)
{
    m_radius = radius;
    m_depth = depth;
}

SphereUniformPointCloudGenerator::~SphereUniformPointCloudGenerator()
{
}

void SphereUniformPointCloudGenerator::createPOVCloud()
{
    m_vertices.clear();
    m_faces.clear();
    m_normals.clear();
    m_geographicVertices.clear();

    Vector3 v;

    // Create the initial vertices from the sphere cloud
    createIcosahedronVertices();

    // Create the initial faces from the sphere cloud
    createIcosahedronFaces();

    FaceList tempFaces = m_faces;

    m_faces.clear();

    // Each triangle has to be subdivided as many times as the sphere depth
    for (unsigned short j = 0; j < tempFaces.size(); j += 3)
    {
        unsigned short face1 = tempFaces[j];
        unsigned short face2 = tempFaces[j + 1];
        unsigned short face3 = tempFaces[j + 2];

        subdivide(m_vertices[face1], m_vertices[face2], m_vertices[face3], m_depth);
    }

    // We scale all vertices about the sphere radius due to all vertices are situed at radius 1 of the center of the sphere
    for (unsigned short i = 0; i < m_vertices.size(); i++)
    {
        m_vertices[i] *= m_radius;
    }

    createGeographicVertices();
}

const QVector<Vector3> & SphereUniformPointCloudGenerator::getVertices() const
{
    return m_vertices;
}

const QVector<Vector3> & SphereUniformPointCloudGenerator::getGeographicVertices() const
{
    return m_geographicVertices;
}

QVector<QVector<int> > SphereUniformPointCloudGenerator::getNeighbours() const
{
    QVector<QVector<int> > neighbours;
    int nVertices = m_vertices.size();
    int nFaces = m_faces.size() / 3;
    Q_ASSERT(nFaces * 3 == m_faces.size());

    for (int i = 0; i < nVertices; i++)
    {
        QVector<int> vertexNeighbours;

        for (int j = 0; j < nFaces; j++)
        {
            if (m_faces.at(3 * j) == i)
            {
                if (!vertexNeighbours.contains(m_faces.at(3 * j + 1)))
                {
                    vertexNeighbours << m_faces.at(3 * j + 1);
                }
                if (!vertexNeighbours.contains(m_faces.at(3 * j + 2)))
                {
                    vertexNeighbours << m_faces.at(3 * j + 2);
                }
            }

            if (m_faces.at(3 * j + 1) == i)
            {
                if (!vertexNeighbours.contains(m_faces.at(3 * j)))
                {
                    vertexNeighbours << m_faces.at(3 * j);
                }
                if (!vertexNeighbours.contains(m_faces.at(3 * j + 2)))
                {
                    vertexNeighbours << m_faces.at(3 * j + 2);
                }
            }

            if (m_faces.at(3 * j + 2) == i)
            {
                if (!vertexNeighbours.contains(m_faces.at(3 * j)))
                {
                    vertexNeighbours << m_faces.at(3 * j);
                }
                if (!vertexNeighbours.contains(m_faces.at(3 * j + 1)))
                {
                    vertexNeighbours << m_faces.at(3 * j + 1);
                }
            }
        }

        neighbours << vertexNeighbours;
    }

    return neighbours;
}

void SphereUniformPointCloudGenerator::createIcosahedronVertices()
{
    Vector3 v(1.0, 0.0, (1.0 + sqrt(5.0)) / 2.0);
    v.normalize();

    const double ICOSAHEDRON_X = v.x;
    const double ICOSAHEDRON_Z = v.z;

    m_vertices.append(Vector3(-ICOSAHEDRON_X, 0.0, ICOSAHEDRON_Z));
    m_vertices.append(Vector3(ICOSAHEDRON_X, 0.0, ICOSAHEDRON_Z));
    m_vertices.append(Vector3(-ICOSAHEDRON_X, 0.0, -ICOSAHEDRON_Z));
    m_vertices.append(Vector3(ICOSAHEDRON_X, 0.0, -ICOSAHEDRON_Z));
    m_vertices.append(Vector3(0.0, ICOSAHEDRON_Z, ICOSAHEDRON_X));
    m_vertices.append(Vector3(0.0, ICOSAHEDRON_Z, -ICOSAHEDRON_X));
    m_vertices.append(Vector3(0.0, -ICOSAHEDRON_Z, ICOSAHEDRON_X));
    m_vertices.append(Vector3(0.0, -ICOSAHEDRON_Z, -ICOSAHEDRON_X));
    m_vertices.append(Vector3(ICOSAHEDRON_Z, ICOSAHEDRON_X, 0.0));
    m_vertices.append(Vector3(-ICOSAHEDRON_Z, ICOSAHEDRON_X, 0.0));
    m_vertices.append(Vector3(ICOSAHEDRON_Z, -ICOSAHEDRON_X, 0.0));
    m_vertices.append(Vector3(-ICOSAHEDRON_Z, -ICOSAHEDRON_X, 0.0));

    m_normals.append(Vector3(-ICOSAHEDRON_X, 0.0, ICOSAHEDRON_Z));
    m_normals.append(Vector3(ICOSAHEDRON_X, 0.0, ICOSAHEDRON_Z));
    m_normals.append(Vector3(-ICOSAHEDRON_X, 0.0, -ICOSAHEDRON_Z));
    m_normals.append(Vector3(ICOSAHEDRON_X, 0.0, -ICOSAHEDRON_Z));
    m_normals.append(Vector3(0.0, ICOSAHEDRON_Z, ICOSAHEDRON_X));
    m_normals.append(Vector3(0.0, ICOSAHEDRON_Z, -ICOSAHEDRON_X));
    m_normals.append(Vector3(0.0, -ICOSAHEDRON_Z, ICOSAHEDRON_X));
    m_normals.append(Vector3(0.0, -ICOSAHEDRON_Z, -ICOSAHEDRON_X));
    m_normals.append(Vector3(ICOSAHEDRON_Z, ICOSAHEDRON_X, 0.0));
    m_normals.append(Vector3(-ICOSAHEDRON_Z, ICOSAHEDRON_X, 0.0));
    m_normals.append(Vector3(ICOSAHEDRON_Z, -ICOSAHEDRON_X, 0.0));
    m_normals.append(Vector3(-ICOSAHEDRON_Z, -ICOSAHEDRON_X, 0.0));
}

void SphereUniformPointCloudGenerator::createIcosahedronFaces()
{
    m_faces.append(1); m_faces.append(4); m_faces.append(0);
    m_faces.append(4); m_faces.append(9); m_faces.append(0);
    m_faces.append(4); m_faces.append(5); m_faces.append(9);
    m_faces.append(8); m_faces.append(5); m_faces.append(4);
    m_faces.append(1); m_faces.append(8); m_faces.append(4);
    m_faces.append(1); m_faces.append(10); m_faces.append(8);
    m_faces.append(10); m_faces.append(3); m_faces.append(8);
    m_faces.append(8); m_faces.append(3); m_faces.append(5);
    m_faces.append(3); m_faces.append(2); m_faces.append(5);
    m_faces.append(3); m_faces.append(7); m_faces.append(2);
    m_faces.append(3); m_faces.append(10); m_faces.append(7);
    m_faces.append(10); m_faces.append(6); m_faces.append(7);
    m_faces.append(6); m_faces.append(11); m_faces.append(7);
    m_faces.append(6); m_faces.append(0); m_faces.append(11);
    m_faces.append(6); m_faces.append(1); m_faces.append(0);
    m_faces.append(10); m_faces.append(1); m_faces.append(6);
    m_faces.append(11); m_faces.append(0); m_faces.append(9);
    m_faces.append(2); m_faces.append(11); m_faces.append(9);
    m_faces.append(5); m_faces.append(2); m_faces.append(9);
    m_faces.append(11); m_faces.append(2); m_faces.append(7);
}

inline bool equal(const Vector3 &v1, const Vector3 &v2, float error)
{
    Vector3 v = v1 - v2;

    return (fabs(v.x) <= error && fabs(v.y) <= error && fabs(v.z) <= error);
}

bool SphereUniformPointCloudGenerator::findSphereCloudVertex(const Vector3 &v, unsigned short &position) const
{
    unsigned short i = 0;
    bool trobat = false;

    while (i < m_vertices.size() && !trobat)
    {
        if (equal(v, m_vertices[i], 0.0001))
        {
            trobat = true;
        }
        else
        {
            i++;
        }
    }

    position = i;

    return trobat;
}

void SphereUniformPointCloudGenerator::createSphereCloudTriangle(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3)
{
    unsigned short pos;

    // Afegim el triangle v1,v2,v3 a la llista --> Afegir vèrtexs (si calen) i cares!!!

    if (findSphereCloudVertex(v1, pos))
    {
        // Hem trobat el vèrtex a la posició pos --> Només afegim l'índex de la cara a un vèrtex existent
        m_faces.append(pos);
    }
    else
    {
        // Afegim el vèrtex i l'índex de la cara
        m_vertices.append(v1);
        m_normals.append(v1);
        m_faces.append(m_vertices.size() - 1);
    }

    if (findSphereCloudVertex(v2, pos))
    {
        // Hem trobat el vèrtex a la posició pos --> Només afegim l'índex de la cara a un vèrtex existent
        m_faces.append(pos);
    }
    else
    {
        // Afegim el vèrtex i l'índex de la cara
        m_vertices.append(v2);
        m_normals.append(v2);
        m_faces.append(m_vertices.size() - 1);
    }

    if (findSphereCloudVertex(v3, pos))
    {
        // Hem trobat el vèrtex a la posició pos --> Només afegim l'índex de la cara a un vèrtex existent
        m_faces.append(pos);
    }
    else
    {
        // Afegim el vèrtex i l'índex de la cara
        m_vertices.append(v3);
        m_normals.append(v3);
        m_faces.append(m_vertices.size() - 1);
    }
}

/**
 * \todo En teoria es podrien passar els vectors com a referències constants,
 * però llavors a vegades es modifiquen màgicament. (!!!!!!!!!!!!!!!!!!!!!!!)
 */
void SphereUniformPointCloudGenerator::subdivide(Vector3 v1, Vector3 v2, Vector3 v3, unsigned short depth)
{
    if (depth == 0)
    {
        createSphereCloudTriangle(v1, v2, v3);
    }
    else
    {
        Vector3 v12, v23, v31;

        // Trobem el punt mig de cada costat del triangle
        v12 = v1 + v2;
        v23 = v2 + v3;
        v31 = v3 + v1;

        // No cal dividir per dos els punts mitjos perquè normalitzem
        v12.normalize();
        v23.normalize();
        v31.normalize();

        subdivide(v1, v12, v31, depth - 1);
        subdivide(v2, v23, v12, depth - 1);
        subdivide(v3, v31, v23, depth - 1);
        subdivide(v12, v23, v31, depth - 1);
    }
}

void SphereUniformPointCloudGenerator::createGeographicVertices()
{
    m_geographicVertices.clear();
    m_geographicVertices.resize(m_vertices.size());

    for (unsigned short i = 0; i < m_vertices.size(); i++)
    {
        const Vector3 &v = m_vertices[i];
        Vector3 gv;
        gv.x = v.length();
        // Latitud
        gv.y = 90.0 - acos(v.z / gv.x) * 180.0 / MathTools::PiNumber;
        // Longitud
        if (v.x == 0.0 && v.y == 0.0)
        {
            gv.z = 0.0;
        }
        else
        {
            gv.z = acos(v.x / sqrt(v.x * v.x + v.y * v.y)) * 180.0 / MathTools::PiNumber;
            if (v.y < 0.0)
            {
                gv.z = -gv.z;
            }
        }
        m_geographicVertices[i] = gv;
    }
}

}
