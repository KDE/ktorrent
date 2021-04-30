/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Will Stephenson <wstephenson@kde.org>
    SPDX-License-Identifier: LGPL-2.0-only WITH Qt-Commercial-exception-1.0
*/

#ifndef STATUSBAROFFLINEINDICATOR_H
#define STATUSBAROFFLINEINDICATOR_H

#include <QWidget>

class StatusBarOfflineIndicatorPrivate;

class StatusBarOfflineIndicator : public QWidget
{
    Q_OBJECT
public:
    /**
     * Default constructor.
     * @param parent the widget's parent
     */
    explicit StatusBarOfflineIndicator(QWidget *parent);
    ~StatusBarOfflineIndicator();

private:
    StatusBarOfflineIndicatorPrivate *const d;
};

#endif
