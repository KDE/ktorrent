/*
 * tools_p.cpp
 *
 * Copyright (c) 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "tools_p.h"

#include <krfcdate.h>
#include <qdom.h>

time_t RSS::parseISO8601Date(const QString &s)
{
    // do some sanity check: 26-12-2004T00:00+00:00 is parsed to epoch+1 in the KRFCDate, which is wrong. So let's check if the date begins with YYYY -fo
    if (s.stripWhiteSpace().left(4).toInt() < 1000)
        return 0; // error

    // FIXME: imho this is done in KRFCDate::parseDateISO8601() automatically, so we could omit it? -fo
	if (s.find('T') != -1)
		return KRFCDate::parseDateISO8601(s);
    else
        return KRFCDate::parseDateISO8601(s + "T12:00:00");
}


QString RSS::extractNode(const QDomNode &parent, const QString &elemName, bool isInlined)
{
	QDomNode node = parent.namedItem(elemName);
	if (node.isNull())
		return QString::null;

	QString result = node.toElement().text();

	bool hasPre = result.contains("<pre>",false);
	bool hasHtml = hasPre || result.contains("<");	// FIXME: test if we have html, should be more clever -> regexp
	if(!isInlined && !hasHtml)						// perform nl2br if not a inline elt and it has no html elts
		result = result = result.replace(QChar('\n'), "<br />");
	if(!hasPre)										// strip white spaces if no <pre>
		result = result.simplifyWhiteSpace();

	if (result.isEmpty())
		return QString::null;

	return result;
}

// vim:noet:ts=4
