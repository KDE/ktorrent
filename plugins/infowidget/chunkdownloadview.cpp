#include <QHeaderView>
#include <klocale.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include <interfaces/chunkdownloadinterface.h>
#include <util/functions.h>
#include "chunkdownloadview.h"

using namespace bt;

namespace kt
{
	ChunkDownloadViewItem::ChunkDownloadViewItem(QTreeWidget* cdv,ChunkDownloadInterface* cd,bt::TorrentInterface* tc) 
		: QTreeWidgetItem(cdv,QTreeWidgetItem::UserType),cd(cd)
	{
		cd->getStats(stats);
		setText(0,QString::number(stats.chunk_index));
		update(true);
		QString files;
		if (tc->getStats().multi_file_torrent)
		{
			int n = 0;
			for (Uint32 i = 0;i < tc->getNumFiles();i++)
			{
				const bt::TorrentFileInterface & tf = tc->getTorrentFile(i);
				if (stats.chunk_index >= tf.getFirstChunk() && stats.chunk_index <= tf.getLastChunk())
				{
					if (n > 0)
						files += "\n";
					
					files += tf.getPath();
					n++;
				}
			}
			setText(5,files);
		}
	}

	ChunkDownloadViewItem::~ChunkDownloadViewItem()
	{
	}
	
	bool ChunkDownloadViewItem::operator < (const QTreeWidgetItem & other) const
	{
		const ChunkDownloadViewItem & cdvi = (const ChunkDownloadViewItem &) other;
		const ChunkDownloadInterface::Stats & s = stats;
		const ChunkDownloadInterface::Stats & os = cdvi.stats;
		switch (treeWidget()->sortColumn())
		{
		case 0: return s.chunk_index < os.chunk_index;
		case 1: return s.pieces_downloaded < os.pieces_downloaded;
		case 2: return s.current_peer_id < os.current_peer_id;
		case 3: return s.download_speed < os.download_speed;
		case 4: return s.num_downloaders < os.num_downloaders;
		case 5: return text(5) < other.text(5);
		default:
			 return false;
		}
	}


	void ChunkDownloadViewItem::update(bool init)
	{
		ChunkDownloadInterface::Stats s;
		cd->getStats(s);

		if (init || s.pieces_downloaded != stats.pieces_downloaded)
			setText(1,QString("%1 / %2").arg(s.pieces_downloaded).arg(s.total_pieces));
		
		if (init || s.download_speed != stats.download_speed)
			setText(3,KBytesPerSecToString(s.download_speed / 1024.0));

		if (init || s.num_downloaders != stats.num_downloaders)
			setText(4,QString::number(s.num_downloaders));

		if (init || s.current_peer_id != stats.current_peer_id)
			setText(2,s.current_peer_id);

		stats = s;
	}

	ChunkDownloadView::ChunkDownloadView(QWidget* parent) : QWidget(parent),curr_tc(0)
	{
		setupUi(this);
		m_chunk_view->setRootIsDecorated(false);
		m_chunk_view->setSortingEnabled(true);
		m_chunk_view->setAlternatingRowColors(true);
	}

	ChunkDownloadView::~ChunkDownloadView()
	{
	}

	void ChunkDownloadView::downloadAdded(ChunkDownloadInterface* cd)
	{
		items.insert(cd, new ChunkDownloadViewItem(m_chunk_view,cd,curr_tc));
	}

	void ChunkDownloadView::downloadRemoved(ChunkDownloadInterface* cd)
	{
		ChunkDownloadViewItem* v = items.find(cd);
		if (v)
		{
			items.erase(cd);
			delete v;
		}
	}

	void ChunkDownloadView::update()
	{
		if (!curr_tc)
			return;

		bt::PtrMap<ChunkDownloadInterface*,ChunkDownloadViewItem>::iterator i = items.begin();
		while (i != items.end())
		{
			i->second->update(false);
			i++;
		}

		const TorrentStats & s = curr_tc->getStats();
		m_chunks_downloading->setText(QString::number(s.num_chunks_downloading));
		m_chunks_downloaded->setText(QString::number(s.num_chunks_downloaded));
		m_excluded_chunks->setText(QString::number(s.num_chunks_excluded));
		m_chunks_left->setText(QString::number(s.num_chunks_left));
	}

	void ChunkDownloadView::changeTC(TorrentInterface* tc)
	{
		curr_tc = tc;
		if (!curr_tc)
		{
			setEnabled(false);
		}
		else
		{
			setEnabled(true);
			const TorrentStats & s = curr_tc->getStats();
			m_total_chunks->setText(QString::number(s.total_chunks));
			m_size_chunks->setText(BytesToString(s.chunk_size));
		}
	}

	void ChunkDownloadView::removeAll()
	{
		items.clear();
		m_chunk_view->clear();
	}

	void ChunkDownloadView::saveState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("ChunkDownloadView");
		QByteArray s = m_chunk_view->header()->saveState();
		g.writeEntry("state",s.toBase64());
	}
	
	void ChunkDownloadView::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("ChunkDownloadView");
		QByteArray s = QByteArray::fromBase64(g.readEntry("state",QByteArray()));
		if (!s.isNull())
			m_chunk_view->header()->restoreState(s);
	}
}

#include "chunkdownloadview.moc"
