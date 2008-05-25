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
#include <qvbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qpixmap.h>
#include <kdialog.h>
#include <util/log.h>
#include "trayhoverpopup.h"

using namespace bt;

TrayHoverPopup::TrayHoverPopup(const QPixmap & pix,QWidget *parent, const char *name )
	: KPassivePopup(KPassivePopup::Boxed,parent,name),pix(pix)
{
	setTimeout(0);
	setAutoDelete(false);
	connect(&hover_timer,SIGNAL(timeout()),this,SLOT(onHoverTimeout()));
	connect(&show_timer,SIGNAL(timeout()),this,SLOT(onShowTimeout()));
	create();
	setPalette(QToolTip::palette());
	setLineWidth(1);
	context_menu_shown = false;
	cursor_over_icon = false;
}


TrayHoverPopup::~TrayHoverPopup()
{}

void TrayHoverPopup::contextMenuAboutToShow()
{
	context_menu_shown = true;
	if (isShown())
	{
		hide();
		hover_timer.stop();
	}
}

void TrayHoverPopup::contextMenuAboutToHide()
{
	context_menu_shown = false;
}
			

void TrayHoverPopup::enterEvent()
{
	cursor_over_icon = true;
	if (isHidden() && !context_menu_shown)
	{
		// start the show timer
		show_timer.start(1000,true);
	}
	else
		hover_timer.stop(); // stop timeout
}

void TrayHoverPopup::leaveEvent()
{
	cursor_over_icon = false;
	// to avoid problems with a quick succession of enter and leave events, because the cursor
	// is on the edge, use a timer to expire the popup
	// in enterEvent we will stop the timer
	if (isShown())
		hover_timer.start(200,true);
}

void TrayHoverPopup::onHoverTimeout()
{
	hide();
	show_timer.stop();
}

void TrayHoverPopup::onShowTimeout()
{
	if (!context_menu_shown && cursor_over_icon)
		show();
}

void TrayHoverPopup::updateText(const QString & msg)
{
	text->setText(msg);
}

void TrayHoverPopup::create()
{
	QVBox *vb = new QVBox(this);
	vb->setSpacing(KDialog::spacingHint());
	 
	QHBox *hb=0;
	if (!pix.isNull()) 
	{
		hb = new QHBox(vb);
		hb->setMargin(0);
		hb->setSpacing(KDialog::spacingHint());
		QLabel* pix_lbl = new QLabel(hb,"title_icon");
		pix_lbl->setPixmap(pix);
		pix_lbl->setAlignment(AlignLeft);
	}
	 
	
	QLabel* title = new QLabel("KTorrent", hb ? hb : vb, "title_label" );
	QFont fnt = title->font();
	fnt.setBold( true );
	title->setFont( fnt );
	title->setAlignment( Qt::AlignHCenter );
	if ( hb )
		hb->setStretchFactor(title, 10 ); // enforce centering

	// text will be filled later
	text = new QLabel( "Dummy", vb, "msg_label" );
				text->setAlignment( AlignLeft );
	setView(vb);
}
	

#include "trayhoverpopup.moc"
