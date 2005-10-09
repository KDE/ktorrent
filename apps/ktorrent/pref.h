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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/


#ifndef _KTORRENTPREF_H_
#define _KTORRENTPREF_H_

#include <kdialogbase.h>
#include <qframe.h>
#include <qptrlist.h> 
#include "downloadpref.h"
#include "generalpref.h"
#include "searchpref.h" 


class KTorrent;
class QListViewItem; 

class PrefPageOne : public DownloadPref
{
	Q_OBJECT
public:
	PrefPageOne(QWidget *parent = 0);
	
	void apply();
	bool checkPorts();
};

class PrefPageTwo : public GeneralPref
{
	Q_OBJECT
public:
	PrefPageTwo(QWidget *parent = 0);

	void apply();
private slots:
	void autosaveChecked(bool on);
};

class PrefPageThree : public SEPreferences 
{ 
    Q_OBJECT 
public: 
    PrefPageThree(QWidget *parent = 0); 
     
    void apply(); 
     
private slots: 
    void addClicked(); 
    void removeClicked(); 
    void addDefaultClicked(); 
    void removeAllClicked(); 
private: 
    void loadSearchEngines(); 
    void saveSearchEngines(); 
     
    QPtrList<QListViewItem> m_items; 
}; 
 
class KTorrentPreferences : public KDialogBase
{
	Q_OBJECT
public:
	KTorrentPreferences(KTorrent & ktor);

private:
	virtual void slotOk();
	virtual void slotApply();
	
private:
	KTorrent & ktor;
	PrefPageOne* page_one;
	PrefPageTwo* page_two;
	PrefPageThree* page_three; 
};





#endif // _KTORRENTPREF_H_
