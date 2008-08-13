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
#include "core.h"
#include <QClipboard>
#include <QApplication>
#include <KUrl>
#include <KLineEdit>
#include <KMessageBox>
#include <KLocale>
#include <KStandardGuiItem>

namespace kt
{
	PasteDialog::PasteDialog ( Core* core, QWidget* parent, Qt::WFlags fl )
			:KDialog ( parent, fl )
	{
		setupUi ( mainWidget() );
		
		m_core = core;
		QClipboard *cb = QApplication::clipboard();
		QString text = cb->text ( QClipboard::Clipboard );

		KUrl url = KUrl(text);

		if ( url.isValid() )
			m_url->setText ( text );
	}

	void PasteDialog::accept()
	{
		KUrl url = KUrl( m_url->text() );
		if ( url.isValid() )
		{
			m_core->load(url,QString());
			QDialog::accept();
		}
		else
		{
			KMessageBox::error(this,i18n("Invalid URL: ",m_url->text()) );
		}
	}
}

#include "pastedialog.moc"
