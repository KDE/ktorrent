/*
 * textinput.cpp
 *
 * Copyright (c) 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "textinput.h"
#include "tools_p.h"

#include <kurl.h>

#include <qdom.h>

using namespace RSS;

struct TextInput::Private : public Shared
{
	QString title;
	QString description;
	QString name;
	KURL link;
};

TextInput::TextInput() : d(new Private)
{
}

TextInput::TextInput(const TextInput &other) : d(0)
{
	*this = other;
}

TextInput::TextInput(const QDomNode &node) : d(new Private)
{
	QString elemText;

	if (!(elemText = extractNode(node, QString::fromLatin1("title"))).isNull())
		d->title = elemText;
	if (!(elemText = extractNode(node, QString::fromLatin1("description"))).isNull())
		d->description = elemText;
	if (!(elemText = extractNode(node, QString::fromLatin1("name"))))
		d->name = elemText;
	if (!(elemText = extractNode(node, QString::fromLatin1("link"))).isNull())
		d->link = elemText;
}

TextInput::~TextInput()
{
	if (d->deref())
		delete d;
}

QString TextInput::title() const
{
	return d->title;
}

QString TextInput::description() const
{
	return d->description;
}

QString TextInput::name() const
{
	return d->name;
}

const KURL &TextInput::link() const
{
	return d->link;
}

TextInput &TextInput::operator=(const TextInput &other)
{
	if (this != &other) {
		other.d->ref();
		if (d && d->deref())
			delete d;
		d = other.d;
	}
	return *this;
}

bool TextInput::operator==(const TextInput &other) const
{
	return d->title == other.title() &&
	       d->description == other.description() &&
		   d->name == other.name() &&
		   d->link == other.link();
}

// vim:noet:ts=4
