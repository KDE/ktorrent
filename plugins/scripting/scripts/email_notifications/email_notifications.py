#!/usr/bin/env kross
import KTorrent
import KTScriptingPlugin
import Kross

import smtplib
from email.MIMEMultipart import MIMEMultipart
from email.MIMEBase import MIMEBase
from email.MIMEText import MIMEText
from email import Encoders
import os

mail_user = "your_email@gmail.com"
mail_pwd = "your_password"
mail_server = "smtp.gmail.com"
mail_port = 587
mail_dest = "your_email@gmail.com"
mail_add_cc = False
mail_cc = ""
mail_add_bcc = False
mail_bcc = ""
mail_use_tls = True

def mail(subject, text):
	global mail_user,mail_pwd,mail_server,mail_port,mail_dest,mail_add_cc,mail_cc,mail_add_bcc,mail_bcc,mail_use_tls
	msg = MIMEMultipart()
	
	msg['From'] = mail_user
	msg['To'] = mail_dest
	msg['Subject'] = subject
	if mail_add_cc:
		msg['CC'] = mail_cc
	if mail_add_bcc:
		msg['BCC'] = mail_bcc
	
	msg.attach(MIMEText(text))
	
	try:
		mailServer = smtplib.SMTP(mail_server, mail_port)
		mailServer.ehlo()
		if mail_use_tls:
			mailServer.starttls()
			mailServer.ehlo()
			
		mailServer.login(mail_user, mail_pwd)
		mailServer.sendmail(mail_user, mail_dest, msg.as_string())
		mailServer.quit()
	except SMTPRecipientsRefused:
		KTorrent.log("Failed to send e-mail notification: recipients refushed")
	except SMTPHeloError:
		KTorrent.log("Failed to send e-mail notification: helo error")
	except SMTPSenderRefused:
		KTorrent.log("Failed to send e-mail notification: sender refused")
	except SMTPDataError:
		KTorrent.log("Failed to send e-mail notification: something went wrong sending data")
	except SMTPAuthenticationError:
		KTorrent.log("Failed to send e-mail notification: authentication failed")
	except SMTPException:
		KTorrent.log("Failed to send e-mail notification")
	else:
		KTorrent.log("Successfully sent e-mail notification")

#mail("some.person@some.address.com",
#   "Hello from python!",
#   "This is a email sent with python")


def save():
	global mail_user,mail_pwd,mail_server,mail_port,mail_dest,mail_add_cc,mail_cc,mail_add_bcc,mail_bcc,mail_use_tls
	KTScriptingPlugin.writeConfigEntry("EMailNotificationsScript","username",mail_user)
	KTScriptingPlugin.writeConfigEntry("EMailNotificationsScript","password",mail_pwd)
	KTScriptingPlugin.writeConfigEntry("EMailNotificationsScript","server",mail_server)
	KTScriptingPlugin.writeConfigEntryInt("EMailNotificationsScript","port",mail_port)
	KTScriptingPlugin.writeConfigEntry("EMailNotificationsScript","dest",mail_dest)
	KTScriptingPlugin.writeConfigEntryBool("EMailNotificationsScript","add_cc",mail_add_cc)
	KTScriptingPlugin.writeConfigEntry("EMailNotificationsScript","cc",mail_cc)
	KTScriptingPlugin.writeConfigEntryBool("EMailNotificationsScript","add_bcc",mail_add_bcc)
	KTScriptingPlugin.writeConfigEntry("EMailNotificationsScript","bcc",mail_bcc)
	KTScriptingPlugin.writeConfigEntryBool("EMailNotificationsScript","use_tls",mail_use_tls)
	KTScriptingPlugin.syncConfig("EMailNotificationsScript")

def load():
	global mail_user,mail_pwd,mail_server,mail_port,mail_dest,mail_add_cc,mail_cc,mail_add_bcc,mail_bcc,mail_use_tls
	mail_user = KTScriptingPlugin.readConfigEntry("EMailNotificationsScript","username",mail_user)
	mail_pwd = KTScriptingPlugin.readConfigEntry("EMailNotificationsScript","password",mail_pwd)
	mail_server = KTScriptingPlugin.readConfigEntry("EMailNotificationsScript","server",mail_server)
	mail_port = KTScriptingPlugin.readConfigEntryInt("EMailNotificationsScript","port",mail_port)
	mail_dest = KTScriptingPlugin.readConfigEntry("EMailNotificationsScript","dest",mail_dest)
	mail_add_cc = KTScriptingPlugin.readConfigEntryBool("EMailNotificationsScript","add_cc",mail_add_cc)
	mail_cc = KTScriptingPlugin.readConfigEntry("EMailNotificationsScript","cc",mail_cc)
	mail_add_bcc = KTScriptingPlugin.readConfigEntryBool("EMailNotificationsScript","add_bcc",mail_add_bcc)
	mail_bcc = KTScriptingPlugin.readConfigEntry("EMailNotificationsScript","bcc",mail_bcc)
	mail_use_tls = KTScriptingPlugin.readConfigEntryBool("EMailNotificationsScript","use_tls",mail_use_tls)

def configure():
	global mail_user,mail_pwd,mail_server,mail_port,mail_dest,mail_add_cc,mail_cc,mail_add_bcc,mail_bcc,mail_use_tls
	forms = Kross.module("forms")
	dialog = forms.createDialog("E-Mail Script Settings")
	dialog.setButtons("Ok|Cancel")
	page = page = dialog.addPage("E-Mail Settings","E-Mail Settings","mail-send")
	widget = forms.createWidgetFromUIFile(page,KTScriptingPlugin.scriptsDir() + "email_notifications/emailconfig.ui")
	widget["username"].text = mail_user
	widget["password"].text = mail_pwd
	widget["server"].text = mail_server
	widget["port"].value = mail_port
	widget["to"].text = mail_dest
	widget["cc"].text = mail_cc
	widget["bcc"].text = mail_bcc
	widget["add_cc"].checked = mail_add_cc
	widget["add_bcc"].checked = mail_add_bcc
	widget["cc"].enabled = mail_add_cc
	widget["bcc"].enabled = mail_add_bcc
	widget["tls"].checked = mail_use_tls
	if dialog.exec_loop():
		# update settings
		mail_user = widget["username"].text
		mail_pwd = widget["password"].text 
		mail_server = widget["server"].text
		mail_port = widget["port"].value
		mail_dest = widget["to"].text
		mail_cc = widget["cc"].text
		mail_bcc = widget["bcc"].text
		mail_add_cc = widget["add_cc"].checked
		mail_add_bcc = widget["add_bcc"].checked
		mail_use_tls = widget["tls"].checked
		save()
	


def torrentFinished(tor):
	mail("Torrent %s has finished" % (tor.name()),"The download of the torrent %s has finished" % (tor.name()))
	
def torrentStoppedByError(tor,error_message):
	mail("Torrent %s was stopped by an error" % (tor.name()), "Error: " + error_message)
	
def seedingAutoStopped(tor,reason):
	mail("Torrent %s has finished seeding" % (tor.name()), "Seeding stopped because " + reason)
	
def corruptedDataFound(tor):
	mail("Corrupted data found","KTorrent has found corrupted data in the torrent " + tor.name())
	

def connectSignals(tor):
	tor.connect("finished(DBusTorrent* )",torrentFinished)
	tor.connect("stoppedByError(DBusTorrent* ,const QString & )",torrentStoppedByError)
	tor.connect("seedingAutoStopped(DBusTorrent* ,const QString & )",seedingAutoStopped)
	tor.connect("corruptedDataFound(DBusTorrent* )",corruptedDataFound)
	
def torrentAdded(ih):
	connectSignals(KTorrent.torrent(ih))
 
 # load settings
load()
KTorrent.connect("torrentAdded(const QString &)",torrentAdded)
tors = KTorrent.torrents()
# bind to signals for each torrent
for t in tors:
	connectSignals(KTorrent.torrent(t))
