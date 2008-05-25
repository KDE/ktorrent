/*
 * image.h
 *
 * Copyright (c) 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef LIBRSS_IMAGE_H
#define LIBRSS_IMAGE_H

#include "global.h"

#include <qobject.h>

class QDomNode;

namespace KIO
{
   class Job;
}
class KURL;

namespace RSS
{
   /**
    * Represents an image as stored in a RSS file. You don't have to
    * instantiate one of these yourself, the common way to access instances
    * is via Document::image().
    * @see Document::image()
    */
   class Image : public QObject
   {
      Q_OBJECT
      public:
         /**
          * Default constructor.
          */
         Image();

         /**
          * Copy constructor.
          * @param other The Image object to copy.
          */
         Image(const Image &other);

         /**
          * Constructs an Image from a piece of RSS markup.
          * @param node A QDomNode which references the DOM leaf to be used
          * for constructing the Image.
          */
         Image(const QDomNode &node);

         /**
          * Assignment operator.
          * @param other The Image object to clone.
          * @return A reference to the cloned Image object.
          */
         Image &operator=(const Image &other);

         /**
          * Compares two images. Two images are considered identical if
          * their properties (title, description, link etc.) are identical.
          * Note that this does not include the actual pixmap data!
          * @param other The image to compare with.
          * @return Whether the two images are equal.
          */
         bool operator==(const Image &other) const;

         /**
          * Convenience method. Simply calls !operator==().
          * @param other The image to compared with.
          * @return Whether the two images are unequal.
          */
         bool operator!=(const Image &other) const { return !operator==(other); }

         /**
          * Destructor.
          */
         virtual ~Image();

         /**
          * RSS 0.90 and upwards
          * @return The 'caption' of this image, or QString::null if no
          * caption is available.
          */
         QString title() const;

         /**
          * RSS 0.90 and upwards
          * @return The URL pointing to the file containing the graphic
          * data (GIF, JPEG or PNG format), or an empty KURL if no URL
          * is available. You can use getPixmap() and gotPixmap() to have
          * the Image download the pixmap data itself.
          * Note that the RSS 0.91 Specification dictates that URLs not
          * starting with "http://" or "ftp://" are considered invalid.
          */
         const KURL &url() const;

         /**
          * RSS 0.90 and upwards
          * @return A link to some resource, or an empty KURL of no link is
          * available. Clicking on the image should lead the user to the
          * resource referenced by this URL.
          * Note that the RSS 0.91 Specification dictates that URLs not
          * starting with "http://" or "ftp://" are considered invalid.
          */
         const KURL &link() const;

         /**
          * RSS 0.91 and upwards
          * @return A description of what this picture shows, or
          * QString::null if no description is available. Useful for
          * people who deactivated images but want or need to know what is
          * shown.
          */
         QString description() const;

         /**
          * RSS 0.91 and upwards
          * @return The height in pixels as reported by the news site, the
          * default value is 31 pixels. The RSS 0.91 Specification requires
          * this value to be between 1 and 400.
          * '0' if this information isn't available. This is merely provided
          * for completeness, you should not rely on this value but rather
          * check what height the QPixmap as returned by gotPixmap()
          * reports.
          */
         unsigned int height() const;

         /**
          * RSS 0.91 and upwards
          * @return The width in pixels as reported by the news site, the
          * default value is 88 pixels. The RSS 0.91 Specification requires
          * this value to be between 1 and 144.
          * This is merely provided for completeness, you should not rely
          * on this value but rather check what width the QPixmap as
          * returned by gotPixmap() reports.
          */
         unsigned int width() const;

         /**
          * Makes the image download the image data as referenced by the
          * URL returned by url(). You have to connect to the signal
          * gotPixmap() first and then call getPixmap().
          */
         void getPixmap();
		 void abort();

      signals:
         /**
          * Emitted when this Image is done downloading the actual graphics
          * data as referenced by the URL returned by url(). You can trigger
          * this download by calling getPixmap().
          * @param pixmap The pixmap as constructed from the data referenced
          * by the URL returned by link().
          */
         void gotPixmap(const QPixmap &pixmap);

      private slots:
         void slotData(KIO::Job *job, const QByteArray &data);
         void slotResult(KIO::Job *job);

      private:
         struct Private;
         Private *d;
   };
}

#endif // LIBRSS_IMAGE_H
// vim: noet:ts=4
