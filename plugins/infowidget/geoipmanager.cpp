/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2023 Jack Hill <jackhill3103@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "geoipmanager.h"

#include <KIO/FileCopyJob>

#include <QDateTime>
#include <QLocale>
#include <QSaveFile>

#include <interfaces/functions.h>
#include <util/decompressfilejob.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
static const QString LOCAL_DATABASE_PATH = DataDir() + QStringLiteral("geoip.mmdb");

GeoIPManager::GeoIPManager(QObject *parent)
    : QObject(parent)
    , db_open{false}
    , downloading{false}
{
    if (!hasDatabase()) {
        downloadDatabase();
    } else {
        openDatabase();
    }
}

GeoIPManager::~GeoIPManager()
{
    closeDatabase();
}

Country GeoIPManager::findCountry(const QStringView ip) const
{
    const auto ip_utf8 = ip.toUtf8().constData();
    auto db_entry = lookupIP(ip_utf8);
    if (!db_entry.has_value()) {
        return {};
    }

    const auto name = countryName(db_entry.value());
    const auto code = countryCode(db_entry.value());
    return Country{name, code};
}

bool GeoIPManager::hasDatabase() const
{
    return QFile::exists(LOCAL_DATABASE_PATH);
}

void GeoIPManager::openDatabase()
{
    const auto db_name = LOCAL_DATABASE_PATH.toLocal8Bit().constData();
    int status = MMDB_open(db_name, MMDB_MODE_MMAP, &db);

    if (status != MMDB_SUCCESS) {
        Out(SYS_INW | LOG_IMPORTANT) << "Error from libmaxmindb when opening " << db_name << " - " << MMDB_strerror(status) << endl;
        db_open = false;
    } else {
        db_open = true;
        updateSupportedLocale();
    }
}

void GeoIPManager::closeDatabase()
{
    if (!db_open) {
        return;
    }
    MMDB_close(&db);
    db_open = false;
}

bool GeoIPManager::languageIsSupported(const QByteArray language_code) const
{
    if (!db_open) {
        return false;
    }

    for (int i = 0; i < static_cast<int>(db.metadata.languages.count); i++) {
        if (language_code.compare(db.metadata.languages.names[i]) == 0) {
            return true;
        }
    }
    return false;
}

void GeoIPManager::updateSupportedLocale()
{
    // Qt gives codes like pt_BR but mmdb gives codes like pt-BR
    const auto qt_locale = QLocale::system().name().replace(QLatin1Char('_'), QLatin1Char('-'));

    mmdb_locale = qt_locale.toUtf8();
    if (!languageIsSupported(mmdb_locale)) {
        // Try only the first part of country code (e.g. fr-FR -> fr)
        mmdb_locale = QStringView(qt_locale).split(QLatin1Char('-')).at(0).toUtf8();
        if (!languageIsSupported(mmdb_locale)) {
            mmdb_locale = QByteArrayLiteral("en");
        }
    }
}

std::optional<MMDB_entry_s> GeoIPManager::lookupIP(const char *ip) const
{
    if (!db_open) {
        return {};
    }
    int gai_error, mmdb_error;
    MMDB_lookup_result_s result = MMDB_lookup_string(&db, ip, &gai_error, &mmdb_error);
    if (gai_error != 0) {
        Out(SYS_INW | LOG_DEBUG) << "Error from getaddrinfo for ip " << ip << " - " << gai_strerror(gai_error) << endl;
        return {};
    }
    if (mmdb_error != MMDB_SUCCESS) {
        Out(SYS_INW | LOG_DEBUG) << "Error from libmaxmindb for ip " << ip << " - " << MMDB_strerror(mmdb_error) << endl;
        return {};
    }
    if (!result.found_entry) {
        Out(SYS_INW | LOG_DEBUG) << "IP is not in database " << ip << endl;
        return {};
    }

    return result.entry;
}

QString GeoIPManager::countryName(MMDB_entry_s db_entry) const
{
    MMDB_entry_data_s name;
    int status = MMDB_get_value(&db_entry, &name, "country", "names", mmdb_locale.constData(), NULL);

    // Maybe there is no DB entry for this locale so fall back to English (guaranteed DB entry)
    if (status == MMDB_LOOKUP_PATH_DOES_NOT_MATCH_DATA_ERROR) {
        status = MMDB_get_value(&db_entry, &name, "country", "names", "en", NULL);
    }

    if (status != MMDB_SUCCESS) {
        Out(SYS_INW | LOG_DEBUG) << "Error from libmaxmindb when reading country name "
                                 << " - " << MMDB_strerror(status) << endl;
        return {};
    }

    if (!name.has_data) {
        return {};
    }

    return QString::fromUtf8(name.utf8_string, name.data_size);
}

QString GeoIPManager::countryCode(MMDB_entry_s db_entry) const
{
    MMDB_entry_data_s code;
    int status = MMDB_get_value(&db_entry, &code, "country", "iso_code", NULL);

    if (status != MMDB_SUCCESS) {
        Out(SYS_INW | LOG_IMPORTANT) << "Error from libmaxmindb when reading country code ip "
                                     << " - " << MMDB_strerror(status) << endl;
        return {};
    }

    if (!code.has_data) {
        return {};
    }

    return QString::fromUtf8(code.utf8_string, code.data_size);
}

void GeoIPManager::downloadDatabase()
{
    if (downloading) {
        Out(SYS_INW | LOG_IMPORTANT) << "Attempted to download GeoIP database twice" << endl;
        return;
    }

    const QUrl external_db{
        QStringLiteral("https://download.db-ip.com/free/dbip-country-lite-%1.mmdb.gz").arg(QDate::currentDate().toString(QStringLiteral("yyyy-MM")))};
    Out(SYS_INW | LOG_NOTICE) << "Downloading new GeoIP database from " << external_db << endl;

    downloading = true;
    download_file.open();
    auto *download_job = KIO::file_copy(external_db, QUrl::fromLocalFile(download_file.fileName()), -1, KIO::Overwrite | KIO::HideProgressInfo);
    connect(download_job, &KIO::FileCopyJob::result, this, &GeoIPManager::downloadFinished);
}

void GeoIPManager::downloadFinished(KJob *download_job)
{
    downloading = false;
    if (download_job->error()) {
        Out(SYS_INW | LOG_IMPORTANT) << "Failed to download GeoIP database with error: " << download_job->errorString() << endl;
        download_file.close();
        return;
    }
    Out(SYS_INW | LOG_NOTICE) << "Downloaded GeoIP database" << endl;

    decompress_file.open();
    auto *decompress_job = new bt::DecompressFileJob(download_file.fileName(), decompress_file.fileName());
    connect(decompress_job, &KJob::result, this, &GeoIPManager::extractionFinished);
    decompress_job->start();
}

void GeoIPManager::extractionFinished(KJob *extraction_job)
{
    download_file.close();
    if (extraction_job->error()) {
        Out(SYS_INW | LOG_IMPORTANT) << "Failed to extract GeoIP database with error: " << extraction_job->errorString() << endl;
        decompress_file.close();
        return;
    }
    Out(SYS_INW | LOG_NOTICE) << "Extracted GeoIP database" << endl;

    closeDatabase();
    auto *move_job =
        KIO::file_move(QUrl::fromLocalFile(decompress_file.fileName()), QUrl::fromLocalFile(LOCAL_DATABASE_PATH), -1, KIO::Overwrite | KIO::HideProgressInfo);
    connect(move_job, &KIO::FileCopyJob::result, this, &GeoIPManager::overwriteFinished);
}

void GeoIPManager::overwriteFinished(KJob *overwrite_job)
{
    decompress_file.close();
    if (overwrite_job->error()) {
        Out(SYS_INW | LOG_IMPORTANT) << "Failed to overwrite GeoIP database with error: " << overwrite_job->errorString() << endl;
        return;
    }
    Out(SYS_INW | LOG_NOTICE) << "Updated GeoIP database" << endl;

    openDatabase();
}
}

#include "moc_geoipmanager.cpp"
