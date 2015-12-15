/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#include <kio/job.h>
#include <util/fileops.h>
#include <util/log.h>
#include <QXmlDefaultHandler>
#include <QXmlInputSource>
#include "searchengine.h"

using namespace bt;

namespace kt
{
    class OpenSearchHandler : public QXmlDefaultHandler
    {
    public:
        OpenSearchHandler(SearchEngine* engine) : engine(engine)
        {
        }

        virtual ~OpenSearchHandler()
        {
        }

        virtual bool characters(const QString& ch)
        {
            tmp += ch;
            return true;
        }

        virtual bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts)
        {
            Q_UNUSED(namespaceURI);
            Q_UNUSED(qName);
            tmp = QString();
            if (localName == QLatin1String("Url"))
            {
                if (atts.value(QLatin1String("type")) == QLatin1String("text/html"))
                    engine->url = atts.value(QLatin1String("template"));
            }

            return true;
        }

        virtual bool endElement(const QString& namespaceURI, const QString& localName, const QString& qName)
        {
            Q_UNUSED(namespaceURI);
            Q_UNUSED(qName);

            if (localName == QLatin1String("ShortName"))
                engine->name = tmp;
            else if (localName == QLatin1String("Description"))
                engine->description = tmp;
            else if (localName == QLatin1String("Image"))
                engine->icon_url = tmp;

            return true;
        }


        SearchEngine* engine;
        QString tmp;
    };

    SearchEngine::SearchEngine(const QString& data_dir) : data_dir(data_dir)
    {
    }


    SearchEngine::~SearchEngine()
    {
    }

    bool SearchEngine::load(const QString& xml_file)
    {
        QXmlSimpleReader xml_reader;
        QFile fptr(xml_file);
        QXmlInputSource source(&fptr);
        OpenSearchHandler hdlr(this);
        xml_reader.setErrorHandler(&hdlr);
        xml_reader.setContentHandler(&hdlr);

        if (!xml_reader.parse(&source))
        {
            Out(SYS_SRC | LOG_NOTICE) << "Failed to parse opensearch description !" << endl;
            return false;
        }

        // check if icon file is present in data_dir
        // if not, download it
        if (!icon_url.isEmpty())
        {
            QString icon_name = QUrl(icon_url).fileName();
            if (!bt::Exists(data_dir + icon_name))
            {
                KJob* j = KIO::storedGet(QUrl(icon_url), KIO::Reload, KIO::HideProgressInfo);
                connect(j, SIGNAL(result(KJob*)), this, SLOT(iconDownloadFinished(KJob*)));
            }
            else
            {
                // load the icon
                icon = QIcon(data_dir + icon_name);
            }
        }

        return true;
    }

    QUrl SearchEngine::search(const QString& terms)
    {
        QString r = url;
        r = r.replace(QLatin1String("{searchTerms}"), terms);
        return QUrl(r);
    }

    void SearchEngine::iconDownloadFinished(KJob* job)
    {
        if (!job->error())
        {
            QString icon_name = QUrl(icon_url).fileName();
            KIO::StoredTransferJob* j = (KIO::StoredTransferJob*)job;
            QFile fptr(data_dir + icon_name);
            if (!fptr.open(QIODevice::WriteOnly))
            {
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
