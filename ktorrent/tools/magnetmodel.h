/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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



#ifndef KT_MAGNETMODEL_H
#define KT_MAGNETMODEL_H

#include <QAbstractTableModel>
#include <interfaces/coreinterface.h>
#include <magnet/magnetdownloader.h>


namespace kt
{

	/**
	 * Adds options struct to bt::MagnetDownloader
	 */
	class MagnetDownloader : public bt::MagnetDownloader
	{
		Q_OBJECT
	public:
		MagnetDownloader(const bt::MagnetLink& mlink, const MagnetLinkLoadOptions & options, QObject* parent) :
				bt::MagnetDownloader(mlink, parent),
				options(options)
		{}

		virtual ~MagnetDownloader()
		{}

		MagnetLinkLoadOptions options;
	};


	/**
		Model which keeps track of MagnetDownloaders
	*/
	class MagnetModel : public QAbstractTableModel
	{
		Q_OBJECT
	public:
		MagnetModel(QObject* parent = 0);
		virtual ~MagnetModel();

		/**
			Start a magnet download
		*/
		void download(const bt::MagnetLink & mlink, const MagnetLinkLoadOptions & options);

		virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
		virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
		virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
		virtual bool removeRows(int row, int count, const QModelIndex & parent);
		virtual bool insertRows(int row, int count, const QModelIndex & parent);

		/// Update the MagnetDownloaders
		void updateMagnetDownloaders();

		/// Remove a MagnetDownloader
		void removeMagnetDownloader(kt::MagnetDownloader* md);

		/// Start a magnet downloader
		void start(const QModelIndex & idx);

		/// Stop a magnet downloader
		void stop(const QModelIndex & idx);

		/// Save current magnet links to a file
		void saveMagnets(const QString & file);

		/// Load magnets from file
		void loadMagnets(const QString & file);

	private slots:
		void downloadFinished(bt::MagnetDownloader* md, const QByteArray & data);

	signals:
		/// Emitted when metadata has been found for a MagnetLink
		void metadataFound(const bt::MagnetLink & mlink, const QByteArray & data, const kt::MagnetLinkLoadOptions & options);

	private:
		QString displayName(const bt::MagnetDownloader* md) const;
		QString status(const bt::MagnetDownloader* md) const;
		void addMagnetDownloader(const bt::MagnetLink & mlink, const MagnetLinkLoadOptions & options, bool start);

	private:
		QList<kt::MagnetDownloader*> magnet_downloaders;
	};

}

#endif // KT_MAGNETMODEL_H
