/***************************************************************************
 *   Copyright (C) 2007 by Lukasz Fibinger <lucke@o2.pl>                   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "filterbar.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include <kdialog.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <ktoolbarbutton.h>
#include <klineedit.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <interfaces/torrentinterface.h>



FilterBar::FilterBar(QWidget *parent, const char *name) :
    QWidget(parent, name)
{
	const int gap = 3;
	
	QVBoxLayout* foo = new QVBoxLayout(this);
	foo->addSpacing(gap);
	
	QHBoxLayout* layout = new QHBoxLayout(foo);
	layout->addSpacing(gap);
	
	m_close = new KToolBarButton("fileclose",0,this);
	connect(m_close,SIGNAL(clicked()),this,SLOT(hide()));
	layout->addWidget(m_close);
	
	m_filter = new QLabel(i18n("Find:"), this);
	layout->addWidget(m_filter);
	layout->addSpacing(KDialog::spacingHint());
	
	m_filterInput = new KLineEdit(this);
	layout->addWidget(m_filterInput);
	
	m_clear = new KPushButton(this);
	m_clear->setIconSet(SmallIcon("clear_left"));
	m_clear->setFlat(true);
	layout->addWidget(m_clear);
	layout->addSpacing(gap);
	
	m_case_sensitive = new QCheckBox(i18n("Case sensitive"),this);
	m_case_sensitive->setChecked(true);
	layout->addWidget(m_case_sensitive);
	layout->addItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
		
	connect(m_filterInput, SIGNAL(textChanged(const QString&)),
		this, SLOT(slotChangeFilter(const QString&)));
	connect(m_clear, SIGNAL(clicked()), m_filterInput, SLOT(clear()));
}

FilterBar::~FilterBar()
{
}

void FilterBar::saveSettings(KConfig* cfg)
{
	cfg->writeEntry("filter_bar_hidden",isHidden());
	cfg->writeEntry("filter_bar_text",m_name_filter);
	cfg->writeEntry("filter_bar_case_sensitive",m_case_sensitive->isChecked());
}
	
void FilterBar::loadSettings(KConfig* cfg)
{
	setHidden(cfg->readBoolEntry("filter_bar_hidden",true));
	m_case_sensitive->setChecked(cfg->readBoolEntry("filter_bar_case_sensitive",true));
	m_name_filter = cfg->readEntry("filter_bar_text",QString::null);
	m_filterInput->setText(m_name_filter);
}

bool FilterBar::matchesFilter(kt::TorrentInterface* tc)
{
	bool cs = m_case_sensitive->isChecked();
	if (m_name_filter.length() == 0)
		 return true;
	else
		return tc->getStats().torrent_name.contains(m_name_filter,cs);
}

void FilterBar::slotChangeFilter(const QString& nameFilter)
{	
	m_name_filter = nameFilter;
}

void FilterBar::keyPressEvent(QKeyEvent* event)
{
    if ((event->key() == Qt::Key_Escape))
	{
        m_filterInput->clear();
		m_name_filter = QString::null;
		//hide();
    }
	else 
		QWidget::keyPressEvent(event);
}

void FilterBar::hideEvent(QHideEvent* event)
{
	m_filterInput->releaseKeyboard();
	QWidget::hideEvent(event);
}

#include "filterbar.moc"
