#!/usr/bin/python
import KTorrent
import Kross

def torrentAdded(ih):
	tor = KTorrent.torrent(ih)
	KTorrent.log("torrentAdded=%s" % tor.name())

def torrentRemoved(ih):
	tor = KTorrent.torrent(ih)
	KTorrent.log("torrentRemoved=%s" % tor.name())

 
KTorrent.connect("torrentAdded(const QString &)",torrentAdded)
KTorrent.connect("torrentRemoved(const QString &)",torrentRemoved)

tors = KTorrent.torrents()

KTorrent.log("Num torrents : %i" % len(tors))
for t in tors:
	tor = KTorrent.torrent(t)
	KTorrent.log("Torrent %s = %s" % (t, tor.name()))
	for i in range(0,tor.numFiles()):
		KTorrent.log("File %i = %s" % (i,tor.filePath(i)))