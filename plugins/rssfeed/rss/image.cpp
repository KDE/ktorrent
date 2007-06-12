/*
 * image.cpp
 *
 * Copyright (c) 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "image.h"
#include "tools_p.h"

#include <kio/job.h>
#include <kurl.h>

#include <qbuffer.h>
#include <qdom.h>
#include <qpixmap.h>

using namespace RSS;

struct Image::Private : public Shared
{
	Private() : height(31), width(88), pixmapBuffer(NULL), job(NULL)
		{ }

	QString title;
	KURL url;
	KURL link;
	QString description;
	unsigned int height;
	unsigned int width;
	QBuffer *pixmapBuffer;
	KIO::Job *job;
};

Image::Image() : QObject(), d(new Private)
{
}

Image::Image(const Image &other) : QObject(), d(0)
{
	*this = other;
}

Image::Image(const QDomNode &node) : QObject(), d(new Private)
{
	QString elemText;

	if (!(elemText = extractNode(node, QString::fromLatin1("title"))).isNull())
		d->title = elemText;
	if (!(elemText = extractNode(node, QString::fromLatin1("url"))).isNull())
		d->url = elemText;
	if (!(elemText = extractNode(node, QString::fromLatin1("link"))).isNull())
		d->link = elemText;
	if (!(elemText = extractNode(node, QString::fromLatin1("description"))).isNull())
		d->description = elemText;
	if (!(elemText = extractNode(node, QString::fromLatin1("height"))).isNull())
		d->height = elemText.toUInt();
	if (!(elemText = extractNode(node, QString::fromLatin1("width"))).isNull())
		d->width = elemText.toUInt();
}

Image::~Image()
{
	if (d->deref())
        {
            delete d->pixmapBuffer;
            d->pixmapBuffer=0L;
            delete d;
        }
}

QString Image::title() const
{
	return d->title;
}

const KURL &Image::url() const
{
	return d->url;
}

const KURL &Image::link() const
{
	return d->link;
}

QString Image::description() const
{
	return d->description;
}

unsigned int Image::height() const
{
	return d->height;
}

unsigned int Image::width() const
{
	return d->width;
}

void Image::getPixmap()
{
	// Ignore subsequent calls if we didn't finish the previous download.
	if (d->pixmapBuffer)
		return;

	d->pixmapBuffer = new QBuffer;
	d->pixmapBuffer->open(IO_WriteOnly);

	d->job = KIO::get(d->url, false, false);
	connect(d->job, SIGNAL(data(KIO::Job *, const QByteArray &)),
	        this, SLOT(slotData(KIO::Job *, const QByteArray &)));
	connect(d->job, SIGNAL(result(KIO::Job *)), this, SLOT(slotResult(KIO::Job *)));
}

void Image::slotData(KIO::Job *, const QByteArray &data)
{
	d->pixmapBuffer->writeBlock(data.data(), data.size());
}

void Image::slotResult(KIO::Job *job)
{
	QPixmap pixmap;
	if (!job->error())
		pixmap = QPixmap(d->pixmapBuffer->buffer());
	emit gotPixmap(pixmap);

	delete d->pixmapBuffer;
	d->pixmapBuffer = NULL;
}

void Image::abort()
{
	if (d->job)
	{
		d->job->kill(true);
		d->job = NULL;
	}
}

Image &Image::operator=(const Image &other)
{
	if (this != &other) {
		other.d->ref();
		if (d && d->deref())
			delete d;
		d = other.d;
	}
	return *this;
}

bool Image::operator==(const Image &other) const
{
	return d->title == other.title() &&
	       d->url == other.url() &&
	       d->description == other.description() &&
	       d->height == other.height() &&
	       d->width == other.width() &&
	       d->link == other.link();
}

#include "image.moc"
// vim:noet:ts=4
