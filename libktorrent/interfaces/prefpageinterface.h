/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef PREFPAGEINTERFACE_H
#define PREFPAGEINTERFACE_H

#include <qpixmap.h>

class QWidget;

namespace kt
{
	/**
	 * @author Ivan Vasic
	 * @brief Interface to add configuration dialog page.
	 * 
	 * This interface allows plugins and others to add their own pages in Configuration dialog
	 */
	class PrefPageInterface
	{
	public:
		/**
		 * Constructor, set the name, header and pixmap
		 * @param name 
		 * @param header 
		 * @param pix 
		 */
		PrefPageInterface(const QString & name,const QString & header,const QPixmap & pix);
		virtual ~PrefPageInterface();

		const QString& getItemName() { return itemName; }
		const QString& getHeader() { return header; }
		const QPixmap& getPixmap() { return pixmap; }

		/**
		 * Apply the changes that have been made in the
		 * pref page. If the settings the user gave isn't valid false should be returned.
		 * This will prevent the dialog from closing.
		 * @return true if the data validates, false otherwise
		 */
		virtual bool apply() = 0;
		
		/**
		 * Create the actual widget.
		 * @param parent The parent of the widget
		 */
		virtual void createWidget(QWidget* parent)=0;

		/**
		 * Update all data on the widget, gets called before
		 * the preference dialog gets shown.
		 */
		virtual void updateData() = 0;

		/// Delete the widget, gets called when the page gets removed.
		virtual void deleteWidget() = 0;
		
	private:
		///Used in IconList mode. You should prefer a pixmap with size 32x32 pixels
		QPixmap pixmap;
		///String used in the list or as tab item name.
		QString itemName;
		///Header text use in the list modes. Ignored in Tabbed mode. If empty, the item text is used instead.
		QString header;
	};
}
#endif

