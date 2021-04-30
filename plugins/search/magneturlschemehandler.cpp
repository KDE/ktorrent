/*
    SPDX-FileCopyrightText: 2019 Gilbert Assaf <gassaf@gmx.de>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "magneturlschemehandler.h"
#include <QWebEngineUrlRequestJob>
#include <QWebEngineUrlSchemeHandler>

MagnetUrlSchemeHandler::MagnetUrlSchemeHandler(QObject *parent)
    : QWebEngineUrlSchemeHandler(parent)
{
}

void MagnetUrlSchemeHandler::requestStarted(QWebEngineUrlRequestJob *request)
{
    request->fail(QWebEngineUrlRequestJob::RequestAborted);
    const QUrl url = request->requestUrl();

    Q_EMIT magnetUrlDetected(url);
}
