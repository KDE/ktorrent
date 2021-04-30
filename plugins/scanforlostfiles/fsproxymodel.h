/*
    SPDX-FileCopyrightText: 2020 Alexander Trufanov <trufanovan@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FSPROXYMODEL_H
#define FSPROXYMODEL_H
#include <QFileSystemModel>
#include <QSet>
#include <QSortFilterProxyModel>

namespace kt
{
/**
 * A simple proxy model that may filter out all data
 * that isn't presented in its filter
 */
class FSProxyModel : public QSortFilterProxyModel
{
public:
    FSProxyModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
        , m_filter(nullptr)
        , m_filtered(true)
    {
    }

    ~FSProxyModel()
    {
        if (m_filter)
            delete m_filter;
    }

    /**
     * @return a pointer to the filter set.
     */
    const QSet<QString> *filter() const
    {
        return m_filter;
    }

    /**
     * Sets a new filter. The previous one is destroyed.
     * @param filter   A pointer to the new filter.
     */
    void setFilter(QSet<QString> *filter)
    {
        if (m_filter && m_filter != filter)
            delete m_filter;
        m_filter = filter;
    }

    /**
     * @return true if filter is applied.
     */
    bool filtered() const
    {
        return m_filtered;
    }

    /**
     * Disable or enable filtering (if any filter is set).
     * @param val    Indicates if filtering is turned on or off.
     */
    void setFiltered(bool val)
    {
        m_filtered = val;
    }

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override
    {
        if (m_filter && m_filtered) {
            QFileSystemModel *m = reinterpret_cast<QFileSystemModel *>(sourceModel());
            QModelIndex i = m->index(source_row, 0, source_parent);
            QString fpath = m->filePath(i);
            return m_filter->contains(fpath);
        } else {
            return !m_filtered;
        }
    }

private:
    QSet<QString> *m_filter;
    bool m_filtered;
};

}

#endif // FSPROXYMODEL_H
