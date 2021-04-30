/*
    SPDX-FileCopyrightText: 2019 Gilbert Assaf <gassaf@gmx.de>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MAGNETURLSCHEMEHANDLER_H
#define MAGNETURLSCHEMEHANDLER_H

#include <QWebEngineUrlSchemeHandler>

class MagnetUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT

public:
    explicit MagnetUrlSchemeHandler(QObject *parent = nullptr);
    void requestStarted(QWebEngineUrlRequestJob *request) override;

Q_SIGNALS:
    void magnetUrlDetected(const QUrl &url);
};

#endif // MAGNETURLSCHEMEHANDLER_H
