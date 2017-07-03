/***************************************************************************
 *   Copyright (C) 2014 by Juan Palacios                                   *
 *   jpalaciosdev@gmail.com                                                *
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

#include "magnetmanager.h"

#include <QFile>
#include <QTextStream>

#include <util/log.h>
#include <bcodec/bencoder.h>
#include <bcodec/bdecoder.h>
#include <util/error.h>
#include <bcodec/bnode.h>

using namespace bt;

namespace kt {

DownloadSlot::DownloadSlot(QObject *parent)
    : magnetIdx(-1)
    , timerDuration(0)
{
    timer = new QTimer(parent);
    timer->setSingleShot(true);
    timer->setInterval(timerDuration);
    connect(timer, &QTimer::timeout, this, &DownloadSlot::onTimeout);
}

DownloadSlot::~DownloadSlot()
{
    delete timer;
}

void DownloadSlot::setTimerDuration(unsigned int duration)
{
    timerDuration = duration;
    timer->setInterval(duration);
}

void DownloadSlot::startTimer()
{
    timer->start();
}

void DownloadSlot::stopTimer()
{
    timer->stop();
}

void DownloadSlot::reset()
{
    stopTimer();
    setTimerDuration(timerDuration);
    magnetIdx = -1;
}

void DownloadSlot::setMagnetIndex(int index)
{
    magnetIdx = index;
}

int DownloadSlot::getMagnetIndex() const
{
    return magnetIdx;
}

bool DownloadSlot::isTimerActived() const
{
    return timer->isActive();
}

bool DownloadSlot::isOccupied() const
{
    return magnetIdx < 0;
}

void DownloadSlot::onTimeout()
{
    emit timeout(magnetIdx);
}

//---------------------------------------------------


MagnetManager::MagnetManager(QObject *parent)
    : QObject(parent)
    , useSlotTimer(true)
    , timerDuration(180000)
    , usedDownloadingSlots()
    , freeDownloadingSlots()
    , magnetQueue()
    , stoppedList()
    , magnetHashes()
    , stoppedHashes()
{
    setDownloadingSlots(1);
}

MagnetManager::~MagnetManager()
{
    for (DownloadSlot* slot : qAsConst(usedDownloadingSlots))
        delete slot;

    for (DownloadSlot* slot : qAsConst(freeDownloadingSlots))
        delete slot;
}

void MagnetManager::addMagnet(const bt::MagnetLink& mlink, const kt::MagnetLinkLoadOptions& options, bool stopped)
{
    if (magnetHashes.contains(mlink.infoHash()))
        return; // Already managed, do nothing

    MagnetDownloader* md = new MagnetDownloader(mlink, options, this);
    connect(md, &MagnetDownloader::foundMetadata, this, &MagnetManager::onDownloadFinished);

    int updateIndex = 0;
    int updateCount = 0;
    if (stopped)
    {
        stoppedList.append(md);
        stoppedHashes.insert(mlink.infoHash());
        magnetHashes.insert(mlink.infoHash());

        updateIndex = magnetHashes.size() - 1;
        updateCount = 1;
    }
    else
    {
        magnetQueue.append(md);
        magnetHashes.insert(mlink.infoHash());

        int nextIndex = startNextQueuedMagnets();
        if (nextIndex >= 0)
            updateIndex = nextIndex;
        else
            updateIndex = magnetQueue.size() - 1;
        updateCount = magnetHashes.size() - updateIndex;
    }
    emit updateQueue(updateIndex, updateCount);
}

void MagnetManager::removeMagnets(bt::Uint32 idx, bt::Uint32 count)
{
    if (idx >= (Uint32) magnetHashes.size() || count < 1)
        return;

    while (count > 0 && idx < (Uint32) magnetHashes.size())
    {
        MagnetDownloader* md =  0;
        Uint32 magnetQueueSize = magnetQueue.size();
        if (idx < magnetQueueSize)
        {
            md = magnetQueue.at(idx);
            magnetQueue.removeAt(idx);
            if (md->running())
                freeDownloadSlot(idx);
        }
        else
        {
            int stoppedIdx = idx - magnetQueueSize;
            md = stoppedList.at(stoppedIdx);
            stoppedList.removeAt(stoppedIdx);
            stoppedHashes.remove(md->magnetLink().infoHash());
        }
        magnetHashes.remove(md->magnetLink().infoHash());
        md->deleteLater();

        --count;
    }

    int updateIndex = startNextQueuedMagnets();
    if (updateIndex < 0)
        updateIndex = idx;

    emit updateQueue(updateIndex, magnetHashes.size() - updateIndex);
}

void MagnetManager::start(bt::Uint32 idx, bt::Uint32 count)
{
    Uint32 magnetQueueSize = magnetQueue.size();
    if (idx + count < magnetQueueSize || count < 1)
        return;

    if (idx < magnetQueueSize) // jump to stopped magnets
    {
        int alreadyStarted = magnetQueueSize - idx;
        idx += alreadyStarted;
        count -= alreadyStarted;
    }

    Uint32 updateIndex = idx;
    Uint32 updateCount = 0;

    int stoppedIdx = idx - magnetQueueSize;
    Uint32 totalMagnets = magnetHashes.size();
    while (count > 0 && idx < totalMagnets)
    {
        MagnetDownloader* md = stoppedList.at(stoppedIdx);
        stoppedList.removeAt(stoppedIdx);
        stoppedHashes.remove(md->magnetLink().infoHash());
        magnetQueue.append(md);

        --count; ++idx; ++updateCount;
    }

    int startedIdx = startNextQueuedMagnets();
    if (startedIdx >= 0)
        updateIndex = startedIdx;

    if (updateCount > 0)
        emit updateQueue(updateIndex, updateCount);
}

void MagnetManager::stop(bt::Uint32 idx, bt::Uint32 count)
{
    Uint32 magnetQueueSize = magnetQueue.size();
    if (idx >= magnetQueueSize || count < 1)
        return;

    if (idx + count >= magnetQueueSize)
        count -= (idx + count) - magnetQueueSize; // do not include already stopped magnets

    Uint32 updateCount = count;
    Uint32 updateIndex = idx;

    while(count > 0)
    {
        MagnetDownloader* md = magnetQueue.at(idx);
        if (md->running())
        {
            md->stop();
            freeDownloadSlot(idx);
        }
        magnetQueue.removeAt(idx);
        stoppedList.append(md);
        stoppedHashes.insert(md->magnetLink().infoHash());

        --count;
    }

    int startedIdx = startNextQueuedMagnets();
    if (startedIdx >= 0)
        updateIndex = startedIdx;

    if (updateCount > 0)
        emit updateQueue(updateIndex, updateCount);
}

bool MagnetManager::isStopped(bt::Uint32 idx) const
{
    if (idx < (Uint32) magnetQueue.size())
        return false;

    return true;
}

void MagnetManager::setDownloadingSlots(bt::Uint32 count)
{
    int updateIndex = 0;
    int updateCount = 0;
    int totalSlots = usedDownloadingSlots.size() + freeDownloadingSlots.size();
    int slotsToAdd = count - totalSlots;
    if (slotsToAdd > 0) // add new slots
    {
        for (int i = 0; i < slotsToAdd; ++i)
        {
            DownloadSlot* slot = new DownloadSlot();
            slot->setTimerDuration(timerDuration);
            freeDownloadingSlots.push_back(slot);
            connect(slot, &DownloadSlot::timeout, this, &MagnetManager::onSlotTimeout);
        }
        updateIndex = startNextQueuedMagnets();
        updateCount = slotsToAdd;
    }
    else // remove slots
    {
        int slotsToRemove = qAbs(slotsToAdd);

        // try to remove free slots
        if (!freeDownloadingSlots.isEmpty())
        {
            while (slotsToRemove > 0 && !freeDownloadingSlots.isEmpty())
            {
                DownloadSlot* slot = freeDownloadingSlots.front();
                freeDownloadingSlots.pop_front();
                delete slot;

                --slotsToRemove;
            }
        }

        if (slotsToRemove == 0)
        {
            // all removed slots where unused so we don't need to emit an updateQueue signal
            updateIndex = -1;
        }
        else // remove used slots
        {
            updateIndex = usedDownloadingSlots.size() - slotsToRemove;
            if (updateIndex < 0)
                updateIndex = 0;
            updateCount = slotsToRemove;

            while (slotsToRemove > 0 && !usedDownloadingSlots.isEmpty())
            {
                DownloadSlot* slot = usedDownloadingSlots.back();
                usedDownloadingSlots.pop_back();
                slot->stopTimer();
                magnetQueue.at(slot->getMagnetIndex())->stop();
                delete slot;

                --slotsToRemove;
            }
        }
    }
    if (updateIndex >= 0)
        emit updateQueue(updateIndex, updateCount);
}

void MagnetManager::setUseSlotTimer(bool value)
{
    useSlotTimer = value;

    if (!useSlotTimer)
    {
        for (DownloadSlot* slot : qAsConst(usedDownloadingSlots))
        {
            slot->stopTimer();
            slot->setTimerDuration(timerDuration);
        }
    }
    else
    {
        for (DownloadSlot* slot : qAsConst(usedDownloadingSlots))
        {
            if (!slot->isTimerActived())
            {
                slot->setTimerDuration(timerDuration);
                slot->startTimer();
            }
        }
    }
}

void MagnetManager::setTimerDuration(bt::Uint32 duration)
{
    timerDuration = duration * 60000; // convert to milliseconds
    for (DownloadSlot* slot : qAsConst(usedDownloadingSlots))
        slot->setTimerDuration(timerDuration);

    for (DownloadSlot* slot : qAsConst(freeDownloadingSlots))
        slot->setTimerDuration(timerDuration);

    emit updateQueue(0, usedDownloadingSlots.size());
}

void MagnetManager::update()
{
    for (DownloadSlot* slot : qAsConst(usedDownloadingSlots))
        magnetQueue.at(slot->getMagnetIndex())->update();

    emit updateQueue(0, usedDownloadingSlots.size());
}

void MagnetManager::loadMagnets(const QString& file)
{
    QFile fptr(file);
    if (!fptr.open(QIODevice::ReadOnly))
    {
        Out(SYS_GEN | LOG_NOTICE) << "Failed to open " << file << " : " << fptr.errorString() << endl;
        return;
    }

    QByteArray magnet_data = fptr.readAll();
    if (magnet_data.size() == 0)
        return;

    BDecoder decoder(magnet_data, 0, false);
    BNode* node = 0;
    try
    {
        node = decoder.decode();
        if (!node || node->getType() != BNode::LIST)
            throw Error(QStringLiteral("Corrupted magnet file"));

        BListNode* ml = (BListNode*)node;
        for (Uint32 i = 0; i < ml->getNumChildren(); i++)
        {
            BDictNode* dict = ml->getDict(i);
            MagnetLink mlink(dict->getString(QByteArrayLiteral("magnet"), 0));
            MagnetLinkLoadOptions options;
            bool stopped = dict->getInt(QByteArrayLiteral("stopped")) == 1;
            options.silently = dict->getInt(QByteArrayLiteral("silent")) == 1;

            if (dict->keys().contains("group"))
                options.group = dict->getString(QByteArrayLiteral("group"), 0);
            if (dict->keys().contains("location"))
                options.location = dict->getString(QByteArrayLiteral("location"), 0);
            if (dict->keys().contains("move_on_completion"))
                options.move_on_completion = dict->getString(QByteArrayLiteral("move_on_completion"), 0);

            addMagnet(mlink, options, stopped);
        }
    }
    catch (Error& err)
    {
        Out(SYS_GEN | LOG_NOTICE) << "Failed to load " << file << " : " << err.toString() << endl;
    }
    delete node;
}

void MagnetManager::saveMagnets(const QString& file)
{
    File fptr;
    if (!fptr.open(file, QStringLiteral("wb")))
    {
        Out(SYS_GEN | LOG_NOTICE) << "Failed to open " << file << " : " << fptr.errorString() << endl;
        return;
    }

    BEncoder enc(&fptr);
    enc.beginList();

    for (MagnetDownloader* md : qAsConst(magnetQueue))
        writeEncoderInfo(enc, md);

    for (MagnetDownloader* md : qAsConst(stoppedList))
        writeEncoderInfo(enc, md);

    enc.end();
}

void MagnetManager::writeEncoderInfo(bt::BEncoder &enc, kt::MagnetDownloader* md)
{
    enc.beginDict();
    enc.write(QByteArrayLiteral("magnet"), md->magnetLink().toString().toUtf8());
    enc.write(QByteArrayLiteral("stopped"), stoppedHashes.contains(md->magnetLink().infoHash()));
    enc.write(QByteArrayLiteral("silent"), md->options.silently);
    enc.write(QByteArrayLiteral("group"), md->options.group.toUtf8());
    enc.write(QByteArrayLiteral("location"), md->options.location.toUtf8());
    enc.write(QByteArrayLiteral("move_on_completion"), md->options.move_on_completion.toUtf8());
    enc.end();
}

MagnetManager::MagnetState MagnetManager::status(bt::Uint32 idx) const
{
    Q_ASSERT(idx < (Uint32) magnetHashes.size());

    const MagnetDownloader* md = getMagnetDownloader(idx);

    if (idx >= (Uint32)magnetQueue.size())
        return STOPPED;
    else if (md->running())
        return DOWNLOADING;
    else
        return QUEUED;
}

int MagnetManager::count() const
{
    return magnetHashes.size();
}

const MagnetDownloader *MagnetManager::getMagnetDownloader(bt::Uint32 idx) const
{
    Q_ASSERT(idx < (Uint32) magnetHashes.size());

    MagnetDownloader* md = nullptr;

    Uint32 downloadQueueSize = magnetQueue.size();
    if (idx < downloadQueueSize)
        md = magnetQueue.at(idx);
    else
        md = stoppedList.at(idx - downloadQueueSize);

    return md;
}

void MagnetManager::onDownloadFinished(bt::MagnetDownloader* md, const QByteArray& data)
{
    MagnetDownloader* ktmd = (MagnetDownloader*) md;
    emit metadataDownloaded(md->magnetLink(), data, ktmd->options);

    int magnetIndex = getMagnetIndex((MagnetDownloader*) md);
    if (magnetIndex >= 0)
        removeMagnets(magnetIndex, 1);
}

void MagnetManager::onSlotTimeout(int magnetIdx)
{
    if (magnetIdx >= usedDownloadingSlots.size())
        return;

    freeDownloadSlot(magnetIdx);
    MagnetDownloader* md = magnetQueue.at(magnetIdx);
    md->stop();
    magnetQueue.removeAt(magnetIdx);
    magnetQueue.push_back(md);

    int updateIndex = startNextQueuedMagnets();
    if (updateIndex < 0)
        updateIndex = magnetIdx;

    emit updateQueue(updateIndex, magnetQueue.size() - updateIndex);
}

int MagnetManager::startNextQueuedMagnets()
{
    if (magnetQueue.empty() || freeDownloadingSlots.isEmpty())
        return -1;

    int firstStartedIdx = usedDownloadingSlots.size();
    int queued = magnetQueue.size() - firstStartedIdx;
    int magnetsToStart = qMin(freeDownloadingSlots.size(), queued);

    int nextIdx = firstStartedIdx;
    while (magnetsToStart > 0)
    {
        DownloadSlot* slot = freeDownloadingSlots.front();
        freeDownloadingSlots.pop_front();
        slot->setMagnetIndex(nextIdx);
        usedDownloadingSlots.push_back(slot);

        magnetQueue.at(nextIdx)->start();
        if (useSlotTimer)
            slot->startTimer();

        --magnetsToStart; ++nextIdx;
    }

    return firstStartedIdx;
}

void MagnetManager::freeDownloadSlot(bt::Uint32 magnetIdx)
{
    Uint32 usedDownloadingSlotsSize = usedDownloadingSlots.size();
    if (magnetIdx >= usedDownloadingSlotsSize)
        return;

    // free the slot used by magnetIdx
    DownloadSlot* slot = usedDownloadingSlots.at(magnetIdx);
    usedDownloadingSlots.removeAt(magnetIdx);
    slot->reset();
    freeDownloadingSlots.push_front(slot);

    // sync magnet indices of next slots
    --usedDownloadingSlotsSize;
    for (Uint32 i = magnetIdx; i < usedDownloadingSlotsSize; ++i)
        usedDownloadingSlots.at(i)->setMagnetIndex(i);
}

int MagnetManager::getMagnetIndex(kt::MagnetDownloader* md)
{
    if (stoppedHashes.contains(md->magnetLink().infoHash()))
    {
        int magnetIndex = magnetQueue.size() + stoppedList.indexOf(md);
        if (magnetIndex >= magnetQueue.size()) // md is inside of magnetStopList
            return magnetIndex;
    }
    else return magnetQueue.indexOf(md);

    return -1;
}

}
