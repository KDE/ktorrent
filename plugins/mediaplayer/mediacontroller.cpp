/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
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
 ***************************************************************************/

#include "mediacontroller.h"

#include <QBoxLayout>
#include <QFile>

#include <KLocalizedString>

#include <taglib/tag.h>
#include <taglib/fileref.h>
#include "mediaplayer.h"

namespace kt
{
    static QString t2q(const TagLib::String& t)
    {
        return QString::fromWCharArray((const wchar_t*)t.toCWString(), t.length());
    }

    MediaController::MediaController(kt::MediaPlayer* player, KActionCollection* ac, QWidget* parent) : QWidget(parent)
    {
        setupUi(this);

        info_label->setText(i18n("Ready to play"));
        seek_slider->setMediaObject(player->media0bject());
        volume->setAudioOutput(player->output());
        volume->setOrientation(Qt::Horizontal);


        connect(player, SIGNAL(stopped()), this, SLOT(stopped()));
        connect(player, SIGNAL(playing(MediaFileRef)), this, SLOT(playing(MediaFileRef)));

        play->setDefaultAction(ac->action(QStringLiteral("media_play")));
        play->setAutoRaise(true);
        pause->setDefaultAction(ac->action(QStringLiteral("media_pause")));
        pause->setAutoRaise(true);
        stop->setDefaultAction(ac->action(QStringLiteral("media_stop")));
        stop->setAutoRaise(true);
        prev->setDefaultAction(ac->action(QStringLiteral("media_prev")));
        prev->setAutoRaise(true);
        next->setDefaultAction(ac->action(QStringLiteral("media_next")));
        next->setAutoRaise(true);

        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    }


    MediaController::~MediaController()
    {

    }

    void MediaController::playing(const MediaFileRef& file)
    {
        if (file.path().isEmpty())
        {
            stopped();
        }
        else
        {
            current_file = file;
            metaDataChanged();
        }
    }

    void MediaController::stopped()
    {
        info_label->setText(i18n("Ready to play"));
        current_file = MediaFileRef(QString());
    }


    void MediaController::metaDataChanged()
    {
        QString extra_data;
        QByteArray encoded = QFile::encodeName(current_file.path());
        TagLib::FileRef ref(encoded.data(), true, TagLib::AudioProperties::Fast);
        if (ref.isNull())
        {
            info_label->setText(i18n("Playing: <b>%1</b>", current_file.name()));
            return;
        }

        TagLib::Tag* tag = ref.tag();
        if (!tag)
        {
            info_label->setText(i18n("Playing: <b>%1</b>", current_file.name()));
            return;
        }

        QString artist = t2q(tag->artist());
        QString title =  t2q(tag->title());
        QString album = t2q(tag->album());

        bool has_artist = !artist.isEmpty();
        bool has_title = !title.isEmpty();
        bool has_album = !album.isEmpty();

        if (has_artist && has_title && has_album)
        {
            extra_data = i18n("<b>%2</b> - <b>%1</b> (Album: <b>%3</b>)", title, artist, album);
            info_label->setText(extra_data);
        }
        else if (has_title && has_artist)
        {
            extra_data = i18n("<b>%2</b> - <b>%1</b>", title, artist);
            info_label->setText(extra_data);
        }
        else if (has_title)
        {
            extra_data = i18n("<b>%1</b>", title);
            info_label->setText(extra_data);
        }
        else
        {
            info_label->setText(i18n("<b>%1</b>", current_file.name()));
        }
    }

}
