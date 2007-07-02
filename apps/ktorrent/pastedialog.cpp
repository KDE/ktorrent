/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#include "pastedialog.h"
#include "ktorrentcore.h"
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <qclipboard.h>
#include <qapplication.h>
#include <kurl.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <klocale.h>

PasteDialog::PasteDialog(KTorrentCore* core, QWidget *parent, const char *name)
		:PasteDlgBase(parent, name)
{
	m_core = core;
	QClipboard *cb = QApplication::clipboard();
	QString text = cb->text(QClipboard::Clipboard);
	KURL url = KURL::fromPathOrURL(text);
	if ( url.isValid() )
		m_url->setText(url.url());
	
	btnOK->setGuiItem(KStdGuiItem::ok());
	btnCancel->setGuiItem(KStdGuiItem::cancel());
}

void PasteDialog::btnOK_clicked()
{
	KURL url = KURL::fromPathOrURL(m_url->text());
	if ( url.isValid() )
	{
		m_core->load(url);
		QDialog::accept();
	}
	else
	{
		KMessageBox::error(this,i18n("Malformed URL."));
	}
}



#include "pastedialog.moc"
