/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <algorithm>
#include <errno.h>
#include <regex>
#include <string>

#include <QFile>
#include <QTextStream>
#include <QTimer>

#include <KIO/Job>
#include <KLocalizedString>

#include "convertdialog.h"
#include "convertthread.h"
#include <interfaces/functions.h>
#include <util/constants.h>
#include <util/fileops.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
ConvertThread::ConvertThread(ConvertDialog *dlg)
    : dlg(dlg)
    , abort(false)
{
    txt_file = kt::DataDir() + QStringLiteral("level1.txt");
    dat_file = kt::DataDir() + QStringLiteral("level1.dat");
    tmp_file = kt::DataDir() + QStringLiteral("level1.dat.tmp");
}

ConvertThread::~ConvertThread()
{
}

void ConvertThread::run()
{
    readInput();
    writeOutput();
}

void ConvertThread::readInput()
{
    /*    READ INPUT FILE  */
    QFile source(txt_file);
    if (!source.open(QIODevice::ReadOnly)) {
        Out(SYS_IPF | LOG_IMPORTANT) << "Cannot find level1.txt file" << endl;
        failure_reason = i18n("Cannot open %1: %2", txt_file, QString::fromLatin1(strerror(errno)));
        return;
    }

    Out(SYS_IPF | LOG_NOTICE) << "Loading " << txt_file << " ..." << endl;
    dlg->message(i18n("Loading txt file..."));

    ulong source_size = source.size();
    QTextStream stream(&source);

    int i = 0;
    const std::regex rx("(?:[0-9]{1,3}\\.){3}[0-9]{1,3}");

    while (!stream.atEnd() && !abort) {
        std::string line = stream.readLine().toStdString();
        i += line.length() * sizeof(char); // rough estimation of string size
        dlg->progress(i, source_size);
        ++i;

        std::vector<std::string> addresses;
        for (auto it = std::sregex_iterator(line.begin(), line.end(), rx); it != std::sregex_iterator(); ++it) {
            addresses.push_back(it->str());
        }

        // if we have found two addresses, create a block out of it
        if (addresses.size() == 2) {
            input += IPBlock(QString::fromStdString(addresses[0]), QString::fromStdString(addresses[1]));
        }
    }
    source.close();
    Out(SYS_IPF | LOG_NOTICE) << "Loaded " << input.count() << " lines" << endl;
    dlg->progress(100, 100);
}

static bool LessThan(const IPBlock &a, const IPBlock &b)
{
    if (a.ip1 == b.ip1)
        return a.ip2 < b.ip2;
    else
        return a.ip1 < b.ip1;
}

void ConvertThread::sort()
{
    std::sort(input.begin(), input.end(), LessThan);
}

void ConvertThread::merge()
{
    if (input.count() < 2) // noting to merge
        return;

    QList<IPBlock>::iterator i = input.begin();
    QList<IPBlock>::iterator j = i;
    j++;
    while (j != input.end() && i != input.end()) {
        IPBlock &a = *i;
        IPBlock &b = *j;
        if (a.ip2 < b.ip1 || b.ip2 < a.ip1) {
            // separate ranges, so go to the next pair
            i = j;
            j++;
        } else {
            // merge b into a
            a.ip1 = (a.ip1 < b.ip1) ? a.ip1 : b.ip1;
            a.ip2 = (a.ip2 > b.ip2) ? a.ip2 : b.ip2;

            // remove b
            j = input.erase(j);
        }
    }
}

void ConvertThread::writeOutput()
{
    if (input.count() == 0) {
        failure_reason = i18n("There are no IP addresses to convert in %1", txt_file);
        return;
    }

    sort(); // sort the block
    merge(); // merge neighbouring blocks

    QFile target(dat_file);
    if (!target.open(QIODevice::WriteOnly)) {
        Out(SYS_IPF | LOG_IMPORTANT) << "Unable to open file for writing" << endl;
        failure_reason = i18n("Cannot open %1: %2", dat_file, QString::fromLatin1(strerror(errno)));
        return;
    }

    Out(SYS_IPF | LOG_NOTICE) << "Loading finished, starting conversion..." << endl;
    dlg->message(i18n("Converting..."));

    int i = 0;
    int tot = input.count();
    for (const IPBlock &block : qAsConst(input)) {
        dlg->progress(i, tot);
        target.write((char *)&block, sizeof(IPBlock));
        if (abort) {
            return;
        }
        i++;
    }
}
}
