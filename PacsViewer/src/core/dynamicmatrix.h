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

#ifndef UDGDYNAMICMATRIX_H
#define UDGDYNAMICMATRIX_H

#include <QList>

class QPoint;

namespace udg {

/**
This class represents a dynamic array of integers, which allows you to add and obtain values without considering
     move away from rank. It is made from a list of integer lists and two pointers, one for rows and one for
     columns, which indicate which row or column the 0 is in.

     Thus, if an item is added to the position (row -1, column 0) what will be done is to add a QList to the first position
     from the list, and increase the row pointer.

     To add an item to the position (row x, column -1), a -1 will be added to the beginning of each QList in the list and modified
     the pointer to the columns.
  */
class DynamicMatrix {
public:
    /// Default constructor..
    DynamicMatrix();
    /// Put a value in a row and column of the array, it will never go out of range..
    void setValue(int row, int column, int value);
    /// Returns the value of the array of the row and column position, if it does not exist returns a -1.
    int getValue(int row, int column) const;

    /// Debug the contents of the array.
    void print();

    ///  Returns a list with all the identifiers of the screens in the first column.
    QList<int> getLeftColumn() const;
    /// Returns a list of all screen identifiers in the last column.
    QList<int> getRightColumn() const;
    /// Returns a list of all identifiers on the top row screens.
    QList<int> getTopRow() const;
    /// Returns a list of all identifiers on the screens in the bottom row.
    QList<int> getBottomRow() const;

    /// Method that indicates whether the window can be maximized to more than one screen (called ScreenManager).
    bool isMaximizable() const;
    /// Returns the number of rows.
    int getNumberOfRows() const;
    /// Returns the number of columns in the longest row.
    int getNumberOfColumns() const;

    /// Returns the index of the first row.
    int getRowBase() const;
    /// Returns the index of the first column.
    int getColumnBase() const;

    /// Searches the matrix for the given value.
    /// If found, it returns true and fills the row and column parameters with the row and column indices of the first instance of the value.
    /// If the value is not found, it returns false.
    bool getPosition(int value, int &row, int &column) const;

private:
    /// The list of lists that stores the structure of the array.
    QList<QList<int> > m_matrix;
    /// Indicates where the position of row 0 is in the list of lists.
    int m_indexRow;
    /// Indicates where the position of column 0 is in the list of lists.
    int m_indexColumn;
    /// Number of columns with the longest row.
    int m_columnCount;
};

} // End namespace udg

#endif // UDGDYNAMICMATRIX_H
