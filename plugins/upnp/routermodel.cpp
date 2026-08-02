/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KLocalizedString>
#include <QIcon>

#include "routermodel.h"
#include <upnp/upnprouter.h>
#include <util/error.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
RouterModel::RouterModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

RouterModel::~RouterModel()
{
}

void RouterModel::addRouter(bt::UPnPRouter *r)
{
    routers.append(r);
    insertRow(routers.count() - 1);
}

int RouterModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return routers.count();
    } else {
        return 0;
    }
}

int RouterModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return NUM_COLUMNS;
    } else {
        return 0;
    }
}

QVariant RouterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return QVariant();
    }

    switch (Column{section}) {
    case Column::DEVICE:
        return i18n("Device");
    case Column::PORTS_FORWARDED:
        return i18n("Ports Forwarded");
    default:
        return QVariant();
    }
}

bt::UPnPRouter *RouterModel::routerForIndex(const QModelIndex &index)
{
    if (!index.isValid()) {
        return nullptr;
    } else {
        return routers.at(index.row());
    }
}

QVariant RouterModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const Column column{index.column()};
    const bt::UPnPRouter *r = routers.at(index.row());
    if (role == Qt::DisplayRole) {
        switch (column) {
        case Column::DEVICE:
            return r->getDescription().friendlyName;
        case Column::PORTS_FORWARDED:
            if (!r->getError().isEmpty()) {
                return r->getError();
            } else {
                return ports(r);
            }
        default:
            break;
        }
    } else if (role == Qt::DecorationRole) {
        if (column == Column::DEVICE) {
            return QIcon::fromTheme(QStringLiteral("modem"));
        } else if (column == Column::PORTS_FORWARDED && !r->getError().isEmpty()) {
            return QIcon::fromTheme(QStringLiteral("dialog-error"));
        }
    } else if (role == Qt::ToolTipRole) {
        if (column == Column::DEVICE) {
            const bt::UPnPDeviceDescription &d = r->getDescription();
            return i18n(
                "Model Name: <b>%1</b><br/>"
                "Manufacturer: <b>%2</b><br/>"
                "Model Description: <b>%3</b><br/>",
                d.modelName,
                d.manufacturer,
                d.modelDescription);
        } else if (column == Column::PORTS_FORWARDED && !r->getError().isEmpty()) {
            return r->getError();
        }
    }

    return QVariant();
}

bool RouterModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    endRemoveRows();
    return true;
}

bool RouterModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginInsertRows(QModelIndex(), row, row + count - 1);
    endInsertRows();
    return true;
}

class PortsVisitor : public bt::UPnPRouter::Visitor
{
public:
    ~PortsVisitor() override
    {
    }

    void forwarding(const net::Port &port, bool pending, const bt::UPnPService *service) override
    {
        Q_UNUSED(service);
        if (!pending) {
            QString ret = QString::number(port.number) + QStringLiteral(" (");
            QString prot = (port.proto == net::UDP ? QStringLiteral("UDP") : QStringLiteral("TCP"));
            ret += prot + QStringLiteral(")");
            ports.append(ret);
        }
    }

    QString result()
    {
        return ports.join(QStringLiteral(", "));
    }

    QStringList ports;
};

QString RouterModel::ports(const bt::UPnPRouter *r) const
{
    PortsVisitor pv;
    r->visit(&pv);
    return pv.result();
}

void RouterModel::update()
{
    Q_EMIT dataChanged(index(0, 0), index(rowCount(QModelIndex()) - 1, columnCount(QModelIndex()) - 1));
}

void RouterModel::forward(const net::Port &port)
{
    try {
        for (bt::UPnPRouter *r : std::as_const(routers)) {
            r->forward(port);
        }
    } catch (bt::Error &e) {
        Out(SYS_PNP | LOG_DEBUG) << "Error : " << e.toString() << endl;
    }
}

void RouterModel::undoForward(const net::Port &port, bt::WaitJob *wjob)
{
    try {
        for (bt::UPnPRouter *r : std::as_const(routers)) {
            r->undoForward(port, wjob);
        }
    } catch (Error &e) {
        Out(SYS_PNP | LOG_DEBUG) << "Error : " << e.toString() << endl;
    }
}
}

#include "moc_routermodel.cpp"
