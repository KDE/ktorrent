/*
 * article.h
 *
 * Copyright (c) 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef LIBRSS_ARTICLE_H
#define LIBRSS_ARTICLE_H

#include <qmap.h>

#include "global.h"

class QDateTime;
class QDomNode;
template <class> class QValueList;
class QString;
class QWidget;
class KURL;
class KURLLabel;

namespace RSS
{
   /**
    * Represents an article as stored in a RSS file. You don't have to
    * instantiate one of these yourself, the common way to access instances
    * is via Document::articles().
    * @see Document::articles()
    */
   class Article
   {
      public:
         /**
          * A list of articles.
          */
         typedef QValueList<Article> List;

         /**
          * Default constructor.
          */
         Article();

         /**
          * Copy constructor.
          * @param other The Article object to copy.
          */
         Article(const Article &other);

         /**
          * Constructs an Article from a piece of RSS markup.
          * @param node A QDomNode which references the DOM leaf to be used
          * for constructing the Article.
          */
         Article(const QDomNode &node, Format format);

         /**
          * Assignment operator.
          * @param other The Article object to clone.
          * @return A reference to the cloned Article object.
          */
         Article &operator=(const Article &other);

         /**
          * Compares two articles. Two articles are treated to be identical
          * if all their properties (title, link, description etc.) are
          * equal.
          * @param other The article this article should be compared with.
          * @return Whether the two articles are equal.
          */
         bool operator==(const Article &other) const;

         /**
          * Convenience method. Simply calls !operator==().
          * @param other The article this article should be compared with.
          * @return Whether the two articles are unequal.
          */
         bool operator!=(const Article &other) const { return !operator==(other); }

         /**
          * Destructor.
          */
         virtual ~Article();

         /**
          * RSS 0.90 and upwards
          * @return The headline of this article, or QString::null if
          * no headline was available.
          */
         QString title() const;

         /**
          * RSS 0.90 and upwards
          * @return A URL referencing the complete text for this article,
          * or an empty KURL if no link was available.
          * Note that the RSS 0.91 Specification dictates that URLs not
          * starting with "http://" or "ftp://" are considered invalid.
          */
         const KURL &link() const;

         /**
          * RSS 0.91 and upwards
          * @return A story synopsis, or QString::null if no description
          * was available.
          */
         QString description() const;

         /**
          * RSS 2.0 and upwards
          * @return An article GUID (globally unique identifier).
          */
         QString guid() const;

         /**
          * RSS 2.0 and upwards
          * @return If this article GUID is permalink. Has no meaning when guid() is QString::null.
          */
         bool guidIsPermaLink() const;

         /**
          * RSS 2.0 and upwards
          * @return The date when the article was published.
          */
         const QDateTime &pubDate() const;
         
		 const KURL &commentsLink() const;
		 int comments() const;

		 QString meta(const QString &key) const;
         
         /**
          * @param parent The parent widget for the KURLLabel.
          * @param name A name for the widget which will be used internally.
          * @return a widget (a KURLLabel in this case) for the Article.
          * This makes building a user-interface which contains the
          * information in this Article object more convenient.
          * The returned KURLLabel's caption will be the title(), clicking
          * on it will emit the URL link(), and it has a QToolTip attached
          * to it which displays the description() (in case it has one,
          * if there is no description, the URL which the label links to
          * will be used).
          * Note that you have to delete the KURLLabel object returned by
          * this method yourself.
          */
         KURLLabel *widget(QWidget *parent = 0, const char *name = 0) const;
         
         typedef QMap<QString, QString> MetaInfoMap;
         
      private:
         struct Private;
         Private *d;
   };
}

#endif // LIBRSS_ARTICLE_H
// vim: noet:ts=4
