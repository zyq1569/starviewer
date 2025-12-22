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
    // First of all look if the position where we want to put the value is in the matrix
    // If it is not there we expand the matrix
    int listSelected = row + m_indexRow;
    // If the row goes below the current range, add a new QList to the beginning of the list
    if (listSelected < 0)
    {
        //Fill in the new list of -1
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
    /// If the row is passed the current range above,
    /// add a new QList to the end of the list
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
    // If the column is skipped to the left, add as many -1 as needed at the beginning of each
    // of the lists
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
    /// If the column goes to the right, add as many -1 as needed at the end of each of the lists
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

    // Now the array is large enough that if we do the shortcut it does not fall out
    m_matrix[listSelected][columnSelected] = value;
}

int DynamicMatrix::getValue(int row, int column) const
{
    // Calculate the actual row and column that we want to access
    int rowSelected = m_indexRow + row;
    int columnSelected = m_indexColumn + column;
    // Since the matrix is square, look to see if the box we want is within the limits of the matrix
    if (rowSelected < 0 || rowSelected > m_matrix.count() || columnSelected < 0 || columnSelected > m_columnCount)
    {
        //If it does not find the value it returns -1
        return -1;
    }
    else
    {
        return m_matrix[rowSelected][columnSelected];
    }
}

QList<int> DynamicMatrix::getLeftColumn() const
{
    //Returns a list with the first value in each row
    QList<int> result;
    for (int i = m_matrix.count() - 1; i >= 0; i--)
    {
        result.append(m_matrix[i][0]);
    }
    return result;
}

QList<int> DynamicMatrix::getRightColumn() const
{
    //Returns a list with the last value in each row
    QList<int> result;
    for (int i = m_matrix.count() - 1; i >= 0; i--)
    {
        result.append(m_matrix[i][m_columnCount - 1]);
    }
    return result;
}

QList<int> DynamicMatrix::getTopRow() const
{
    // Returns a list with the top row (which is the last QList)
    return m_matrix[m_matrix.count() - 1];
}

QList<int> DynamicMatrix::getBottomRow() const
{
    //Returns a list with the bottom row (which is the first QList)
    return m_matrix[0];
}

void DynamicMatrix::print()
{
    // Example result:
    // ######
    // 0 1 2
    // 5 6
    // ######
    // Write a # upper line
    QString result("");
    for (int j = 0; j < m_columnCount; j++)
    {
        result = result + QString("##");
    }
    DEBUG_LOG(result);
    // Write the matrix
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
    //Write a # bottom line
    result = QString("");
    for (int j = 0; j < m_columnCount; j++)
    {
        result = result + QString("##");
    }
    DEBUG_LOG(result);
}

bool DynamicMatrix::isMaximizable() const
{
    // If there is more than one column or more than one row
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
