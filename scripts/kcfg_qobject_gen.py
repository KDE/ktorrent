#!/usr/bin/env python
# -*- coding: utf-8 -*-

# 
# Script to generate a QObject based class out of a kcfg and kcfgc file which
# Can be used as a dbus interface.
#
import sys
import getopt
import xml.dom.minidom
import traceback

license = """/***************************************************************************
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
 ***************************************************************************/"""

class_declaration_header = """%(license)s 
#ifndef __%(class_name)s__
#define __%(class_name)s__

#include <QObject>
#include <QList>
#include <KUrl>
#include <ktcore_export.h>

namespace %(namespace)s 
{
\tclass CoreInterface;

\tclass KTCORE_EXPORT %(class_name)s : public QObject
\t{
\t\tQ_OBJECT
\t\tQ_CLASSINFO("D-Bus Interface", "org.ktorrent.settings")
\tpublic:
\t\t%(class_name)s(CoreInterface* core,QObject* parent);
\t\tvirtual~%(class_name)s();

\tpublic slots:
"""

class_declaration_footer = """
\t\tQ_SCRIPTABLE void apply();
\tprivate:
\t\tCoreInterface* core;
\t};
}

#endif
"""

class_implementation_header = """%(license)s 
#include "%(header_file)s.h"
#include "settings.h"
#include <QDBusConnection>
#include <interfaces/coreinterface.h>

namespace %(namespace)s 
{
\t%(class_name)s::%(class_name)s(CoreInterface* core,QObject* parent) : QObject(parent),core(core)
\t{
\t\tQDBusConnection::sessionBus().registerObject("/settings", this,
\t\t\tQDBusConnection::ExportScriptableSlots|QDBusConnection::ExportScriptableSignals);
\t}

\t%(class_name)s::~%(class_name)s()
\t{}

\tvoid %(class_name)s::apply()
\t{
\t\tcore->applySettings();
\t}
"""
get_template = """
\t%(type)s %(class_name)s::%(getter)s()
\t{
\t\treturn %(settings)s::%(getter)s();
\t}
"""

set_template = """
\tvoid %(class_name)s::%(setter)s(%(type)s val)
\t{
\t\t%(settings)s::%(setter)s(val);
\t}
"""

class KCfgFile:
	def __init__(self,kcfg_file):
		self.kcfg_file = kcfg_file
		self.type_map = {
			"Int" : "int", 
			"Bool" : "bool", 
			"Double" : "double", 
			"Url" : "KUrl", 
			"String" : "QString",
			"IntList" : "QList<int>"
		}
		
	def load(self):
		self.doc = xml.dom.minidom.parse(self.kcfg_file)
		self.entries = self.doc.getElementsByTagName("entry")
		#for entry in self.entries:
		#	print "Name: %s, Type: %s" % (entry.getAttribute("name"),entry.getAttribute("type"))
			
	def cppType(self,kcfg_type):
		if kcfg_type in self.type_map:
			return self.type_map[kcfg_type]
		else:
			raise "Unkown type %s" % (kcfg_type)
			
	def writeDeclarations(self,indent,output):
		for entry in self.entries:
			cpp_type = self.cppType(entry.getAttribute("type"));
			name = entry.getAttribute("name")
			# write the get function
			output.write("%sQ_SCRIPTABLE %s %s();\n" % (indent,cpp_type,name))
			output.write("%sQ_SCRIPTABLE void set%s(%s val);\n" % (indent,name[0].capitalize() + name[1:],cpp_type))
			
	def writeImplementations(self,indent,output,class_name,settings_class_name):
		global get_template
		global set_template
		for entry in self.entries:
			cpp_type = self.cppType(entry.getAttribute("type"))
			name = entry.getAttribute("name")
			setter = "set" + name[0].capitalize() + name[1:]
			d = {"class_name" : class_name, "type" : cpp_type, "getter" : name, "setter" : setter, "settings" : settings_class_name}
			output.write(get_template % d)
			output.write(set_template % d)
			
			
class KCfgcFile:
	def __init__(self,kcfgc_file):
		self.kcfgc_file = kcfgc_file
		self.class_name = ""
		
	def load(self):
		infile = open(self.kcfgc_file,"r")
		for line in infile:
			if line.startswith("ClassName"):
				self.class_name = line.split('=')[1].strip()
				
	def className(self):
		return self.class_name
				
class Generator:
	def __init__(self,class_name,namespace,settings_class_name,kcfg_file):
		self.class_name = class_name
		self.namespace = namespace
		self.settings_class = settings_class_name
		self.kcfg_file = kcfg_file
		
	def header(self,output):
		global class_declaration_header
		global class_declaration_footer
		global license
		output.write(class_declaration_header % 
			{"class_name" : self.class_name, "namespace" : self.namespace, "license" : license})
		self.kcfg_file.writeDeclarations("\t\t",output)
		output.write(class_declaration_footer)
		
	def implementation(self,output,header_file):
		global class_implementation_header
		global license
		output.write(class_implementation_header % 
			{"class_name" : self.class_name, "namespace" : self.namespace, "license" : license, "header_file" : header_file})
		self.kcfg_file.writeImplementations("\t",output,self.class_name,self.settings_class)
		output.write("\n}\n")
	

def usage():
	print """
Usage: kcfg_qobject_gen.py -i input.kcfg -c input.kcfgc -o OutputClass [-n namespace ]
	"""

def main(args):
	try:                                
		opts, args = getopt.getopt(args, "hi:c:o:n:f:", ["help"])
	except getopt.GetoptError:          
		usage()                         
		sys.exit(2)                     
	
	kcfg_file = None
	kcfgc_file = None
	output_class = None
	output_namespace = 'kt'
	output_file = None
	for opt, arg in opts:               
		if opt in ("-h", "--help"):     
			usage()                     
			sys.exit()                  
		elif opt == '-i':               
			kcfg_file = arg
		elif opt == "-c":
			kcfgc_file = arg          
		elif opt == "-o":
			output_class = arg
		elif opt == "-n":
			output_namespace = arg
		elif opt == "-f":
			output_file = arg
			
	if kcfg_file == None or kcfgc_file == None or output_class == None:
		usage()
		sys.exit(2)
	
	try:
		kcfg = KCfgFile(kcfg_file)
		kcfg.load()
		kcfgc = KCfgcFile(kcfgc_file)
		kcfgc.load()
		gen = Generator(output_class,output_namespace,kcfgc.className(),kcfg)
		if output_file == None:
			output_file = output_class.lower()
		
		gen.header(open(output_file + ".h","w"))
		gen.implementation(open(output_file + ".cpp","w"),output_file)
	except:
		print "Failed to process input"
		traceback.print_exc(file=sys.stdout)
		exit(1)

if __name__ == "__main__":
	main(sys.argv[1:])
	exit(0)