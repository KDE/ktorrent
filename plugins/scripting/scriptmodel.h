/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTSCRIPTMODEL_H
#define KTSCRIPTMODEL_H

#include <QAbstractListModel>

class KArchive;
class KArchiveDirectory;

namespace kt
{
class Script;

/**
    Model which keeps track of all scripts
*/
class ScriptModel : public QAbstractListModel
{
    Q_OBJECT
public:
    ScriptModel(QObject *parent);
    ~ScriptModel() override;

    enum Role {
        CommentRole = Qt::UserRole,
        ConfigurableRole,
        ConfigureRole,
        AboutRole,
    };

    /**
     * Add a script to the model
     * @param file
     */
    void addScript(const QString &file);

    /**
     * Add script which is described by a desktop file.
     * @param dir The directory the script is in
     * @param desktop_file The desktop file (relative to dir, not an absolute path)
     * @return The Script or 0 if something goes wrong
     */
    Script *addScriptFromDesktopFile(const QString &dir, const QString &desktop_file);

    /// Get a script given an index
    Script *scriptForIndex(const QModelIndex &index) const;

    /// Get a list of all scripts
    QStringList scriptFiles() const;

    /// Get a list of all running scripts
    QStringList runningScriptFiles() const;

    /// Remove a bunch of scripts
    void removeScripts(const QModelIndexList &indices);

    /// Run all the scripts in the string list
    void runScripts(const QStringList &r);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;

private:
    void addScriptFromArchive(KArchive *archive);
    void addScriptFromArchiveDirectory(const KArchiveDirectory *dir);

Q_SIGNALS:
    void showPropertiesDialog(Script *s);

private:
    QList<Script *> scripts;
};

}

#endif
