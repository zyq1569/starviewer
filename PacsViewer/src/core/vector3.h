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

#ifndef VECTOR3_H
#define VECTOR3_H

#include <cmath>

#include <QDataStream>
#include <QString>

namespace udg {

/**
Dimension vector 3.
     It includes all methods for working with 3D vectors with real components
  */
template <class T>
class TVector3 {

public:
    /// Returns the scalar product of the vectors.
    static T dot(const TVector3<T> &v1, const TVector3<T> &v2);
    /// Returns the vector product of the vectors.
    static TVector3<T> cross(const TVector3<T> &v1, const TVector3<T> &v2);

    TVector3(T x = 0.0, T y = 0.0, T z = 0.0);
    template <class C>
    TVector3(const TVector3<C> &v);

    /// Assign the components of the vector.
    void set(T x, T y, T z);

    /// Returns the vector module..
    T length() const;
    /// Returns the squared module of the vector.
    T lengthSquared() const;
    /// Normalize the vector and return it for reference.
    TVector3<T>& normalize();

    /// Assignment operator.
    template <class C>
    TVector3<T>& operator =(const TVector3<C> &v);

    /// Returns true if the vectors are equal.
    bool operator ==(const TVector3<T> &v) const;
    ///Returns true if the vectors are different.
    bool operator !=(const TVector3<T> &v) const;

    ///  Returns a copy of the vector.
    TVector3<T> operator +() const;
    /// Returns the denied vector.
    TVector3<T> operator -() const;
    ///Returns the sum of vectors.
    TVector3<T> operator +(const TVector3<T> &v) const;
    /// Add the vectors, assign the result over the current one and return it for reference..
    TVector3<T>& operator +=(const TVector3<T> &v);
    ///Returns the rest of the vectors..
    TVector3<T> operator -(const TVector3<T> &v) const;
    /// Subtract the vectors, assign the result over the current one and return it for reference
    TVector3<T>& operator -=(const TVector3<T> &v);
    /// Returns the scalar product of the vectors.

    // Producte escalar
    T operator *(const TVector3<T> &v) const;
    ///  Returns the vector product of the vectors.

    // Vector product
    TVector3<T> operator ^(const TVector3<T> &v) const;

    ///  Returns a representation of the vector in text form.
    QString toString() const;
    operator QString() const;

    /// Product of a scalar for a vector..
    template <class C>
    friend TVector3<C> operator *(double a, const TVector3<C> &v);
    /// Product of a vector by a scalar..
    template <class C>
    friend TVector3<C> operator *(const TVector3<C> &v, double a);
    /// Product of a vector by a scalar assigned to the vector itself..
    template <class C>
    friend TVector3<C>& operator *=(TVector3<C> &v, double a);
    /// Division of a vector by a scalar.
    template <class C>
    friend TVector3<C> operator /(const TVector3<C> &v, double a);
    /// Division of a vector by a scalar assigned to the vector itself..
    template <class C>
    friend TVector3<C>& operator /=(TVector3<C> &v, double a);

    /// Reading from a QDataStream..
    template <class C>
    friend QDataStream& operator >>(QDataStream &in, TVector3<C> &v);
    ///Writing to a QDataStream.
    template <class C>
    friend QDataStream& operator <<(QDataStream &out, const TVector3<C> &v);

public:
    /// Components del vector.
    T x, y, z;

};

template <class T>
inline T TVector3<T>::dot(const TVector3<T> &v1, const TVector3<T> &v2)
{
    return v1 * v2;
}

template <class T>
inline TVector3<T> TVector3<T>::cross(const TVector3<T> &v1, const TVector3<T> &v2)
{
    return v1 ^ v2;
}

template <class T>
inline TVector3<T>::TVector3(T aX, T aY, T aZ)
    : x(aX), y(aY), z(aZ)
{
}

template <class T>
template <class C>
inline TVector3<T>::TVector3(const TVector3<C> &v)
    : x(v.x), y(v.y), z(v.z)
{
}

template <class T>
inline void TVector3<T>::set(T x, T y, T z)
{
    this->x = x; this->y = y; this->z = z;
}

template <class T>
inline T TVector3<T>::length() const
{
    return sqrt(x * x + y * y + z * z);
}

template <class T>
inline T TVector3<T>::lengthSquared() const
{
    return x * x + y * y + z * z;
}

template <class T>
inline TVector3<T>& TVector3<T>::normalize()
{
    T length = this->length();
    x /= length; y /= length; z /= length;
    return *this;
}

template <class T>
template <class C>
inline TVector3<T>& TVector3<T>::operator =(const TVector3<C> &v)
{
    x = v.x; y = v.y; z = v.z;
    return *this;
}

template <class T>
inline bool TVector3<T>::operator ==(const TVector3<T> &v) const
{
    return x == v.x && y == v.y && z == v.z;
}

template <class T>
inline bool TVector3<T>::operator !=(const TVector3<T> &v) const
{
    return x != v.x || y != v.y || z != v.z;
}

template <class T>
inline TVector3<T> TVector3<T>::operator +() const
{
    return *this;
}

template <class T>
inline TVector3<T> TVector3<T>::operator -() const
{
    return TVector3<T>(-x, -y, -z);
}

template <class T>
inline TVector3<T> TVector3<T>::operator +(const TVector3<T> &v) const
{
    return TVector3<T>(x + v.x, y + v.y, z + v.z);
}

template <class T>
inline TVector3<T>& TVector3<T>::operator +=(const TVector3<T> &v)
{
    x += v.x; y += v.y; z += v.z;
    return *this;
}

template <class T>
inline TVector3<T> TVector3<T>::operator -(const TVector3<T> &v) const
{
    return TVector3<T>(x - v.x, y - v.y, z - v.z);
}

template <class T>
inline TVector3<T>& TVector3<T>::operator -=(const TVector3<T> &v)
{
    x -= v.x; y -= v.y; z -= v.z;
    return *this;
}

// Producte escalar
template <class T>
inline T TVector3<T>::operator *(const TVector3<T> &v) const
{
    return x * v.x + y * v.y + z * v.z;
}

// Producte vectorial
template <class T>
inline TVector3<T> TVector3<T>::operator ^(const TVector3<T> &v) const
{
    return TVector3<T>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}

template <class T>
inline QString TVector3<T>::toString() const
{
    return QString("(%1, %2, %3)").arg(x).arg(y).arg(z);
}

template <class T>
inline TVector3<T>::operator QString() const
{
    return toString();
}

template <class T>
inline TVector3<T> operator *(double a, const TVector3<T> &v)
{
    return TVector3<T>(a * v.x, a * v.y, a * v.z);
}

template <class T>
inline TVector3<T> operator *(const TVector3<T> &v, double a)
{
    return TVector3<T>(v.x * a, v.y * a, v.z * a);
}

template <class T>
inline TVector3<T>& operator *=(TVector3<T> &v, double a)
{
    v.x *= a; v.y *= a; v.z *= a;
    return v;
}

template <class T>
inline TVector3<T> operator /(const TVector3<T> &v, double a)
{
    return TVector3<T>(v.x / a, v.y / a, v.z / a);
}

template <class T>
inline TVector3<T>& operator /=(TVector3<T> &v, double a)
{
    v.x /= a; v.y /= a; v.z /= a;
    return v;
}

template <class T>
inline QDataStream& operator >>(QDataStream &in, TVector3<T> &v)
{
    in >> v.x >> v.y >> v.z;
    return in;
}

template <class T>
inline QDataStream& operator <<(QDataStream &out, const TVector3<T> &v)
{
    out << v.x << v.y << v.z;
    return out;
}

typedef TVector3<float> Vector3Float;
typedef TVector3<double> Vector3Double;
typedef TVector3<double> Vector3;

}

#endif
