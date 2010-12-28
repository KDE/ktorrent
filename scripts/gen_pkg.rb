#!/usr/bin/env ruby
#
# Ruby script for generating amaroK tarball releases from KDE SVN
#
# (c) 2005 Mark Kretschmann <markey@web.de>
# (c) 2006-2007 Tom Albers <tomalbers@kde.nl>
# Some parts of this code taken from cvs2dist
# License: GNU General Public License V2
if ARGV.length != 2
	puts "Usage: gen_kde4_pkg_stable.rb <version> <i18n-branch>"
	exit
end

egmodule   = "network"
name       = "ktorrent"
docs       = "no"
addDocs    = []
addPo      = []
remove     = ""

version    = ARGV[0]
svnbase    = "svn+ssh://guisson@svn.kde.org/home/kde"
if ARGV[1] == "stable"
	svnroot = "#{svnbase}/branches/stable"
	svnextragear = "extragear-kde4"
else
	svnroot = "#{svnbase}/trunk"
	svnextragear = "extragear"
end
svntags    = "#{svnbase}/tags/#{name}"


#----------------------------------------------------------------

folder     = name + "-" + version
addPo      = [name] + addPo
addDocs    = [name] + addDocs

puts "Fetching #{egmodule}/#{name}..."
# Remove old folder, if exists
`rm -rf #{folder} 2> /dev/null`
`rm -rf folder.tar.bz2 2> /dev/null`

Dir.mkdir( folder )
Dir.chdir( folder )

# Do the main checkouts.
Dir.mkdir( name + "-tmp" )
Dir.chdir( name + "-tmp" )
`git archive --format=tar -o tmp.tar --remote=git@git.kde.org:ktorrent v#{version}`
`tar -xvf tmp.tar && rm tmp.tar`

# Move them to the toplevel
`/bin/mv * ..`
Dir.chdir( ".." )
`find -name ".svn" | xargs rm -rf`
`rmdir #{name}-tmp`

puts "done\n"

puts "\n"
puts "Fetching l10n docs for #{egmodule}/#{name}...\n"
puts "\n"

i18nlangs = `svn cat #{svnroot}/l10n-kde4/subdirs`
i18nlangsCleaned = []
for lang in i18nlangs
  l = lang.chomp
  if (l != "x-test")
    i18nlangsCleaned += [l];
  end
end
i18nlangs = i18nlangsCleaned

Dir.mkdir( "l10n" )
Dir.chdir( "l10n" )

# docs
for lang in i18nlangs
  lang.chomp!

  for dg in addDocs
    dg.chomp!
    `rm -rf #{dg}`
    docdirname = "l10n-kde4/#{lang}/docs/extragear-#{egmodule}/#{dg}"
    if ( docs != "no")
        puts "Checking if #{dg} has translated documentation...\n"
        `svn co -q #{svnroot}/#{docdirname} > /dev/null 2>&1`
    end
    next unless FileTest.exists?( dg )
    print "Copying #{lang}'s #{dg} documentation over...  "
    `cp -R #{dg}/ ../doc/#{lang}_#{dg}`

    makefile = File.new( "../doc/#{lang}_#{dg}/CMakeLists.txt", File::CREAT | File::RDWR | File::TRUNC )
    makefile << "KDE4_CREATE_HANDBOOK(#{lang})\n"
    makefile.close()

    puts( "done.\n" )
  end
end

puts "\n"
puts "Fetching l10n po for #{egmodule}/#{name}...\n"
puts "\n"

Dir.chdir( ".." ) # in egmodule now

$subdirs = false
Dir.mkdir( "po" )

topmakefile = File.new( "po/CMakeLists.txt", File::CREAT | File::RDWR | File::TRUNC )
for lang in i18nlangs
  lang.chomp!
  dest = "po/#{lang}"

  for dg in addPo
    dg.chomp!
    pofilename = "l10n-kde4/#{lang}/messages/extragear-#{egmodule}/#{dg}.po"
    `svn cat #{svnroot}/#{pofilename} 2> /dev/null | tee l10n/#{dg}.po`
    next if FileTest.size( "l10n/#{dg}.po" ) == 0

    if !FileTest.exist?( dest )
      Dir.mkdir( dest )
      makefile = File.new( "#{dest}/CMakeLists.txt", File::CREAT | File::RDWR | File::TRUNC )
      makefile << "file(GLOB _po_files *.po)\n"
      makefile << "GETTEXT_PROCESS_PO_FILES( #{lang} ALL INSTALL_DESTINATION ${LOCALE_INSTALL_DIR} ${_po_files} )\n"
      makefile.close()
      topmakefile << "add_subdirectory( #{lang} )\n"
    end

    print "Copying #{lang}'s #{dg}.po over ..  "
    `mv l10n/#{dg}.po #{dest}`
    puts( "done.\n" )


  end
end
topmakefile.close()

`rm -rf l10n`
puts "\n"

# add l10n to compilation.
`echo "find_package(Msgfmt REQUIRED)" >> CMakeLists.txt`
`echo "find_package(Gettext REQUIRED)" >> CMakeLists.txt`
`echo "add_subdirectory( po )" >> CMakeLists.txt`

# Remove cruft 
`find -name ".svn" | xargs rm -rf`
#`find -name "Messages.sh" | xargs rm -rf`
if ( remove != "")
   `/bin/rm #{remove}`
end

puts "\n"
puts "Compressing..  "
Dir.chdir( ".." ) # root folder
`tar -jcf #{folder}.tar.bz2 #{folder}`
#`rm -rf #{folder}`
puts "done.\n"

