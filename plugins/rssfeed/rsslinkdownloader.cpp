/***************************************************************************
 *   Copyright (C) 2006 by Alan Jones                                      *
 *   skyphyr@gmail.com                                                     *
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
 ***************************************************************************/
#include "rsslinkdownloader.h"

#include <klocale.h>
#include <kmimetype.h>
#include <kmessagebox.h>

#include <qfile.h>

#include "../../libktorrent/torrent/bdecoder.h"
#include "../../libktorrent/torrent/bnode.h"

using namespace bt;

namespace kt
{

	RssLinkDownloader::RssLinkDownloader(CoreInterface* core, QString link, RssFilter * filter, QObject * parent) : QObject (parent)
		{
			//tempFile.setAutoDelete(true);
			m_core = core;
			firstLink = true;
			curFilter = filter;
			if (!KURL(link).isValid())
			{
				// no valid URL, so just display an error message
				KMessageBox::error(0,i18n("Failed to find and download a valid torrent for %1").arg(curLink));
				QTimer::singleShot(50,this,SLOT(suicide()));
			}
			else
			{
				//first let's download the link so we can process it to check for the actual torrent
				curLink = curSubLink = link;
				curFile = KIO::storedGet(link,false,false);
				connect(curFile, SIGNAL(result(KIO::Job*)),this,SLOT(processLink( KIO::Job* )));
			}
		}
	
	RssLinkDownloader::~RssLinkDownloader()
		{
			
		}
		
	void RssLinkDownloader::processLink(KIO::Job* jobStatus)
		{
		
		if (!jobStatus->error())
			{
			//the file downloaded ok - so let's check if it's a torrent
			KMimeType linkType = *KMimeType::findByContent(curFile->data());
			if (linkType.is("text/html"))
				{
				if (firstLink)
					{
					KURL url = curLink;
					//let's go through the data and populate our sublink array
					QTextStream html(curFile->data(), IO_ReadOnly);
					
					//go through a line at a time checking for a torrent
					QString htmlline = html.readLine();
					while (!htmlline.isNull())
						{
						QRegExp hrefTags = QString("<A.*HREF.*</A");
						hrefTags.setCaseSensitive(false);
						hrefTags.setMinimal(true);
						
						int matchPos = 0;
						while (htmlline.find(hrefTags, matchPos) >= 0)
							{
							matchPos += hrefTags.matchedLength();
							//we're found an <a href tag - let's check it if contains download
							QRegExp hrefText = QString("d(own)?load");
							hrefText.setCaseSensitive(false);
							
							if (!hrefTags.capturedTexts()[0].contains(hrefText))
								{
								//link text doesn't contain dload/download
								continue;
								}
								
							//we're found an <a href tag - now let's the the url out of it
							hrefText = QString("HREF=\"?([^\">< ]*)[\" ]");
							hrefText.setCaseSensitive(false);
				
							hrefTags.capturedTexts()[0].find(hrefText);
							//lets get the captured
							QString hrefLink = hrefText.capturedTexts()[1];
								
							if (hrefLink.startsWith("/"))
								{
								hrefLink = url.protocol() + "://" + url.host() + hrefLink;
								} 
							else if (!hrefLink.startsWith("http://", false)) 
								{
								hrefLink = url.url().left(url.url().findRev("/")+1) + hrefLink;
								}
								
							subLinks.append(hrefLink);
							
							}
							
							//run the query again
							htmlline = html.readLine();
						}
						
						
						firstLink = false;
					}
				}
			else
				{
			
				//I know this may check a file which we've already been told is html, but sometimes it lies
				try
					{
						//last ditched brute force attempt to check if it's a torrent file
						BNode* node = 0;
						BDecoder decoder(curFile->data(),false);
						node = decoder.decode();
						BDictNode* dict = dynamic_cast<BDictNode*>(node);
						
						if (dict)
						{
							delete node;
							node = dict = 0;
							
							if (curFilter)
							{
								m_core->loadSilently( curSubLink );
								emit linkDownloaded( curLink, 3);
							}
							else
							{
								m_core->load( curSubLink );
								emit linkDownloaded( curLink, 1);
							}
							
							//delete ourself and finish
							deleteLater();
							return;
						}
						
					
					}
				catch (...)
					{
						//we can just ignore any errors here
					}
				}
				
			}
		//curFile->deleteLater();
		
		//check for the next item
		if (subLinks.isEmpty())
			{
			if (curFilter)
				{
				//we've failed to download a torrent for this match
				curFilter->deleteMatch( curLink );
				}
			else
				{
				//failed to download a selected article from a feed
				KMessageBox::error(0,i18n("Failed to find and download a valid torrent for %1").arg(curLink));
				}
				deleteLater();
			}
		else
			{
			curSubLink = subLinks.first();
			subLinks.pop_front();
			curFile = KIO::storedGet(curSubLink,false,false);
			connect(curFile, SIGNAL(result(KIO::Job*)),this,SLOT(processLink( KIO::Job* )));
			}
		}
	
	
	void RssLinkDownloader::suicide()
	{
		deleteLater();
	}

}