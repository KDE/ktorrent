/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    StringCompletionModel(const QString &file, QObject *parent);
    ~StringCompletionModel() override;

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
    void addString(const QString &s);

private:
    QString file;
};

}

#endif // KT_STRINGCOMPLETIONMODEL_H
