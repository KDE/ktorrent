/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2023 Jack Hill <jackhill3103@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_GEOIPMANAGER_H
#define KT_GEOIPMANAGER_H

#include <QObject>
#include <QTemporaryFile>

#include <optional>

#include <maxminddb.h>

class KJob;

namespace kt
{
struct Country {
    QString name;
    QString code;
};

/**
 * Manages GeoIP database. Downloads it from the internet and handles all queries to it.
 */
class GeoIPManager : public QObject
{
    Q_OBJECT
public:
    GeoIPManager(QObject *parent = nullptr);
    ~GeoIPManager() override;

    /**
     * Find the country given an IP address
     * @param addr The IP address
     * @return The country
     */
    [[nodiscard]] Country findCountry(const QStringView ip) const;

private:
    /*
     * Whether the database file exists
     *
     * This does not check validity of the database.
     */
    [[nodiscard]] bool hasDatabase() const;

    /*
     * Attempts to open the database
     *
     * The result of this can be queried using the member `db_open`.
     */
    void openDatabase();

    /**
     * Closes the database
     */
    void closeDatabase();

    /**
     * Whether the database can provide entries in this language
     *
     * Even if this returns true, there is no guarantee that an entry
     * can provide results in this language.
     */
    bool languageIsSupported(const QByteArray language_code) const;

    /**
     * Convert the current QLocale to a locale supported by the database
     *
     * The result of this can be queried using the member `mmdb_locale`.
     * If the database is not open then `mmdb_locale` has no meaningful value.
     * This function will always fall back to English as a supported locale.
     */
    void updateSupportedLocale();

    /**
     * Get the database entry corresponding to the ip address
     *
     * Returns empty if the ip could not be found or if an error occurred.
     */
    [[nodiscard]] std::optional<MMDB_entry_s> lookupIP(const char *ip) const;

    /**
     * Returns the country name of an entry
     *
     * If an error occurs, an empty QString will be returned. This function will
     * try to return the country in the system locale. If the system locale
     * cannot be used then the result will be in English.
     */
    [[nodiscard]] QString countryName(MMDB_entry_s db_entry) const;

    /**
     * Returns the country code on an entry
     *
     * If an error occurs, an empty QString will be returned.
     */
    [[nodiscard]] QString countryCode(MMDB_entry_s db_entry) const;

    /**
     * Download a new copy the database and overwrite the old database
     *
     * If the database is successfully downloaded then the old database is overwritten
     * and the new database is opened. This function closes the old database and opens
     * the new database automatically.
     */
    void downloadDatabase();

private Q_SLOTS:
    /**
     * Start extracting the new database
     */
    void downloadFinished(KJob *download_job);

    /**
     * Close the old database then overwrite it with the new database
     */
    void extractionFinished(KJob *extraction_job);

    /**
     * Open the new database
     */
    void overwriteFinished(KJob *overwrite_job);

private:
    MMDB_s db;
    bool db_open;
    QByteArray mmdb_locale;
    bool downloading;
    QTemporaryFile download_file;
    QTemporaryFile decompress_file;
};

}

#endif // KT_GEOIPMANAGER_H
