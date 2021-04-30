/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QFileInfo>
#include <QStringRef>
#include <QXmlStreamReader>

#include <KIO/Job>

#include "searchengine.h"
#include <util/fileops.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
class OpenSearchHandler
{
public:
    OpenSearchHandler(SearchEngine *engine)
        : engine(engine)
    {
    }

    ~OpenSearchHandler()
    {
    }

    bool characters(const QStringRef &ch)
    {
        tmp += ch;
        return true;
    }

    bool startElement(const QStringRef &namespaceURI, const QStringRef &localName, const QStringRef &qName, const QXmlStreamAttributes &atts)
    {
        Q_UNUSED(namespaceURI);
        Q_UNUSED(qName);
        tmp = QString();
        if (localName == QLatin1String("Url")) {
            if (atts.value(QLatin1String("type")) == QLatin1String("text/html"))
                engine->url = atts.value(QLatin1String("template")).toString();
        }

        return true;
    }

    bool endElement(const QStringRef &namespaceURI, const QStringRef &localName, const QStringRef &qName)
    {
        Q_UNUSED(namespaceURI)
        Q_UNUSED(localName)
        Q_UNUSED(qName)

        if (localName == QLatin1String("ShortName"))
            engine->name = tmp;
        else if (localName == QLatin1String("Description"))
            engine->description = tmp;
        else if (localName == QLatin1String("Image"))
            engine->icon_url = tmp;

        return true;
    }

    bool parse(const QByteArray &data)
    {
        QXmlStreamReader reader(data);

        while (!reader.atEnd()) {
            reader.readNext();
            if (reader.hasError())
                return false;

            switch (reader.tokenType()) {
            case QXmlStreamReader::StartElement:
                if (!startElement(reader.namespaceUri(), reader.name(), reader.qualifiedName(), reader.attributes())) {
                    return false;
                }
                break;
            case QXmlStreamReader::EndElement:
                if (!endElement(reader.namespaceUri(), reader.name(), reader.qualifiedName())) {
                    return false;
                }
                break;
            case QXmlStreamReader::Characters:
                if (!reader.isWhitespace() && !reader.text().trimmed().isEmpty()) {
                    if (!characters(reader.text()))
                        return false;
                }
                break;
            default:
                break;
            }
        }

        if (!reader.isEndDocument())
            return false;

        return true;
    }

    SearchEngine *engine;
    QString tmp;
};

SearchEngine::SearchEngine(const QString &data_dir)
    : data_dir(data_dir)
{
}

SearchEngine::~SearchEngine()
{
}

bool SearchEngine::load(const QString &xml_file)
{
    QFile fptr(xml_file);
    if (!fptr.open(QIODevice::ReadOnly))
        return false;

    QByteArray source = fptr.readAll();
    OpenSearchHandler hdlr(this);

    const bool success = hdlr.parse(source);
    if (!success) {
        Out(SYS_SRC | LOG_NOTICE) << "Failed to parse opensearch description !" << endl;
        return false;
    }

    // check if icon file is present in data_dir
    // if not, download it
    if (!icon_url.isEmpty()) {
        QString icon_name = QUrl(icon_url).fileName();
        QString icon_filename = data_dir + icon_name;
        bool found = false;
        found = bt::Exists(icon_filename);
        if (!found) {
            // if there is an icon in xml file folder - use it
            // xml file folder might not be equal to data_dir
            icon_filename = QFileInfo(fptr).absolutePath() + QLatin1Char('/') + icon_name;
            found = bt::Exists(icon_filename);
        }

        if (!found) {
            KJob *j = KIO::storedGet(QUrl(icon_url), KIO::Reload, KIO::HideProgressInfo);
            connect(j, &KJob::result, this, &SearchEngine::iconDownloadFinished);
        } else {
            // load the icon
            icon = QIcon(icon_filename);
        }
    }

    return true;
}

QUrl SearchEngine::search(const QString &terms)
{
    QString r = url;
    r = r.replace(QLatin1String("{searchTerms}"), terms);
    return QUrl(r);
}

void SearchEngine::iconDownloadFinished(KJob *job)
{
    if (!job->error()) {
        QString icon_name = QUrl(icon_url).fileName();
        KIO::StoredTransferJob *j = (KIO::StoredTransferJob *)job;
        QFile fptr(data_dir + icon_name);
        if (!fptr.open(QIODevice::WriteOnly)) {
            Out(SYS_SRC | LOG_NOTICE) << "Failed to save icon: " << fptr.errorString() << endl;
            return;
        }

        fptr.write(j->data());
        fptr.close();

        // load the icon
        icon = QIcon(data_dir + icon_name);
    }
}
}
