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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef TRAYHOVERPOPUP_H
#define TRAYHOVERPOPUP_H

#include <qtimer.h>
#include <kpassivepopup.h>
		
class QLabel;
class QPixmap;

/**
	@author Joris Guisson <joris.guisson@gmail.com>
		
	This is the passive popup which is shown when the mouse cursor is hovered over the tray icon
*/
class TrayHoverPopup : public KPassivePopup
{
	Q_OBJECT
public:
	TrayHoverPopup(const QPixmap & pix,QWidget *parent = 0, const char *name = 0 );
	virtual ~TrayHoverPopup();
	
	/// Cursor entered system tray icon
	void enterEvent();
	
	/// Cursor left system tray icon
	void leaveEvent();
	
	/// Update the text which is shown
	void updateText(const QString & msg);
	
public slots:
	void contextMenuAboutToShow();
	void contextMenuAboutToHide();

private:
	void create();
	
private slots:
	void onHoverTimeout();
	void onShowTimeout();
	
	
private:
	const QPixmap & pix;
	QTimer hover_timer;
	QTimer show_timer;
	QLabel* text;
	bool context_menu_shown;
	bool cursor_over_icon;
};

#endif
