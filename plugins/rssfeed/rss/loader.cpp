/*
 * loader.cpp
 *
 * Copyright (c) 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "loader.h"
#include "document.h"

#include <kio/job.h>
#include <kprocess.h>
#include <kurl.h>
#include <kdebug.h>

#include <qdom.h>
#include <qbuffer.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qtimer.h>

using namespace RSS;

DataRetriever::DataRetriever()
{
}

DataRetriever::~DataRetriever()
{
}

struct FileRetriever::Private
{
   Private()
      : buffer(NULL),
        lastError(0), job(NULL)
   {
   }

   ~Private()
   {
      delete buffer;
   }

   QBuffer *buffer;
   int lastError;
   KIO::Job *job;
};

FileRetriever::FileRetriever()
   : d(new Private)
{
}

FileRetriever::~FileRetriever()
{
   delete d;
}

bool FileRetriever::m_useCache = true;

void FileRetriever::setUseCache(bool enabled)
{
    m_useCache = enabled;
}

void FileRetriever::retrieveData(const KURL &url)
{
   if (d->buffer)
      return;

   d->buffer = new QBuffer;
   d->buffer->open(IO_WriteOnly);

   KURL u=url;

   if (u.protocol()=="feed")
       u.setProtocol("http");

   d->job = KIO::get(u, !m_useCache, false);

   
   QTimer::singleShot(1000*90, this, SLOT(slotTimeout()));
   
   connect(d->job, SIGNAL(data(KIO::Job *, const QByteArray &)),
                SLOT(slotData(KIO::Job *, const QByteArray &)));
   connect(d->job, SIGNAL(result(KIO::Job *)), SLOT(slotResult(KIO::Job *)));
   connect(d->job, SIGNAL(permanentRedirection(KIO::Job *, const KURL &, const KURL &)),
                SLOT(slotPermanentRedirection(KIO::Job *, const KURL &, const KURL &)));
}

void FileRetriever::slotTimeout()
{
    abort();

    delete d->buffer;
    d->buffer = NULL;

    d->lastError = KIO::ERR_SERVER_TIMEOUT;
    
    emit dataRetrieved(QByteArray(), false);
}

int FileRetriever::errorCode() const
{
   return d->lastError;
}

void FileRetriever::slotData(KIO::Job *, const QByteArray &data)
{
   d->buffer->writeBlock(data.data(), data.size());
}

void FileRetriever::slotResult(KIO::Job *job)
{
   QByteArray data = d->buffer->buffer();
   data.detach();

   delete d->buffer;
   d->buffer = NULL;

   d->lastError = job->error();
   emit dataRetrieved(data, d->lastError == 0);
}

void FileRetriever::slotPermanentRedirection(KIO::Job *, const KURL &, const KURL &newUrl)
{
   emit permanentRedirection(newUrl);
}

void FileRetriever::abort()
{
	if (d->job)
	{
		d->job->kill(true);
		d->job = NULL;
	}
}

struct OutputRetriever::Private
{
   Private() : process(NULL),
      buffer(NULL),
      lastError(0)
   {
   }

   ~Private()
   {
      delete process;
      delete buffer;
   }

   KShellProcess *process;
   QBuffer *buffer;
   int lastError;
};

OutputRetriever::OutputRetriever() :
   d(new Private)
{
}

OutputRetriever::~OutputRetriever()
{
   delete d;
}

void OutputRetriever::retrieveData(const KURL &url)
{
   // Ignore subsequent calls if we didn't finish the previous job yet.
   if (d->buffer || d->process)
      return;

   d->buffer = new QBuffer;
   d->buffer->open(IO_WriteOnly);

   d->process = new KShellProcess();
   connect(d->process, SIGNAL(processExited(KProcess *)),
                       SLOT(slotExited(KProcess *)));
   connect(d->process, SIGNAL(receivedStdout(KProcess *, char *, int)),
                       SLOT(slotOutput(KProcess *, char *, int)));
   *d->process << url.path();
   d->process->start(KProcess::NotifyOnExit, KProcess::Stdout);
}

int OutputRetriever::errorCode() const
{
   return d->lastError;
}

void OutputRetriever::slotOutput(KProcess *, char *data, int length)
{
   d->buffer->writeBlock(data, length);
}

void OutputRetriever::slotExited(KProcess *p)
{
   if (!p->normalExit())
      d->lastError = p->exitStatus();

   QByteArray data = d->buffer->buffer();
   data.detach();

   delete d->buffer;
   d->buffer = NULL;

   delete d->process;
   d->process = NULL;

   emit dataRetrieved(data, p->normalExit() && p->exitStatus() == 0);
}

struct Loader::Private
{
   Private() : retriever(NULL),
      lastError(0)
   {
   }

   ~Private()
   {
      delete retriever;
   }

   DataRetriever *retriever;
   int lastError;
   KURL discoveredFeedURL;
   KURL url;
};

Loader *Loader::create()
{
   return new Loader;
}

Loader *Loader::create(QObject *object, const char *slot)
{
   Loader *loader = create();
   connect(loader, SIGNAL(loadingComplete(Loader *, Document, Status)),
           object, slot);
   return loader;
}

Loader::Loader() : d(new Private)
{
}

Loader::~Loader()
{
    delete d;
}

void Loader::loadFrom(const KURL &url, DataRetriever *retriever)
{
   if (d->retriever != NULL)
      return;

   d->url=url;
   d->retriever = retriever;

   connect(d->retriever, SIGNAL(dataRetrieved(const QByteArray &, bool)),
           this, SLOT(slotRetrieverDone(const QByteArray &, bool)));

   d->retriever->retrieveData(url);
}

int Loader::errorCode() const
{
   return d->lastError;
}

void Loader::abort()
{
	if (d && d->retriever)
	{
		d->retriever->abort();
        delete d->retriever;
		d->retriever=NULL;
	}
    emit loadingComplete(this, QDomDocument(), Aborted);
    delete this;
}

const KURL &Loader::discoveredFeedURL() const
{
   return d->discoveredFeedURL;
}

#include <kdebug.h>

void Loader::slotRetrieverDone(const QByteArray &data, bool success)
{
   d->lastError = d->retriever->errorCode();

   delete d->retriever;
   d->retriever = NULL;

   Document rssDoc;
   Status status = Success;

   if (success) {
      QDomDocument doc;

      /* Some servers insert whitespace before the <?xml...?> declaration.
       * QDom doesn't tolerate that (and it's right, that's invalid XML),
       * so we strip that.
       */

      const char *charData = data.data();
      int len = data.count();

      while (len && QChar(*charData).isSpace()) {
         --len;
         ++charData;
      }

      if ( len > 3 && QChar(*charData) == QChar(0357) ) { // 0357 0273 0277 
			  len -= 3;
			  charData += 3;
	  }
      QByteArray tmpData;
      tmpData.setRawData(charData, len);
      
      if (doc.setContent(tmpData))
      {
         rssDoc = Document(doc);
         if (!rssDoc.isValid())
         {
            discoverFeeds(tmpData);
            status = ParseError;
         }
      }
      else
      {
         discoverFeeds(tmpData);
         status = ParseError;
      }

      tmpData.resetRawData(charData, len);      
   } else
      status = RetrieveError;

   emit loadingComplete(this, rssDoc, status);

   delete this;
}

void Loader::discoverFeeds(const QByteArray &data)
{
    QString str = QString(data).simplifyWhiteSpace();
    QString s2;
    //QTextStream ts( &str, IO_WriteOnly );
    //ts << data.data();
    
    // "<[\\s]link[^>]*rel[\\s]=[\\s]\\\"[\\s]alternate[\\s]\\\"[^>]*>"
    // "type[\\s]=[\\s]\\\"application/rss+xml\\\""
    // "href[\\s]=[\\s]\\\"application/rss+xml\\\""
    QRegExp rx( "(?:REL)[^=]*=[^sAa]*(?:service.feed|ALTERNATE)[\\s]*[^s][^s](?:[^>]*)(?:HREF)[^=]*=[^A-Z0-9-_~,./$]*([^'\">\\s]*)", false);
    if (rx.search(str)!=-1)
        s2=rx.cap(1);
    else{
    // does not support Atom/RSS autodiscovery.. try finding feeds by brute force....
        int pos=0;
        QStringList feeds;
        QString host=d->url.host();
        rx.setPattern("(?:<A )[^H]*(?:HREF)[^=]*=[^A-Z0-9-_~,./]*([^'\">\\s]*)");
        while ( pos >= 0 ) {
            pos = rx.search( str, pos );
            s2=rx.cap(1);
            if (s2.endsWith(".rdf")|s2.endsWith(".rss")|s2.endsWith(".xml"))
                    feeds.append(s2);
            if ( pos >= 0 ) {
                pos += rx.matchedLength();
            }
        }

        s2=feeds.first();
        KURL testURL;
        // loop through, prefer feeds on same host
        for ( QStringList::Iterator it = feeds.begin(); it != feeds.end(); ++it ) {
            testURL=*it;
            if (testURL.host()==host)
            {
                s2=*it;
                break;
            }
        }
    }
    
    if (s2.isNull()) {
        kdDebug() << "No feed found for a site" << endl;
        return;
    }

    if (KURL::isRelativeURL(s2))
    {
        if (s2.startsWith("//"))
        {
            s2=s2.prepend(d->url.protocol()+":");
            d->discoveredFeedURL=s2;
        }
        else if (s2.startsWith("/"))
        {
            d->discoveredFeedURL=d->url;
            d->discoveredFeedURL.setPath(s2);
        }
        else
        {
            d->discoveredFeedURL=d->url;
            d->discoveredFeedURL.addPath(s2);
        }
        d->discoveredFeedURL.cleanPath();
    }
    else
        d->discoveredFeedURL=s2;
    
    d->discoveredFeedURL.cleanPath();
}
    
#include "loader.moc"
// vim:noet:ts=4
