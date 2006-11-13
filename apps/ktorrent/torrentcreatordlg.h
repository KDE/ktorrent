//
// C++ Interface: $MODULE$
//
// Description: 
//
//
// Author: Joris Guisson <joris.guisson@gmail.com>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TORENTCREATORDLG_H
#define TORENTCREATORDLG_H

#include "torrentcreatordlgbase.h"

class KTorrentCore;

class TorrentCreatorDlg: public TorrentCreatorDlgBase
{
	Q_OBJECT
public:
	TorrentCreatorDlg(KTorrentCore* core,QWidget *parent = 0, const char *name = 0);
	virtual ~TorrentCreatorDlg();

public slots:
	void onCreate();
    virtual void btnAddNode_clicked();
    virtual void btnRemoveNode_clicked();
    virtual void m_nodeList_selectionChanged(QListViewItem*);
    virtual void m_node_textChanged(const QString&);
private:
	void errorMsg(const QString & text);

private:
	KTorrentCore* core;
};

#endif
