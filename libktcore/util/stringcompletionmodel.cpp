/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "stringcompletionmodel.h"

#include <QFile>
#include <QSet>
#include <QTextStream>

#include <util/log.h>

using namespace bt;

namespace kt
{
StringCompletionModel::StringCompletionModel(const QString &file, QObject *parent)
    : QStringListModel(parent)
    , file(file)
{
}

StringCompletionModel::~StringCompletionModel()
{
}

void StringCompletionModel::load()
{
    QFile fptr(file);
    if (!fptr.open(QIODevice::ReadOnly)) {
        Out(SYS_GEN | LOG_NOTICE) << "Failed to open " << file << " : " << fptr.errorString() << endl;
        return;
    }

    QSet<QString> strings;
    while (!fptr.atEnd()) {
        QString line = QString::fromUtf8(fptr.readLine().trimmed());
        if (line.length() > 0)
            strings.insert(line);
    }

    setStringList(QList<QString>(strings.begin(), strings.end()));
}

void StringCompletionModel::save()
{
    QFile fptr(file);
    if (!fptr.open(QIODevice::WriteOnly)) {
        Out(SYS_GEN | LOG_NOTICE) << "Failed to open " << file << " : " << fptr.errorString() << endl;
        return;
    }

    QTextStream out(&fptr);
    const QStringList sl = stringList();
    for (const QString &s : sl)
        out << s << Qt::endl;
}

void StringCompletionModel::addString(const QString &s)
{
    QStringList curr = stringList();
    if (!curr.contains(s)) {
        curr.append(s);
        setStringList(curr);
    }
}

}
