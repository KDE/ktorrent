#!/usr/bin/env kf5kross
# -*- coding: utf-8 -*-
import KTorrent
import KTScriptingPlugin
import Kross
#from PyQt4 import QtCore

t = Kross.module("kdetranslation")

class AutoResume:
	def __init__(self):
		self.auto_resume = False
		self.hours = 0
		self.minutes = 5
		self.seconds = 0
		KTorrent.connect("suspendStateChanged(bool)",self.suspendedStateChanged)
		self.timer = KTScriptingPlugin.createTimer(True)
		self.timer.connect('timeout()',self.timerFired)
		

	def timerFired(self):
		if KTorrent.suspended():
			KTorrent.log("AutoResumeScript: resuming suspended torrents")
			KTorrent.setSuspended(False)
		
	def startTimer(self):
		self.timer.start((self.hours * 3600 + self.minutes * 60 + self.seconds)*1000)

	def save(self):
		KTScriptingPlugin.writeConfigEntryBool("AutoResumeScript","auto_resume",self.auto_resume)
		KTScriptingPlugin.writeConfigEntryInt("AutoResumeScript","hours",self.hours)
		KTScriptingPlugin.writeConfigEntryInt("AutoResumeScript","minutes",self.minutes)
		KTScriptingPlugin.writeConfigEntryInt("AutoResumeScript","seconds",self.seconds)
		KTScriptingPlugin.syncConfig("AutoResumeScript")
	
	def load(self):
		self.auto_resume = KTScriptingPlugin.readConfigEntryBool("AutoResumeScript","auto_resume",self.auto_resume)
		self.hours = KTScriptingPlugin.readConfigEntryInt("AutoResumeScript","hours",self.hours)
		self.minutes = KTScriptingPlugin.readConfigEntryInt("AutoResumeScript","minutes",self.minutes)
		self.seconds = KTScriptingPlugin.readConfigEntryInt("AutoResumeScript","seconds",self.seconds)
		if self.auto_resume and KTorrent.suspended():
			self.startTimer()
			
	def suspendedStateChanged(self,on):
		if on and self.auto_resume:
			KTorrent.log("AutoResumeScript: torrents suspended, starting timer")
			self.startTimer()
		else:
			self.timer.stop()
			
	def configure(self):
		forms = Kross.module("forms")
		dialog = forms.createDialog(t.i18n("Auto Resume Settings"))
		dialog.setButtons("Ok|Cancel")
		page = dialog.addPage(t.i18n("Auto Resume"),t.i18n("Auto Resume"),"kt-bandwidth-scheduler")
		widget = forms.createWidgetFromUIFile(page,KTScriptingPlugin.scriptDir("auto_resume") + "auto_resume.ui")
		widget["auto_resume"].checked = self.auto_resume
		widget["hours"].value = self.hours
		widget["minutes"].value = self.minutes
		widget["seconds"].value = self.seconds
		widget["hours"].enabled = self.auto_resume
		widget["minutes"].enabled = self.auto_resume
		widget["seconds"].enabled = self.auto_resume
		if dialog.exec_loop():
			self.auto_resume = widget["auto_resume"].checked
			self.hours = widget["hours"].value
			self.minutes = widget["minutes"].value
			self.seconds = widget["seconds"].value
			self.save()
			if self.auto_resume and KTorrent.suspended():
				self.startTimer()


ar = AutoResume()
ar.load()

def configure():
	global ar
	ar.configure()

def unload():
	global ar
	del ar
