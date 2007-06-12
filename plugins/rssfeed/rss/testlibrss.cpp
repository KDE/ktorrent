#include "testlibrss.h"

#include "image.h"

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdebug.h>

using namespace RSS;

static const KCmdLineOptions options[] =
{
  { "+url", I18N_NOOP("URL of feed"), 0 },
  KCmdLineLastOption
};


void Tester::test( const QString &url )
{
	Loader *loader = Loader::create();
	connect( loader, SIGNAL( loadingComplete( Loader *, Document, Status ) ),
	         this, SLOT( slotLoadingComplete( Loader *, Document, Status ) ) );
	loader->loadFrom( url, new FileRetriever );
}

void Tester::slotLoadingComplete( Loader *loader, Document doc, Status status )
{
	if ( status == Success )
	{
		kdDebug() << "Successfully retrieved '" << doc.title() << "'" << endl;
		kdDebug() << doc.description() << endl;

                if ( doc.image() ) {
                      kdDebug() << "Image: ";
                      kdDebug() << "  Title: " << doc.image()->title() << endl;
                      kdDebug() << "  URL: " << doc.image()->url() << endl;
                      kdDebug() << "  Link: " << doc.image()->link() << endl;
                }
		
		kdDebug() << "Articles:" << endl;

		Article::List list = doc.articles();
		Article::List::ConstIterator it;
		Article::List::ConstIterator en=list.end();
		for (it = list.begin(); it != en; ++it)
		{
		    kdDebug() << "\tTitle: " << (*it).title() << endl;
		    kdDebug() << "\tText:  " << (*it).description() << endl;
		}
	}

	if ( status != Success )
		kdDebug() << "ERROR " << loader->errorCode() << endl;

	kapp->quit();
}

int main( int argc, char **argv )
{
	KAboutData aboutData( "testlibrss", "testlibrss", "0.1" );
	KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineArgs::addCmdLineOptions( options );
	KApplication app;

        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        if ( args->count() != 1 ) args->usage();

	Tester tester;
	tester.test( args->arg( 0 ) );

	return app.exec();
}

#include "testlibrss.moc"
