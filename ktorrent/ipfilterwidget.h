/*
    SPDX-FileCopyrightText: 2007 Ivan Vasić <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IPFILTERWIDGET_H
#define IPFILTERWIDGET_H

#include "ui_ipfilterwidget.h"
#include <QDialog>

namespace kt
{
class IPFilterList;

/**
 * @author Ivan Vasic <ivasic@gmail.com>
 * @brief Integrated IPFilter GUI class.
 * Used to show, add and remove banned peers from blacklist.
 */
class IPFilterWidget : public QDialog, public Ui_IPFilterWidget
{
    Q_OBJECT
public:
    IPFilterWidget(QWidget *parent);
    ~IPFilterWidget() override;

    /// Register the filter list
    static void registerFilterList();

    void saveFilter(const QString &fn);
    static void loadFilter(const QString &fn);

public Q_SLOTS:
    void save();
    void open() override;
    void clear();
    void remove();
    void add();
    void accept() override;

private:
    void setupConnections();

    static IPFilterList *filter_list;
};
}

#endif
