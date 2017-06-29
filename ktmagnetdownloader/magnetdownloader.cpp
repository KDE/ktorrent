#include <cstdio>
#include <cstdlib>

#include <QApplication>

#include <version.h>
#include <util/log.h>
#include <util/functions.h>

#include "magnettest.h"

using namespace bt;


int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ktmagnetdownloader <magnet-link>\n");
        return 0;
    }

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("KTMagnetDownloader"));
    app.setQuitOnLastWindowClosed(false);

    if (!bt::InitLibKTorrent())
    {
        fprintf(stderr, "Failed to initialize libktorrent\n");
        return -1;
    }

    bt::MagnetLink mlink(QString::fromUtf8(argv[1]));
    if (!mlink.isValid())
    {
        fprintf(stderr, "Invalid magnet link %s\n\n", argv[1]);
        fprintf(stderr, "Usage: ktmagnetdownloader <magnet-link>\n");
        return 0;
    }

    bt::SetClientInfo(QStringLiteral("ktmagnetdownloader"), bt::MAJOR, bt::MINOR, bt::BETA_ALPHA_RC_RELEASE, bt::BETA, QStringLiteral("KT"));
    bt::InitLog(QStringLiteral("ktmagnetdownload.log"), false, true);
    bt::Log& log = Out();
    log.setOutputToConsole(true);
    log << "Downloading " << mlink.toString() << bt::endl;

    MagnetTest mtest(mlink);

    return app.exec();
}
