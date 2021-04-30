/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QFile>
#include <QIcon>

#include "filter.h"
#include "filterlist.h"
#include <bcodec/bdecoder.h>
#include <bcodec/bencoder.h>
#include <bcodec/bnode.h>
#include <util/error.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
FilterList::FilterList(QObject *parent)
    : FilterListModel(parent)
{
}

FilterList::~FilterList()
{
    qDeleteAll(filters);
}

void FilterList::filterEdited(Filter *f)
{
    int idx = filters.indexOf(f);
    if (idx < 0)
        return;

    Q_EMIT dataChanged(index(idx, 0), index(idx, 0));
}

void FilterList::saveFilters(const QString &file)
{
    File fptr;
    if (!fptr.open(file, QStringLiteral("wt"))) {
        Out(SYS_SYN | LOG_DEBUG) << "Failed to open " << file << " : " << fptr.errorString() << endl;
        return;
    }

    BEncoder enc(&fptr);
    enc.beginList();
    for (Filter *f : qAsConst(filters))
        f->save(enc);
    enc.end();
}

void FilterList::loadFilters(const QString &file)
{
    QFile fptr(file);
    if (!fptr.open(QIODevice::ReadOnly)) {
        Out(SYS_SYN | LOG_DEBUG) << "Failed to open " << file << " : " << fptr.errorString() << endl;
        return;
    }

    QByteArray data = fptr.readAll();
    BDecoder dec(data, false);
    BNode *n = 0;
    try {
        n = dec.decode();
        if (!n || n->getType() != BNode::LIST) {
            delete n;
            return;
        }

        BListNode *ln = (BListNode *)n;
        for (Uint32 i = 0; i < ln->getNumChildren(); i++) {
            BDictNode *dict = ln->getDict(i);
            if (dict) {
                Filter *filter = new Filter();
                if (filter->load(dict))
                    addFilter(filter);
                else
                    delete filter;
            }
        }
    } catch (bt::Error &err) {
        Out(SYS_SYN | LOG_DEBUG) << "Failed to parse " << file << " : " << err.toString() << endl;
    }

    delete n;
}
}
