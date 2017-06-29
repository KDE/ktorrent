/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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

#include <QFile>
#include <QIcon>

#include <util/log.h>
#include <util/error.h>
#include <bcodec/bnode.h>
#include <bcodec/bencoder.h>
#include <bcodec/bdecoder.h>
#include "filterlist.h"
#include "filter.h"

using namespace bt;

namespace kt
{

    FilterList::FilterList(QObject* parent)
        : FilterListModel(parent)
    {
    }


    FilterList::~FilterList()
    {
        qDeleteAll(filters);
    }

    void FilterList::filterEdited(Filter* f)
    {
        int idx = filters.indexOf(f);
        if (idx < 0)
            return;

        emit dataChanged(index(idx, 0), index(idx, 0));
    }

    void FilterList::saveFilters(const QString& file)
    {
        File fptr;
        if (!fptr.open(file, QStringLiteral("wt")))
        {
            Out(SYS_SYN | LOG_DEBUG) << "Failed to open " << file << " : " << fptr.errorString() << endl;
            return;
        }

        BEncoder enc(&fptr);
        enc.beginList();
        foreach (Filter* f, filters)
            f->save(enc);
        enc.end();
    }

    void FilterList::loadFilters(const QString& file)
    {
        QFile fptr(file);
        if (!fptr.open(QIODevice::ReadOnly))
        {
            Out(SYS_SYN | LOG_DEBUG) << "Failed to open " << file << " : " << fptr.errorString() << endl;
            return;
        }

        QByteArray data = fptr.readAll();
        BDecoder dec(data, false);
        BNode* n = 0;
        try
        {
            n = dec.decode();
            if (!n || n->getType() != BNode::LIST)
            {
                delete n;
                return;
            }

            BListNode* ln = (BListNode*)n;
            for (Uint32 i = 0; i < ln->getNumChildren(); i++)
            {
                BDictNode* dict = ln->getDict(i);
                if (dict)
                {
                    Filter* filter = new Filter();
                    if (filter->load(dict))
                        addFilter(filter);
                    else
                        delete filter;
                }
            }
        }
        catch (bt::Error& err)
        {
            Out(SYS_SYN | LOG_DEBUG) << "Failed to parse " << file << " : " << err.toString() << endl;
        }

        delete n;
    }
}
