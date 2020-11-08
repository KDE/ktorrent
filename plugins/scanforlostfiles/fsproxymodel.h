/***************************************************************************
 *   Copyright (C) 2020 by Alexander Trufanov                              *
 *   trufanovan@gmail.com                                                  *
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

#ifndef FSPROXYMODEL_H
#define FSPROXYMODEL_H
#include <QSortFilterProxyModel>
#include <QFileSystemModel>
#include <QSet>

namespace kt
{

/**
 * A simple proxy model that may filter out all data
 * that isn't presented in its filter
 */
class FSProxyModel: public QSortFilterProxyModel
{
public:
    FSProxyModel(QObject *parent = nullptr): QSortFilterProxyModel(parent), m_filter(nullptr), m_filtered(true) {}

    ~FSProxyModel()
    {
        if (m_filter) delete m_filter;
    }

    /**
     * @return a pointer to the filter set.
     */
    const QSet<QString>* filter() const
    {
        return m_filter;
    }

    /**
     * Sets a new filter. The previous one is destroyed.
     * @param filter   A pointer to the new filter.
     */
    void setFilter(QSet<QString>* filter)
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
            QFileSystemModel* m = reinterpret_cast<QFileSystemModel*>(sourceModel());
            QModelIndex i = m->index(source_row, 0, source_parent);
            QString fpath = m->filePath(i);
            return m_filter->contains(fpath);
        } else {
            return !m_filtered;
        }
    }
private:
    QSet<QString>* m_filter;
    bool m_filtered;
};


}

#endif // FSPROXYMODEL_H
