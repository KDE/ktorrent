#!/usr/bin/env kf5kross
# -*- coding: utf-8 -*-
import KTorrent
import KTScriptingPlugin
import Kross
from urlparse import urlparse

t = Kross.module("kdetranslation")

def fqdn(tracker):
	netloc = urlparse(tracker).netloc
	if ':' in netloc:
		netloc = netloc[:netloc.find(':')]
	return netloc

class TrackerGroup:
	def __init__(self,url):
		self.url = url
		self.ref_count = 0
		self.cache = {}
		
	def isMember(self,info_hash):
		if info_hash in self.cache:
			return self.cache[info_hash]
		else:
			tor = KTorrent.torrent(info_hash)
			ret = self.url in [fqdn(tracker) for tracker in tor.trackers()]
			self.cache[info_hash] = ret
			return ret
			
	
	def ref(self):
		self.ref_count += 1
		
	def unref(self):
		if self.ref_count > 0:
			self.ref_count -= 1
			
	def canBeRemoved(self):
		return self.ref_count == 0


class TrackerGroupsScript:
	def __init__(self):
		self.tracker_map = {} # initialize the dictionary
		self.id = 0
		KTorrent.connect("torrentAdded(const QString &)",self.torrentAdded)
		KTorrent.connect("torrentRemoved(const QString &)",self.torrentRemoved)
		tors = KTorrent.torrents()
		# go over each torrent and collect all the trackers
		for t in tors:
			self.torrentAdded(t)
			
	def addTracker(self,tracker):
		g = TrackerGroup(tracker)
		g.ref()
		self.tracker_map[tracker] = g
		KTorrent.log("Adding group for tracker " + tracker)
		KTScriptingPlugin.addGroup(tracker,"network-server","/all/" + t.i18n("Trackers") + "/" + str(self.id),g)
		self.id += 1
			
	def removeTracker(self,tracker):
		g = self.tracker_map[fqdn(tracker)]
		g.unref()
		if g.canBeRemoved():
			KTorrent.log("Removing group for tracker " + tracker)
			KTScriptingPlugin.removeGroup(g.url)
			del self.tracker_map[tracker]
		
	def torrentAdded(self,info_hash):
		tor = KTorrent.torrent(info_hash)
		trackers = tor.trackers()
		for tracker in trackers:
			hostname = fqdn(tracker)
			if not hostname in self.tracker_map:
				self.addTracker(hostname)
			else:
				self.tracker_map[hostname].ref()
		
	def torrentRemoved(self,info_hash):
		tor = KTorrent.torrent(info_hash)
		trackers = tor.trackers()
		for tracker in trackers:
			if tracker in self.tracker_map:
				self.removeTracker(tracker)
				
	def unload(self):
		trackers = self.tracker_map.keys()
		for tracker in trackers:
			g = self.tracker_map[tracker]
			KTorrent.log("Removing group for tracker " + tracker)
			KTScriptingPlugin.removeGroup(g.url)
			del self.tracker_map[tracker]

s = TrackerGroupsScript()

# Called when script is unloaded
def unload():
	global s
	s.unload()
	del s
