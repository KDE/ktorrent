/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef KT_STRINGCOMPLETIONMODEL_H
#define KT_STRINGCOMPLETIONMODEL_H

#include <QStringListModel>
#include <ktcore_export.h>


namespace kt
{
    /**
        Model for a QCompleter which works with a list of unique strings loaded from a file.
    */
    class KTCORE_EXPORT StringCompletionModel : public QStringListModel
    {
        Q_OBJECT
    public:
        StringCompletionModel(const QString& file, QObject* parent);
        ~StringCompletionModel();

        /**
            Load the list of strings.
        */
        void load();

        /**
            Save the list of strings to the file
        */
        void save();

        /**
            Add a string to the list, automatically saves it.
        */
        void addString(const QString& s);
    private:
        QString file;
    };

}

#endif // KT_STRINGCOMPLETIONMODEL_H
