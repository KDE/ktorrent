#!/usr/bin/python
import KTorrent
import Kross

KTorrent.log("This is the download order script")

def configure():
	forms = Kross.module("forms")
	dialog = forms.createDialog("Download Order Script Config Dialog")
	dialog.setButtons("Ok|Cancel")
	page = dialog.addPage("Welcome","Welcome Page","document-open")
	label = forms.createWidget(page,"QLabel")
	label.text = "Hello World Label"
	if dialog.exec_loop():
		forms.showMessageBox("Information", "Okay...", "The Ok-button was pressed")
