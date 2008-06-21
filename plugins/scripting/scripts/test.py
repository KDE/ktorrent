#!/usr/bin/python
import KTorrent
import Kross

#def torrentAdded(torrent):
#	KTorrent.log("torrentAdded=%s" % torrent.getStats.torrent_name)

#def torrentRemoved(torrent):
#	KTorrent.log("torrentRemoved=%s" % torrent.getStats.torrent_name)

 
#KTCore.connect("torrentAdded(bt::TorrentInterface*)",torrentAdded)
#KTCore.connect("torrentRemoved(bt::TorrentInterface*)",torrentRemoved)
KTorrent.log("This is python code !!!!!!!!!!!!!!")
KTorrent.log("Num torrents = %i" % KTorrent.numTorrents())
for n in range(0,KTorrent.numTorrents()):
	tor = KTorrent.torrent(n)
	KTorrent.log("Torrent %i = %s" % (n, tor.torrentName()))