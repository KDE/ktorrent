#!/usr/bin/env kf5kross
# -*- coding: utf-8 -*-
import KTorrent
import KTScriptingPlugin
import Kross

import smtplib
from email.MIMEMultipart import MIMEMultipart
from email.MIMEBase import MIMEBase
from email.MIMEText import MIMEText
from email import Encoders
import os
import socket

t = Kross.module("kdetranslation")

class EMailNotifier:
	def __init__(self):
		self.mail_user = "your_email@gmail.com"
		self.mail_pwd = "your_password"
		self.mail_server = "smtp.gmail.com"
		self.mail_port = 587
		self.mail_dest = "your_email@gmail.com"
		self.mail_add_cc = False
		self.mail_cc = ""
		self.mail_add_bcc = False
		self.mail_bcc = ""
		self.mail_use_tls = True
		KTorrent.connect("torrentAdded(const QString &)",self.torrentAdded)
		tors = KTorrent.torrents()
		# bind to signals for each torrent
		for t in tors:
			self.torrentAdded(t)

	def mail(self,subject, text):
		msg = MIMEMultipart()
		
		msg['From'] = self.mail_user
		msg['To'] = self.mail_dest
		msg['Subject'] = subject
		if self.mail_add_cc:
			msg['CC'] = self.mail_cc
		if self.mail_add_bcc:
			msg['BCC'] = self.mail_bcc
		
		msg.attach(MIMEText(text))
		KTorrent.log("Sending mail : " + subject)
		
		try:
			mailServer = smtplib.SMTP(self.mail_server, self.mail_port)
			mailServer.ehlo()
			if self.mail_use_tls:
				mailServer.starttls()
				mailServer.ehlo()
				
			mailServer.login(self.mail_user, self.mail_pwd)
			mailServer.sendmail(self.mail_user, self.mail_dest, msg.as_string())
			mailServer.quit()
		except smtplib.SMTPRecipientsRefused:
			KTorrent.log("Failed to send e-mail notification: recipients refushed")
		except smtplib.SMTPHeloError:
			KTorrent.log("Failed to send e-mail notification: helo error")
		except smtplib.SMTPSenderRefused:
			KTorrent.log("Failed to send e-mail notification: sender refused")
		except smtplib.SMTPDataError:
			KTorrent.log("Failed to send e-mail notification: something went wrong sending data")
		except smtplib.SMTPAuthenticationError:
			KTorrent.log("Failed to send e-mail notification: authentication failed")
		except smtplib.SMTPException:
			KTorrent.log("Failed to send e-mail notification")
		except socket.error, err:
			KTorrent.log("Failed to send e-mail notification: %s " % err)
		except:
			KTorrent.log("Failed to send e-mail notification")
		else:
			KTorrent.log("Successfully sent e-mail notification")

	def save(self):
		KTScriptingPlugin.writeConfigEntry("EMailNotificationsScript","username",self.mail_user)
		KTScriptingPlugin.writeConfigEntry("EMailNotificationsScript","password",self.mail_pwd)
		KTScriptingPlugin.writeConfigEntry("EMailNotificationsScript","server",self.mail_server)
		KTScriptingPlugin.writeConfigEntryInt("EMailNotificationsScript","port",self.mail_port)
		KTScriptingPlugin.writeConfigEntry("EMailNotificationsScript","dest",self.mail_dest)
		KTScriptingPlugin.writeConfigEntryBool("EMailNotificationsScript","add_cc",self.mail_add_cc)
		KTScriptingPlugin.writeConfigEntry("EMailNotificationsScript","cc",self.mail_cc)
		KTScriptingPlugin.writeConfigEntryBool("EMailNotificationsScript","add_bcc",self.mail_add_bcc)
		KTScriptingPlugin.writeConfigEntry("EMailNotificationsScript","bcc",self.mail_bcc)
		KTScriptingPlugin.writeConfigEntryBool("EMailNotificationsScript","use_tls",self.mail_use_tls)
		KTScriptingPlugin.syncConfig("EMailNotificationsScript")
	
	def load(self):
		self.mail_user = KTScriptingPlugin.readConfigEntry("EMailNotificationsScript","username",self.mail_user)
		self.mail_pwd = KTScriptingPlugin.readConfigEntry("EMailNotificationsScript","password",self.mail_pwd)
		self.mail_server = KTScriptingPlugin.readConfigEntry("EMailNotificationsScript","server",self.mail_server)
		self.mail_port = KTScriptingPlugin.readConfigEntryInt("EMailNotificationsScript","port",self.mail_port)
		self.mail_dest = KTScriptingPlugin.readConfigEntry("EMailNotificationsScript","dest",self.mail_dest)
		self.mail_add_cc = KTScriptingPlugin.readConfigEntryBool("EMailNotificationsScript","add_cc",self.mail_add_cc)
		self.mail_cc = KTScriptingPlugin.readConfigEntry("EMailNotificationsScript","cc",self.mail_cc)
		self.mail_add_bcc = KTScriptingPlugin.readConfigEntryBool("EMailNotificationsScript","add_bcc",self.mail_add_bcc)
		self.mail_bcc = KTScriptingPlugin.readConfigEntry("EMailNotificationsScript","bcc",self.mail_bcc)
		self.mail_use_tls = KTScriptingPlugin.readConfigEntryBool("EMailNotificationsScript","use_tls",self.mail_use_tls)

	def configure(self):
		forms = Kross.module("forms")
		dialog = forms.createDialog(t.i18n("E-Mail Script Settings"))
		dialog.setButtons("Ok|Cancel")
		page = page = dialog.addPage(t.i18n("E-Mail Settings"),t.i18n("E-Mail Settings"),"mail-send")
		widget = forms.createWidgetFromUIFile(page,KTScriptingPlugin.scriptDir("email_notifications") + "emailconfig.ui")
		widget["username"].text = self.mail_user
		widget["password"].text = self.mail_pwd
		widget["server"].text = self.mail_server
		widget["port"].value = self.mail_port
		widget["to"].text = self.mail_dest
		widget["cc"].text = self.mail_cc
		widget["bcc"].text = self.mail_bcc
		widget["add_cc"].checked = self.mail_add_cc
		widget["add_bcc"].checked = self.mail_add_bcc
		widget["cc"].enabled = self.mail_add_cc
		widget["bcc"].enabled = self.mail_add_bcc
		widget["tls"].checked = self.mail_use_tls
		if dialog.exec_loop():
			# update settings
			self.mail_user = widget["username"].text
			self.mail_pwd = widget["password"].text 
			self.mail_server = widget["server"].text
			self.mail_port = widget["port"].value
			self.mail_dest = widget["to"].text
			self.mail_cc = widget["cc"].text
			self.mail_bcc = widget["bcc"].text
			self.mail_add_cc = widget["add_cc"].checked
			self.mail_add_bcc = widget["add_bcc"].checked
			self.mail_use_tls = widget["tls"].checked
			self.save()
	
	def torrentFinished(self,tor):
		self.mail("Torrent " + tor.name() + " has finished","The download of the torrent " + tor.name() + " has finished")
		
	def torrentStoppedByError(self,tor,error_message):
		self.mail("Torrent " + tor.name() + " was stopped by an error", "Error: " + error_message)
		
	def seedingAutoStopped(self,tor,reason):
		self.mail("Torrent " + tor.name() + " has finished seeding", "Seeding stopped because " + reason)
		
	def corruptedDataFound(self,tor):
		self.mail("Corrupted data found","KTorrent has found corrupted data in the torrent " + tor.name())
	

	def connectSignals(self,tor):
		KTorrent.log("connectSignals " + tor.name())
		tor.connect("finished(QObject* )",self.torrentFinished)
		tor.connect("stoppedByError(QObject* ,const QString & )",self.torrentStoppedByError)
		tor.connect("seedingAutoStopped(QObject* ,const QString & )",self.seedingAutoStopped)
		tor.connect("corruptedDataFound(QObject* )",self.corruptedDataFound)
	
	def torrentAdded(self,ih):
		tor = KTorrent.torrent(ih)
		self.connectSignals(tor)
 
 # load settings
notifier = EMailNotifier()
notifier.load()

def configure():
	global notifier
	notifier.configure()

def unload():
	global notifier
	del notifier
