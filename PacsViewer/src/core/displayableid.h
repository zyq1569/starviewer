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

#ifndef UDGDISPLAYABLE_H
#define UDGDISPLAYABLE_H

#include <QList>
#include <QString>

namespace udg {

/**
Identifier used in places where an identifier is needed that the user can see on the screen. This one has a fixed, invariable part, which is the
     which is actually used as an identifier, internally. It also has a part that serves to be able to show it in the interface and that can be translated.
     For example, the names of the Extensions.

     It has no "setters" expressly.
     Example of assignments and creation of null id's:
     @code
     DisplayableID id (QString :: null);

     DisplayableID id2 ("test", tr ("test"));

     id = DisplayableID ("newValue", tr ("newValue"));

     id = id2;
     @endcode

  */
class DisplayableID {

public:
    DisplayableID(const QString &id, const QString &name = QString::null);

    ~DisplayableID();

    QString getID() const
    {
        return m_id;
    }
    QString getLabel() const
    {
        return m_name;
    }

    friend inline bool operator==(const DisplayableID &, const DisplayableID &);
    friend inline bool operator!=(const DisplayableID &, const DisplayableID &);
    friend inline bool operator<(const DisplayableID &, const DisplayableID &);
    friend inline bool operator>(const DisplayableID &, const DisplayableID &);

private:
    QString m_id;
    QString m_name;

};

typedef QList<DisplayableID> DisplayableIDList;

}

#endif
