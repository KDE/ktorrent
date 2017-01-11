#!/usr/bin/env kf5kross
# -*- coding: utf-8 -*-
import KTorrent
import KTScriptingPlugin
import Kross
#from PyQt4 import QtCore

t = Kross.module("kdetranslation")

class AutoRemove:
	def __init__(self):
		self.auto_resume = False
		self.remove_on_finish_downloading = False
		self.remove_on_finish_seeding = False
		self.timer = KTScriptingPlugin.createTimer(True)
		KTorrent.connect("torrentAdded(const QString &)",self.torrentAdded)
		tors = KTorrent.torrents()
		# bind to signals for each torrent
		for t in tors:
			self.torrentAdded(t)
		
		
	def torrentFinished(self,tor):
		KTorrent.log("Torrent finished %s" % tor.name())
		if self.remove_on_finish_downloading:
			KTorrent.log("Removing %s" % tor.name())
			KTorrent.removeDelayed(tor.infoHash(),False)
			
			
	def seedingAutoStopped(self,tor,reason):
		KTorrent.log("Torrent finished seeding %s" % tor.name())
		if self.remove_on_finish_seeding:
			KTorrent.log("Removing %s" % tor.name())
			KTorrent.removeDelayed(tor.infoHash(),False)
	
	def torrentAdded(self,ih):
		tor = KTorrent.torrent(ih)
		KTorrent.log("Torrent added %s" % tor.name())
		tor.connect("finished(QObject* )",self.torrentFinished)
		tor.connect("seedingAutoStopped(QObject* ,const QString & )",self.seedingAutoStopped)

	def save(self):
		KTScriptingPlugin.writeConfigEntryBool("AutoRemoveScript","remove_on_finish_downloading",self.remove_on_finish_downloading)
		KTScriptingPlugin.writeConfigEntryBool("AutoRemoveScript","remove_on_finish_seeding",self.remove_on_finish_seeding)
		KTScriptingPlugin.syncConfig("AutoRemoveScript")
	
	def load(self):
		self.remove_on_finish_downloading = KTScriptingPlugin.readConfigEntryBool("AutoRemoveScript","remove_on_finish_downloading",False)
		self.remove_on_finish_seeding = KTScriptingPlugin.readConfigEntryBool("AutoRemoveScript","remove_on_finish_seeding",False)
		
	def configure(self):
		forms = Kross.module("forms")
		dialog = forms.createDialog(t.i18n("Auto Remove Settings"))
		dialog.setButtons("Ok|Cancel")
		page = dialog.addPage(t.i18n("Auto Remove"),t.i18n("Auto Remove"),"kt-remove")
		widget = forms.createWidgetFromUIFile(page,KTScriptingPlugin.scriptDir("auto_remove") + "auto_remove.ui")
		widget["finish_seeding"].checked = self.remove_on_finish_seeding
		widget["finish_downloading"].checked = self.remove_on_finish_downloading
		if dialog.exec_loop():
			self.remove_on_finish_seeding = widget["finish_seeding"].checked 
			self.remove_on_finish_downloading = widget["finish_downloading"].checked
			self.save()


ar = AutoRemove()
ar.load()

def configure():
	global ar
	ar.configure()

def unload():
	global ar
	del ar
