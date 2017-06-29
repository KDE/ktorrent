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

#include "stringcompletionmodel.h"

#include <QFile>
#include <QSet>
#include <QTextStream>

#include <util/log.h>

using namespace bt;

namespace kt
{

    StringCompletionModel::StringCompletionModel(const QString& file, QObject* parent): QStringListModel(parent), file(file)
    {
    }


    StringCompletionModel::~StringCompletionModel()
    {
    }

    void StringCompletionModel::load()
    {
        QFile fptr(file);
        if (!fptr.open(QIODevice::ReadOnly))
        {
            Out(SYS_GEN | LOG_NOTICE) << "Failed to open " << file << " : " << fptr.errorString() << endl;
            return;
        }

        QSet<QString> strings;
        while (!fptr.atEnd())
        {
            QString line = QString::fromUtf8(fptr.readLine().trimmed());
            if (line.length() > 0)
                strings.insert(line);
        }

        setStringList(strings.toList());
    }

    void StringCompletionModel::save()
    {
        QFile fptr(file);
        if (!fptr.open(QIODevice::WriteOnly))
        {
            Out(SYS_GEN | LOG_NOTICE) << "Failed to open " << file << " : " << fptr.errorString() << endl;
            return;
        }

        QTextStream out(&fptr);
        const QStringList sl = stringList();
        for (const QString& s : sl)
            out << s << endl;
    }

    void StringCompletionModel::addString(const QString& s)
    {
        QStringList curr = stringList();
        if (!curr.contains(s))
        {
            curr.append(s);
            setStringList(curr);
        }
    }

}

