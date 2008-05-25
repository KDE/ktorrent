/*
 * document.cpp
 *
 * Copyright (c) 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 *
 */
#include "document.h"
#include "article.h"
#include "image.h"
#include "textinput.h"
#include "tools_p.h"

#include <krfcdate.h>
#include <kurl.h>

#include <qdatetime.h>
#include <qdom.h>
#include <qptrlist.h>

using namespace RSS;

struct Document::Private : public Shared
{
    Private() : version(v0_90), image(NULL), textInput(NULL), language(en)
    {
        format=UnknownFormat;
        valid=false;
		ttl=-1;
    }

    ~Private()
    {
        delete textInput;
        delete image;
    }

    Version version;
    QString title;
    QString description;
    KURL link;
    Image *image;
    TextInput *textInput;
    Article::List articles;
    Language language;
    Format format;
    QString copyright;
    QDateTime pubDate;
    QDateTime lastBuildDate;
    QString rating;
    KURL docs;
	int ttl;
    QString managingEditor;
    QString webMaster;
    HourList skipHours;
    DayList skipDays;
    bool valid;
};

Document::Document() : d(new Private)
{
}

Document::Document(const Document &other) : d(0)
{
    *this = other;
}

Document::Document(const QDomDocument &doc) : d(new Private)
{
    QString elemText;
    QDomNode rootNode = doc.documentElement();

    // Determine the version of the present RSS markup.
    QString attr;

    // we should probably check that it ISN'T feed or rss, rather than check if it is xhtml
    if (rootNode.toElement().tagName()==QString::fromLatin1("html"))
        d->valid=false;
    else
        d->valid=true;
    
    attr = rootNode.toElement().attribute(QString::fromLatin1("version"), QString::null);
    if (!attr.isNull()) {
        if (rootNode.toElement().tagName()=="feed")
        {
            d->format=AtomFeed;
            if (attr == QString::fromLatin1("0.3"))
                d->version = vAtom_0_3;
            else if (attr == QString::fromLatin1("0.2")) /* smt -> review */
               d->version = vAtom_0_2;
            else if (attr == QString::fromLatin1("0.1")) /* smt -> review */
              d->version = vAtom_0_1;
        }
        else
        {
            d->format=RSSFeed;
            if (attr == QString::fromLatin1("0.91"))
                d->version = v0_91;
            else if (attr == QString::fromLatin1("0.92"))
                d->version = v0_92;
            else if (attr == QString::fromLatin1("0.93"))
                d->version = v0_93;
            else if (attr == QString::fromLatin1("0.94"))
                d->version = v0_94;
            else if (attr.startsWith("2.0") || attr == QString::fromLatin1("2")) // http://www.breuls.org/rss puts 2.00 in version (BR #0000016)
                d->version = v2_0;
        }
    }

    if (d->format==UnknownFormat)
    {
        attr = rootNode.toElement().attribute(QString::fromLatin1("xmlns"), QString::null);
        if (!attr.isNull()) {
        /*
         * Hardcoding these URLs is actually a bad idea, since the DTD doesn't
         * dictate a specific namespace. Still, most RSS files seem to use
         * these two, so I'll go for them now. If it turns out that many
         * mirrors of this RSS namespace are in use, I'll probably have to
         * distinguish the RSS versions by analyzing the relationship between
         * the nodes.
         */
            if (attr == QString::fromLatin1("http://my.netscape.com/rdf/simple/0.9/")) {
                d->format=RSSFeed;
                d->version = v0_90;
             }
            else if (attr == QString::fromLatin1("http://purl.org/rss/1.0/")) {
                d->format=RSSFeed;
                d->version = v1_0;
            }
        }
    }

    QDomNode channelNode;

    if (d->format == AtomFeed)
        channelNode=rootNode;
    else
        channelNode=rootNode.namedItem(QString::fromLatin1("channel"));

    if (!(elemText = extractNode(channelNode, QString::fromLatin1("title"))).isNull())
        d->title = elemText;
    if (!(elemText = extractNode(channelNode, QString::fromLatin1("description"))).isNull())
        d->description = elemText;
    if (!(elemText = extractNode(channelNode, QString::fromLatin1("link"))).isNull())
        d->link = elemText;

    
    /* This is ugly but necessary since RSS 0.90 and 1.0 have a different parent
     * node for <image>, <textinput> and <item> than RSS 0.91-0.94 and RSS 2.0.
     */
    QDomNode parentNode;
    if (d->version == v0_90 || d->version == v1_0 || d->format == AtomFeed)
        parentNode = rootNode;
    else
    {
	// following is a HACK for broken 0.91 feeds like xanga.com's
	if (!rootNode.namedItem(QString::fromLatin1("item")).isNull())
	    parentNode = rootNode;
	else
            parentNode = channelNode;
    }
    
    // image and textinput aren't supported by Atom.. handle in case feed provides
    QDomNode n = parentNode.namedItem(QString::fromLatin1("image"));
    if (!n.isNull())
        d->image = new Image(n);

    n = parentNode.namedItem(QString::fromLatin1("textinput"));
    if (!n.isNull())
        d->textInput = new TextInput(n);

    // Our (hopefully faster) version of elementsByTagName()
    QString tagName;
    if (d->format == AtomFeed)
        tagName=QString::fromLatin1("entry");
    else
        tagName=QString::fromLatin1("item");

    for (n = parentNode.firstChild(); !n.isNull(); n = n.nextSibling()) {
        const QDomElement e = n.toElement();
        if (e.tagName() == tagName)
            d->articles.append(Article(e, d->format));
    }

    if (!(elemText = extractNode(channelNode, QString::fromLatin1("copyright"))).isNull())
        d->copyright = elemText;

    if (d->format == AtomFeed)
        elemText = rootNode.toElement().attribute(QString::fromLatin1("xml:lang"), QString::null);
    else
        elemText = extractNode(channelNode, QString::fromLatin1("language"));

    if (!elemText.isNull()){
        if (elemText == QString::fromLatin1("af"))
            d->language = af;
        else if (elemText == QString::fromLatin1("sq"))
            d->language = sq;
        else if (elemText == QString::fromLatin1("eu"))
            d->language = eu;
        else if (elemText == QString::fromLatin1("be"))
            d->language = be;
        else if (elemText == QString::fromLatin1("bg"))
            d->language = bg;
        else if (elemText == QString::fromLatin1("ca"))
            d->language = ca;
        else if (elemText == QString::fromLatin1("zh-cn"))
            d->language = zh_cn;
        else if (elemText == QString::fromLatin1("zh-tw"))
            d->language = zh_tw;
        else if (elemText == QString::fromLatin1("hr"))
            d->language = hr;
        else if (elemText == QString::fromLatin1("cs"))
            d->language = cs;
        else if (elemText == QString::fromLatin1("da"))
            d->language = da;
        else if (elemText == QString::fromLatin1("nl"))
            d->language = nl;
        else if (elemText == QString::fromLatin1("nl-be"))
            d->language = nl_be;
        else if (elemText == QString::fromLatin1("nl-nl"))
            d->language = nl_nl;
        else if (elemText == QString::fromLatin1("en"))
            d->language = en;
        else if (elemText == QString::fromLatin1("en-au"))
            d->language = en_au;
        else if (elemText == QString::fromLatin1("en-bz"))
            d->language = en_bz;
        else if (elemText == QString::fromLatin1("en-ca"))
            d->language = en_ca;
        else if (elemText == QString::fromLatin1("en-ie"))
            d->language = en_ie;
        else if (elemText == QString::fromLatin1("en-jm"))
            d->language = en_jm;
        else if (elemText == QString::fromLatin1("en-nz"))
            d->language = en_nz;
        else if (elemText == QString::fromLatin1("en-ph"))
            d->language = en_ph;
        else if (elemText == QString::fromLatin1("en-za"))
            d->language = en_za;
        else if (elemText == QString::fromLatin1("en-tt"))
            d->language = en_tt;
        else if (elemText == QString::fromLatin1("en-gb"))
            d->language = en_gb;
        else if (elemText == QString::fromLatin1("en-us"))
            d->language = en_us;
        else if (elemText == QString::fromLatin1("en-zw"))
            d->language = en_zw;
        else if (elemText == QString::fromLatin1("fo"))
            d->language = fo;
        else if (elemText == QString::fromLatin1("fi"))
            d->language = fi;
        else if (elemText == QString::fromLatin1("fr"))
            d->language = fr;
        else if (elemText == QString::fromLatin1("fr-be"))
            d->language = fr_be;
        else if (elemText == QString::fromLatin1("fr-ca"))
            d->language = fr_ca;
        else if (elemText == QString::fromLatin1("fr-fr"))
            d->language = fr_fr;
        else if (elemText == QString::fromLatin1("fr-lu"))
            d->language = fr_lu;
        else if (elemText == QString::fromLatin1("fr-mc"))
            d->language = fr_mc;
        else if (elemText == QString::fromLatin1("fr-ch"))
            d->language = fr_ch;
        else if (elemText == QString::fromLatin1("gl"))
            d->language = gl;
        else if (elemText == QString::fromLatin1("gd"))
            d->language = gd;
        else if (elemText == QString::fromLatin1("de"))
            d->language = de;
        else if (elemText == QString::fromLatin1("de-at"))
            d->language = de_at;
        else if (elemText == QString::fromLatin1("de-de"))
            d->language = de_de;
        else if (elemText == QString::fromLatin1("de-li"))
            d->language = de_li;
        else if (elemText == QString::fromLatin1("de-lu"))
            d->language = de_lu;
        else if (elemText == QString::fromLatin1("de-ch"))
            d->language = de_ch;
        else if (elemText == QString::fromLatin1("el"))
            d->language = el;
        else if (elemText == QString::fromLatin1("hu"))
            d->language = hu;
        else if (elemText == QString::fromLatin1("is"))
            d->language = is;
        else if (elemText == QString::fromLatin1("id"))
            d->language = id;
        else if (elemText == QString::fromLatin1("ga"))
            d->language = ga;
        else if (elemText == QString::fromLatin1("it"))
            d->language = it;
        else if (elemText == QString::fromLatin1("it-it"))
            d->language = it_it;
        else if (elemText == QString::fromLatin1("it-ch"))
            d->language = it_ch;
        else if (elemText == QString::fromLatin1("ja"))
            d->language = ja;
        else if (elemText == QString::fromLatin1("ko"))
            d->language = ko;
        else if (elemText == QString::fromLatin1("mk"))
            d->language = mk;
        else if (elemText == QString::fromLatin1("no"))
            d->language = no;
        else if (elemText == QString::fromLatin1("pl"))
            d->language = pl;
        else if (elemText == QString::fromLatin1("pt"))
            d->language = pt;
        else if (elemText == QString::fromLatin1("pt-br"))
            d->language = pt_br;
        else if (elemText == QString::fromLatin1("pt-pt"))
            d->language = pt_pt;
        else if (elemText == QString::fromLatin1("ro"))
            d->language = ro;
        else if (elemText == QString::fromLatin1("ro-mo"))
            d->language = ro_mo;
        else if (elemText == QString::fromLatin1("ro-ro"))
            d->language = ro_ro;
        else if (elemText == QString::fromLatin1("ru"))
            d->language = ru;
        else if (elemText == QString::fromLatin1("ru-mo"))
            d->language = ru_mo;
        else if (elemText == QString::fromLatin1("ru-ru"))
            d->language = ru_ru;
        else if (elemText == QString::fromLatin1("sr"))
            d->language = sr;
        else if (elemText == QString::fromLatin1("sk"))
            d->language = sk;
        else if (elemText == QString::fromLatin1("sl"))
            d->language = sl;
        else if (elemText == QString::fromLatin1("es"))
            d->language = es;
        else if (elemText == QString::fromLatin1("es-ar"))
            d->language = es_ar;
        else if (elemText == QString::fromLatin1("es-bo"))
            d->language = es_bo;
        else if (elemText == QString::fromLatin1("es-cl"))
            d->language = es_cl;
        else if (elemText == QString::fromLatin1("es-co"))
            d->language = es_co;
        else if (elemText == QString::fromLatin1("es-cr"))
            d->language = es_cr;
        else if (elemText == QString::fromLatin1("es-do"))
            d->language = es_do;
        else if (elemText == QString::fromLatin1("es-ec"))
            d->language = es_ec;
        else if (elemText == QString::fromLatin1("es-sv"))
            d->language = es_sv;
        else if (elemText == QString::fromLatin1("es-gt"))
            d->language = es_gt;
        else if (elemText == QString::fromLatin1("es-hn"))
            d->language = es_hn;
        else if (elemText == QString::fromLatin1("es-mx"))
            d->language = es_mx;
        else if (elemText == QString::fromLatin1("es-ni"))
            d->language = es_ni;
        else if (elemText == QString::fromLatin1("es-pa"))
            d->language = es_pa;
        else if (elemText == QString::fromLatin1("es-py"))
            d->language = es_py;
        else if (elemText == QString::fromLatin1("es-pe"))
            d->language = es_pe;
        else if (elemText == QString::fromLatin1("es-pr"))
            d->language = es_pr;
        else if (elemText == QString::fromLatin1("es-es"))
            d->language = es_es;
        else if (elemText == QString::fromLatin1("es-uy"))
            d->language = es_uy;
        else if (elemText == QString::fromLatin1("es-ve"))
            d->language = es_ve;
        else if (elemText == QString::fromLatin1("sv"))
            d->language = sv;
        else if (elemText == QString::fromLatin1("sv-fi"))
            d->language = sv_fi;
        else if (elemText == QString::fromLatin1("sv-se"))
            d->language = sv_se;
        else if (elemText == QString::fromLatin1("tr"))
            d->language = tr;
        else if (elemText == QString::fromLatin1("uk"))
            d->language = uk;
        else
            d->language = UndefinedLanguage;
    }

    if (d->format == AtomFeed)
        tagName=QString::fromLatin1("issued"); // atom doesn't specify this for feeds
                                               // but some broken feeds do this
    else
        tagName=QString::fromLatin1("pubDate");

    if (!(elemText = extractNode(channelNode, tagName)).isNull()) {
        time_t _time;

        if (d->format == AtomFeed)
            _time=parseISO8601Date(elemText);
        else
            _time=KRFCDate::parseDate(elemText);
        /* \bug This isn't really the right way since it will set the date to
         * Jan 1 1970, 1:00:00 if the passed date was invalid; this means that
         * we cannot distinguish between that date, and invalid values. :-/
         */
        d->pubDate.setTime_t(_time);
    }

    if (!(elemText = extractNode(channelNode, QString::fromLatin1("dc:date"))).isNull()) {
        time_t _time = parseISO8601Date(elemText);
        /* \bug This isn't really the right way since it will set the date to
         * Jan 1 1970, 1:00:00 if the passed date was invalid; this means that
         * we cannot distinguish between that date, and invalid values. :-/
         */
        d->pubDate.setTime_t(_time);
    }

    if (d->format == AtomFeed)
        tagName=QString::fromLatin1("modified");
    else
        tagName=QString::fromLatin1("lastBuildDate");
    if (!(elemText = extractNode(channelNode, tagName)).isNull()) {
        time_t _time;
        if (d->format == AtomFeed)
            _time = parseISO8601Date(elemText);
        else
            _time = KRFCDate::parseDate(elemText);
        d->lastBuildDate.setTime_t(_time);
    }

    if (!(elemText = extractNode(channelNode, QString::fromLatin1("rating"))).isNull())
        d->rating = elemText;
    if (!(elemText = extractNode(channelNode, QString::fromLatin1("docs"))).isNull())
        d->docs = elemText;
    if (!(elemText = extractNode(channelNode, QString::fromLatin1((d->format == AtomFeed) ? "author" : "managingEditor"))).isNull())
        d->managingEditor = elemText;
    if (!(elemText = extractNode(channelNode, QString::fromLatin1("webMaster"))).isNull())
        d->webMaster = elemText;

	if (!(elemText = extractNode(channelNode, QString::fromLatin1("ttl"))).isNull())
        d->ttl = elemText.toUInt();

    n = channelNode.namedItem(QString::fromLatin1("skipHours"));
    if (!n.isNull())
        for (QDomElement e = n.firstChild().toElement(); !e.isNull(); e = e.nextSibling().toElement())
            if (e.tagName() == QString::fromLatin1("hour"))
                d->skipHours.append(e.text().toUInt());

    n = channelNode.namedItem(QString::fromLatin1("skipDays"));
    if (!n.isNull()) {
        Day day;
        QString elemText;
        for (QDomElement e = n.firstChild().toElement(); !e.isNull(); e = e.nextSibling().toElement())
            if (e.tagName() == QString::fromLatin1("day")) {
                elemText = e.text().lower();
                if (elemText == QString::fromLatin1("monday"))
                    day = Monday;
                else if (elemText == QString::fromLatin1("tuesday"))
                    day = Tuesday;
                else if (elemText == QString::fromLatin1("wednesday"))
                    day = Wednesday;
                else if (elemText == QString::fromLatin1("thursday"))
                    day = Thursday;
                else if (elemText == QString::fromLatin1("friday"))
                    day = Friday;
                else if (elemText == QString::fromLatin1("saturday"))
                    day = Saturday;
                else if (elemText == QString::fromLatin1("sunday"))
                    day = Sunday;
                else
                    day = UndefinedDay;
                if (day != UndefinedDay)
                    d->skipDays.append(day);
            }
    }
}

Document::~Document()
{
    if (d->deref())
        delete d;
}

bool Document::isValid() const
{
    return d->valid;
}

Version Document::version() const
{
    return d->version;
}

QString Document::verbVersion() const
{
    switch (d->version) {
        case v0_90: return QString::fromLatin1("0.90");
        case v0_91: return QString::fromLatin1("0.91");
        case v0_92: return QString::fromLatin1("0.92");
        case v0_93: return QString::fromLatin1("0.93");
        case v0_94: return QString::fromLatin1("0.94");
        case v1_0: return QString::fromLatin1("1.0");
        case v2_0: return QString::fromLatin1("2.0");
        case vAtom_0_3: return QString::fromLatin1("0.3");
        case vAtom_0_2: return QString::fromLatin1("0.2");
        case vAtom_0_1: return QString::fromLatin1("0.1");
    }
    return QString::null;
}

QString Document::title() const
{
    return d->title;
}

QString Document::description() const
{
    return d->description;
}

const KURL &Document::link() const
{
    return d->link;
}

Image *Document::image()
{
    return d->image;
}

const Image *Document::image() const
{
    return d->image;
}

TextInput *Document::textInput()
{
    return d->textInput;
}

const TextInput *Document::textInput() const
{
    return d->textInput;
}

const Article::List &Document::articles() const
{
    return d->articles;
}

Language Document::language() const
{
    return d->language;
}

QString Document::copyright() const
{
    return d->copyright;
}

const QDateTime &Document::pubDate() const
{
    return d->pubDate;
}

const QDateTime &Document::lastBuildDate() const
{
    return d->lastBuildDate;
}

QString Document::rating() const
{
    return d->rating;
}

const KURL &Document::docs() const
{
    return d->docs;
}

QString Document::managingEditor() const
{
    return d->managingEditor;
}

QString Document::webMaster() const
{
    return d->webMaster;
}

const HourList &Document::skipHours() const
{
    return d->skipHours;
}

const DayList &Document::skipDays() const
{
    return d->skipDays;
}

int Document::ttl() const
{
    return d->ttl;
}

Document &Document::operator=(const Document &other)
{
    if (this != &other) {
        other.d->ref();
        if (d && d->deref())
            delete d;
        d = other.d;
    }
    return *this;
}

// vim:noet:ts=4
