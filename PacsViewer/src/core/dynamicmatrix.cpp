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

#include "dynamicmatrix.h"
#include "logging.h"

#include <QList>
#include <QPoint>
#include <QString>

namespace udg {

DynamicMatrix::DynamicMatrix()
{
    m_indexRow = 0;
    m_indexColumn = 0;
    m_columnCount = 0;
}

void DynamicMatrix::setValue(int row, int column, int value)
{
    // Primer de tot mirar si la posició on volem posar el valor està a la matriu
    // Si no hi és expandim la matriu
    int listSelected = row + m_indexRow;
    // Si la fila es passa del rang actual per baix, afegir una nova QList a l'inici de la llista
    if (listSelected < 0)
    {
        // Emplenar la nova llista de -1
        for (int i = m_indexRow; i < -1 * row; i++)
        {
            QList<int> list;
            for (int j = 0; j < m_columnCount; j++)
            {
                list.append(-1);
            }
            m_matrix.insert(m_matrix.begin(), list);
            m_indexRow++;
        }
        listSelected = row + m_indexRow;
    }
    // Si la fila es passa del rang actual per dalt, afegir una nova QLista al final de la llista
    else if (listSelected >= m_matrix.count())
    {
        for (int i = m_matrix.count() - 1; i < listSelected; i++)
        {
            QList<int> list;
            for (int j = 0; j < m_columnCount; j++)
            {
                list.append(-1);
            }
            m_matrix.append(list);
        }
    }
    // Si la columna es passa de rang per l'esquerra, afegir tants -1 com calgui a l'inici de cada una
    // de les llistes
    int columnSelected = column + m_indexColumn;
    if (columnSelected < 0)
    {
        for (int i = m_indexColumn; i < -1 * column; i++)
        {
            for (int j = 0; j < m_matrix.count(); j++)
            {
                m_matrix[j].insert(m_matrix[j].begin(), -1);
            }
            m_indexColumn++;
            m_columnCount++;
        }
        columnSelected = column + m_indexColumn;
    }
    // Si la columna es passa per la dreta, afegir tants -1 com calgui al final de cada una de les llistes
    else if (columnSelected >= m_columnCount)
    {
        for (int i = m_columnCount - 1; i < columnSelected; i++)
        {
            for (int j = 0; j < m_matrix.count(); j++)
            {
                m_matrix[j].append(-1);
            }
            m_columnCount++;
        }
    }

    // Ara la matriu ja es prou gran com per que si fem l'acces directe no caigui fora
    m_matrix[listSelected][columnSelected] = value;
}

int DynamicMatrix::getValue(int row, int column) const
{
    // Calcular la fila i columna real a la que volem accedir
    int rowSelected = m_indexRow + row;
    int columnSelected = m_indexColumn + column;
    // Com que la matriu és quadrada, mirar si la casella que volem està dins els limits de la matriu
    if (rowSelected < 0 || rowSelected > m_matrix.count() || columnSelected < 0 || columnSelected > m_columnCount)
    {
        // Si no troba el valor retorna -1
        return -1;
    }
    else
    {
        return m_matrix[rowSelected][columnSelected];
    }
}

QList<int> DynamicMatrix::getLeftColumn() const
{
    // Retorna una llista amb el primer valor de cada fila
    QList<int> result;
    for (int i = m_matrix.count() - 1; i >= 0; i--)
    {
        result.append(m_matrix[i][0]);
    }
    return result;
}

QList<int> DynamicMatrix::getRightColumn() const
{
    // Retorna una llista amb l'últim valor de cada fila
    QList<int> result;
    for (int i = m_matrix.count() - 1; i >= 0; i--)
    {
        result.append(m_matrix[i][m_columnCount - 1]);
    }
    return result;
}

QList<int> DynamicMatrix::getTopRow() const
{
    // Retorna una llista amb la fila superior (que és l'última QList)
    return m_matrix[m_matrix.count() - 1];
}

QList<int> DynamicMatrix::getBottomRow() const
{
    // Retorna una llista amb la fila inferior (que és la primera QList)
    return m_matrix[0];
}

void DynamicMatrix::print()
{
    // Exemple de resultat:
    // ######
    //  0 1 2
    //  5   6
    // ######
    // Escriure una linia de # superior
    QString result("");
    for (int j = 0; j < m_columnCount; j++)
    {
        result = result + QString("##");
    }
    DEBUG_LOG(result);
    // Escriure la matriu
    for (int i = m_matrix.count() - 1; i >= 0; i--)
    {
        result = QString("");
        for (int j = 0; j < m_columnCount; j++)
        {
            if (m_matrix[i][j] != -1)
            {
                result = result + QString(" ") + QString::number(m_matrix[i][j]);
            }
            else
            {
                result = result + QString("  ");
            }
        }
        DEBUG_LOG(result);
    }
    // Escriure una linia de # inferior
    result = QString("");
    for (int j = 0; j < m_columnCount; j++)
    {
        result = result + QString("##");
    }
    DEBUG_LOG(result);
}

bool DynamicMatrix::isMaximizable() const
{
    // Si hi ha més d'una columna o més d'una fila
    return m_columnCount > 1 || m_matrix.count() > 1;
}

int DynamicMatrix::getNumberOfRows() const
{
    return m_matrix.count();
}

int DynamicMatrix::getNumberOfColumns() const
{
    return m_columnCount;
}

int DynamicMatrix::getRowBase() const
{
    return -m_indexRow;
}

int DynamicMatrix::getColumnBase() const
{
    return -m_indexColumn;
}

bool DynamicMatrix::getPosition(int value, int &row, int &column) const
{
    for (int r = 0; r < m_matrix.count(); r++)
    {
        int c = m_matrix[r].indexOf(value);

        if (c != -1)
        {
            row = r - m_indexRow;
            column = c - m_indexColumn;
            return true;
        }
    }

    return false;
}

} // End namespace udg
