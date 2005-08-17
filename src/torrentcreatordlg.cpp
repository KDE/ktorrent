//
// C++ Implementation: $MODULE$
//
// Description: 
//
//
// Author: Joris Guisson <joris.guisson@gmail.com>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <qcheckbox.h>
#include <qstringlist.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <keditlistbox.h>
#include <kpushbutton.h>
#include <kfiledialog.h>
#include <kprogress.h>
#include "torrentcreatordlg.h"
#include "ktorrentcore.h"

TorrentCreatorDlg::TorrentCreatorDlg(KTorrentCore* core,QWidget *parent, const char *name)
	:TorrentCreatorDlgBase(parent, name),core(core)
{
	KURLRequester* r = m_file_or_dir;
	r->fileDialog()->setMode(
			KFile::ExistingOnly|KFile::Directory|KFile::File|KFile::LocalOnly);

	KComboBox* cb = m_chunk_size;
	cb->setCurrentItem(3);
	
	connect(m_file_or_dir,SIGNAL(urlSelected(const QString& )),
			this,SLOT(onURLChanged(const QString& )));
	connect(m_create_btn,SIGNAL(clicked()),this,SLOT(onCreate()));
	connect(m_cancel_btn,SIGNAL(clicked()),this,SLOT(reject()));
}

TorrentCreatorDlg::~TorrentCreatorDlg()
{
}

void TorrentCreatorDlg::onCreate()
{
	KURLRequester* r = m_file_or_dir;
	KComboBox* cb = m_chunk_size;
	KEditListBox* eb = m_trackers;
	
	if (r->url().length() == 0)
	{
		errorMsg(i18n("You must select a file or a directory."));
		return;
	}

	if (m_name->text().length() == 0)
	{
		errorMsg(i18n("You must fill in the name field."));
		return;
	}

	if (eb->items().count() == 0)
	{
		errorMsg(i18n("You must add at least one tracker."));
		return;
	}

	QString url = r->url();
	int chunk_size = cb->currentText().toInt();
	QString name = m_name->text();
	QStringList trackers = eb->items();

	QString s = KFileDialog::getSaveFileName(
			QString::null,"*.torrent|" + i18n("Torrent Files (*.torrent)"),
			0,i18n("Choose File to Save Torrent"));

	if (s.isNull())
		return;

	KProgressDialog* dlg = new KProgressDialog(this,0);
	dlg->setLabel(i18n("Creating %1...").arg(s));
	dlg->setModal(true);
	dlg->setAllowCancel(false);
	dlg->show();
	core->makeTorrent(
			url,trackers,chunk_size,
			name,m_comments->text(),
			m_start_seeding->isChecked(),s,
			dlg->progressBar());
	delete dlg;
	accept();
}

void TorrentCreatorDlg::onURLChanged(const QString & str)
{
	KURL url(str);
	if (m_name->text().length() == 0)
		m_name->setText(url.fileName());
}

void TorrentCreatorDlg::errorMsg(const QString & text)
{
	KMessageBox::error(this,text,i18n("Error"));
}

#include "torrentcreatordlg.moc"
