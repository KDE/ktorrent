/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringView>
#include <QUrlQuery>
#include <QXmlStreamReader>

#include <KIO/StoredTransferJob>
#include <KLocalizedString>

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

    bool characters(const QStringView ch)
    {
        tmp += ch;
        return true;
    }

    bool startElement(const QStringView namespaceURI, const QStringView localName, const QStringView qName, const QXmlStreamAttributes &atts)
    {
        Q_UNUSED(namespaceURI);
        Q_UNUSED(qName);
        tmp = QString();
        if (localName == QLatin1String("Url")) {
            if (atts.value(QLatin1String("type")) == QLatin1String("text/html")) {
                engine->url = atts.value(QLatin1String("template")).toString();
            }
        }

        return true;
    }

    bool endElement(const QStringView namespaceURI, const QStringView localName, const QStringView qName)
    {
        Q_UNUSED(namespaceURI)
        Q_UNUSED(localName)
        Q_UNUSED(qName)

        if (localName == QLatin1String("ShortName")) {
            engine->name = tmp;
        } else if (localName == QLatin1String("Description")) {
            engine->description = tmp;
        } else if (localName == QLatin1String("Image")) {
            engine->icon_url = tmp;
        }

        return true;
    }

    bool parse(const QByteArray &data)
    {
        QXmlStreamReader reader(data);

        while (!reader.atEnd()) {
            reader.readNext();
            if (reader.hasError()) {
                return false;
            }

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
                    if (!characters(reader.text())) {
                        return false;
                    }
                }
                break;
            default:
                break;
            }
        }

        if (!reader.isEndDocument()) {
            return false;
        }

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

QString SearchEngine::torznabConfigFileName()
{
    return QStringLiteral("torznab.json");
}

bool SearchEngine::load(const QString &descriptor_file)
{
    if (descriptor_file.endsWith(QLatin1String(".json"))) {
        return loadTorznab(descriptor_file);
    }

    return loadOpenSearch(descriptor_file);
}

bool SearchEngine::loadOpenSearch(const QString &xml_file)
{
    QFile fptr(xml_file);
    if (!fptr.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray source = fptr.readAll();
    OpenSearchHandler hdlr(this);

    const bool success = hdlr.parse(source);
    if (!success) {
        Out(SYS_SRC | LOG_NOTICE) << "Failed to parse opensearch description !" << endl;
        return false;
    }

    type = SearchEngineType::OpenSearch;
    torznab_config = TorznabEngineConfig();

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

TorznabEngineConfig SearchEngine::normalizedTorznabConfig(const TorznabEngineConfig &config)
{
    TorznabEngineConfig normalized = config;
    normalized.name = normalized.name.trimmed();
    normalized.description = normalized.description.trimmed();
    normalized.apiKey = normalized.apiKey.trimmed();

    QString serviceUrl = normalized.serviceUrl.toString(QUrl::RemoveQuery | QUrl::RemoveFragment).trimmed();
    while (serviceUrl.endsWith(QLatin1Char('/'))) {
        serviceUrl.chop(1);
    }
    normalized.serviceUrl = QUrl(serviceUrl);
    normalized.threadCount = qMax(1, normalized.threadCount);
    return normalized;
}

bool SearchEngine::writeTorznabConfig(const QString &config_path, const TorznabEngineConfig &config, QString *error_message)
{
    const TorznabEngineConfig normalized = normalizedTorznabConfig(config);

    if (normalized.name.isEmpty()) {
        if (error_message) {
            *error_message = i18n("The engine name cannot be empty.");
        }
        return false;
    }

    if (!normalized.serviceUrl.isValid() || normalized.serviceUrl.host().isEmpty()
        || (normalized.serviceUrl.scheme() != QLatin1String("http") && normalized.serviceUrl.scheme() != QLatin1String("https"))) {
        if (error_message) {
            *error_message = i18n("The Jackett/Torznab URL must be a valid HTTP or HTTPS address.");
        }
        return false;
    }

    if (normalized.apiKey.isEmpty()) {
        if (error_message) {
            *error_message = i18n("The API key cannot be empty.");
        }
        return false;
    }

    QFile fptr(config_path);
    if (!fptr.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error_message) {
            *error_message = i18n("Cannot open %1: %2", config_path, fptr.errorString());
        }
        return false;
    }

    const QJsonObject root{
        {QStringLiteral("type"), QStringLiteral("torznab")},
        {QStringLiteral("name"), normalized.name},
        {QStringLiteral("description"), normalized.description},
        {QStringLiteral("url"), normalized.serviceUrl.toString()},
        {QStringLiteral("api_key"), normalized.apiKey},
        {QStringLiteral("tracker_first"), normalized.trackerFirst},
        {QStringLiteral("thread_count"), normalized.threadCount},
    };

    fptr.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool SearchEngine::loadTorznab(const QString &json_file)
{
    QFile fptr(json_file);
    if (!fptr.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonParseError parse_error;
    const QJsonDocument document = QJsonDocument::fromJson(fptr.readAll(), &parse_error);
    if (parse_error.error != QJsonParseError::NoError || !document.isObject()) {
        Out(SYS_SRC | LOG_NOTICE) << "Failed to parse torznab configuration: " << parse_error.errorString() << endl;
        return false;
    }

    const QJsonObject root = document.object();

    TorznabEngineConfig config;
    config.name = root.value(QStringLiteral("name")).toString();
    config.description = root.value(QStringLiteral("description")).toString();
    config.serviceUrl = QUrl(root.value(QStringLiteral("url")).toString());
    config.apiKey = root.value(QStringLiteral("api_key")).toString();
    config.trackerFirst = root.value(QStringLiteral("tracker_first")).toBool(false);
    config.threadCount = root.value(QStringLiteral("thread_count")).toInt(20);
    config = normalizedTorznabConfig(config);

    if (config.name.isEmpty() || !config.serviceUrl.isValid() || config.serviceUrl.host().isEmpty() || config.apiKey.isEmpty()) {
        Out(SYS_SRC | LOG_NOTICE) << "Invalid torznab configuration in " << json_file << endl;
        return false;
    }

    type = SearchEngineType::Torznab;
    torznab_config = config;
    name = config.name;
    description = config.description.isEmpty() ? i18n("Jackett/Torznab search engine") : config.description;
    url = config.serviceUrl.toString();
    icon_url.clear();
    icon = QIcon::fromTheme(QStringLiteral("network-server"));
    if (icon.isNull()) {
        icon = QIcon::fromTheme(QStringLiteral("edit-find"));
    }

    return true;
}

QUrl SearchEngine::search(const QString &terms) const
{
    if (type == SearchEngineType::Torznab) {
        QUrl torznabUrl;
        torznabUrl.setScheme(QStringLiteral("torznab"));
        torznabUrl.setPath(QStringLiteral("/") + name);
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("q"), terms);
        torznabUrl.setQuery(query);
        return torznabUrl;
    }

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

#include "moc_searchengine.cpp"
