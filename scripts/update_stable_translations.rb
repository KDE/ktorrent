#!/usr/bin/env ruby
#
# Ruby script for updating stable translations of ktorrent with those from trunk
#
# (c) 2009 Joris Guisson
# License: GNU General Public License V2

# variables
svnbase = "svn+ssh://guisson@svn.kde.org/home/kde/"
trunk = svnbase + "trunk/l10n-kde4"
stable = svnbase + "branches/stable/l10n-kde4"
workdir = "build"

Dir.mkdir(workdir)
puts "Checking out stable and trunk translations dir\n"
# checkout both, but do not go recursive
`svn co -N #{trunk} #{workdir}/trunk`
`svn co -N #{stable} #{workdir}/stable`

puts "Updating templates\n"
# update templates
`svn update -N #{workdir}/trunk/templates`
`svn update -N #{workdir}/trunk/templates/messages/`
`svn update -N #{workdir}/trunk/templates/messages/extragear-network`
`svn update -N #{workdir}/stable/templates`
`svn update -N #{workdir}/stable/templates/messages/`
`svn update -N #{workdir}/stable/templates/messages/extragear-network`
`cp #{workdir}/trunk/templates/messages/extragear-network/ktorrent.pot #{workdir}/stable/templates/messages/extragear-network/ktorrent.pot`
`cp #{workdir}/trunk/templates/messages/extragear-network/desktop_extragear-network_ktorrent.pot #{workdir}/stable/templates/messages/extragear-network/desktop_extragear-network_ktorrent.pot`

# get all translation dirs
i18nlangs = `svn cat #{trunk}/subdirs`
i18nlangsCleaned = []
for lang in i18nlangs
	l = lang.chomp
	if (l != "x-test")
		i18nlangsCleaned += [l];
	end
end
i18nlangs = i18nlangsCleaned

for lang in i18nlangs
	lang.chomp!
	puts "Updating #{lang} ...\n"
	`svn update -N #{workdir}/trunk/#{lang}/`
	`svn update -N #{workdir}/trunk/#{lang}/messages/`
	`svn update -N #{workdir}/trunk/#{lang}/messages/extragear-network`
	`svn update -N #{workdir}/stable/#{lang}/`
	`svn update -N #{workdir}/stable/#{lang}/messages/`
	`svn update -N #{workdir}/stable/#{lang}/messages/extragear-network`
	if FileTest.exist?("#{workdir}/trunk/#{lang}/messages/extragear-network")
		files = Dir.entries("#{workdir}/trunk/#{lang}/messages/extragear-network")
		if files.include?("ktorrent.po") and files.include?("desktop_extragear-network_ktorrent.po")
			if !FileTest.exist?("#{workdir}/stable/#{lang}/messages/extragear-network")
				`svn mkdir --parents #{workdir}/stable/#{lang}/messages/extragear-network`
			end
			`cp #{workdir}/trunk/#{lang}/messages/extragear-network/ktorrent.po #{workdir}/stable/#{lang}/messages/extragear-network`
			`cp #{workdir}/trunk/#{lang}/messages/extragear-network/desktop_extragear-network_ktorrent.po #{workdir}/stable/#{lang}/messages/extragear-network`
			`svn add -q #{workdir}/stable/#{lang}/messages/extragear-network/ktorrent.po`
			`svn add -q #{workdir}/stable/#{lang}/messages/extragear-network/desktop_extragear-network_ktorrent.po`
		end
	end
end
