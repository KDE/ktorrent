/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <klocale.h>
#include "managefiltersdlg.h"

namespace kt
{

	ManageFiltersDlg::ManageFiltersDlg(QWidget* parent) : KDialog(parent)
	{
		setWindowTitle(i18n("Manage Filters"));
		setupUi(mainWidget());
		m_add->setIcon(KIcon("go-previous"));
		m_add->setText(QString());
		m_remove->setIcon(KIcon("go-next"));
		m_remove->setText(QString());
		connect(m_add,SIGNAL(clicked()),this,SLOT(add()));
		connect(m_remove,SIGNAL(clicked()),this,SLOT(remove()));
		connect(m_remove_all,SIGNAL(clicked()),this,SLOT(removeAll()));
		connect(m_new_filter,SIGNAL(clicked()),this,SLOT(newFilter()));
	}


	ManageFiltersDlg::~ManageFiltersDlg()
	{
	}

	void ManageFiltersDlg::add()
	{}
	
	void ManageFiltersDlg::remove()
	{}
	
	void ManageFiltersDlg::removeAll()
	{}
	
	void ManageFiltersDlg::newFilter()
	{}
}
