/***************************************************************************
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
***************************************************************************/

#include <QApplication>
#include <QPainter>
#include <QPushButton>
#include <KIconLoader>

#include "scriptdelegate.h"
#include "scriptmodel.h"


#define MARGIN 5

namespace kt
{
    ScriptDelegate::ScriptDelegate(QAbstractItemView* parent)
        : KWidgetItemDelegate(parent, parent), check_box(new QCheckBox), push_button(new QPushButton)
    {
    }

    ScriptDelegate::~ScriptDelegate()
    {
        delete check_box;
        delete push_button;
    }

    QFont ScriptDelegate::titleFont(const QFont& base) const
    {
        QFont font(base);
        font.setBold(true);
        return font;
    }

    void ScriptDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        if (!index.isValid())
            return;

        int x_offset = check_box->sizeHint().width();

        painter->save();
        QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, 0);

        int iconSize = option.rect.height() - MARGIN * 2;

        QString icon = index.model()->data(index, Qt::DecorationRole).toString();
        KIconLoader::States state = option.state & QStyle::State_Enabled ?
                                    KIconLoader::DefaultState : KIconLoader::DisabledState;
        QPixmap pixmap = KIconLoader::global()->loadIcon(icon, KIconLoader::Desktop, iconSize, state);

        int x = MARGIN + option.rect.left() + x_offset;
        painter->drawPixmap(QRect(x, MARGIN + option.rect.top(), iconSize, iconSize), pixmap, QRect(0, 0, iconSize, iconSize));

        x = MARGIN * 2 + iconSize + option.rect.left() + x_offset;
        QRect contentsRect(x, MARGIN + option.rect.top(), option.rect.width() - MARGIN * 3 - iconSize - x_offset, option.rect.height() - MARGIN * 2);

        int lessHorizontalSpace = MARGIN * 2 + push_button->sizeHint().width();
        contentsRect.setWidth(contentsRect.width() - lessHorizontalSpace);

        QPalette::ColorGroup cg = QPalette::Active;
        if (!(option.state & QStyle::State_Enabled))
            cg = QPalette::Inactive;


        if (option.state & QStyle::State_Selected)
            painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
        else
            painter->setPen(option.palette.color(cg, QPalette::WindowText));

        painter->save();
        painter->save();
        QFont font = titleFont(option.font);
        QFontMetrics fmTitle(font);
        painter->setFont(font);
        QString text = index.model()->data(index, Qt::DisplayRole).toString();
        painter->drawText(contentsRect, Qt::AlignLeft | Qt::AlignTop, fmTitle.elidedText(text, Qt::ElideRight, contentsRect.width()));
        painter->restore();

        QString comment = index.model()->data(index, ScriptModel::CommentRole).toString();
        painter->drawText(contentsRect, Qt::AlignLeft | Qt::AlignBottom, option.fontMetrics.elidedText(comment, Qt::ElideRight, contentsRect.width()));

        painter->restore();
        painter->restore();
    }

    QSize ScriptDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        QFont font = titleFont(option.font);
        QFontMetrics fm(font);

        int w = qMax(fm.width(index.model()->data(index, Qt::DisplayRole).toString()),
                     option.fontMetrics.width(index.model()->data(index, ScriptModel::CommentRole).toString()));
        int h = qMax(KIconLoader::SizeMedium + MARGIN * 2, fm.height() + option.fontMetrics.height() + MARGIN * 2);
        return QSize(w + KIconLoader::SizeMedium, h);
    }

    QList< QWidget* > ScriptDelegate::createItemWidgets(const QModelIndex& index) const
    {
        Q_UNUSED(index)
        QList<QWidget*> widgets;

        QCheckBox* enabled_check = new QCheckBox;
        connect(enabled_check, &QCheckBox::clicked, this, &ScriptDelegate::toggled);

        QPushButton* about_button = new QPushButton;
        about_button->setIcon(QIcon::fromTheme(QStringLiteral("dialog-information")));
        connect(about_button, &QPushButton::clicked, this, &ScriptDelegate::aboutClicked);

        QPushButton* configure_button = new QPushButton;
        configure_button->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
        connect(configure_button, &QPushButton::clicked, this, &ScriptDelegate::settingsClicked);

        QList<QEvent::Type> blocked;
        blocked << QEvent::MouseButtonPress << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick;
        setBlockedEventTypes(enabled_check, blocked);
        setBlockedEventTypes(about_button, blocked);
        setBlockedEventTypes(configure_button, blocked);

        widgets << enabled_check << configure_button << about_button;
        return widgets;
    }

    void ScriptDelegate::updateItemWidgets(const QList< QWidget* > widgets, const QStyleOptionViewItem& option, const QPersistentModelIndex& index) const
    {
        QCheckBox* check_box = static_cast<QCheckBox*>(widgets[0]);
        check_box->resize(check_box->sizeHint());
        int x = MARGIN;
        check_box->move(x, option.rect.height() / 2 - check_box->sizeHint().height() / 2);

        QPushButton* about_button = static_cast<QPushButton*>(widgets[2]);
        QSize about_size_hint = about_button->sizeHint();
        about_button->resize(about_size_hint);
        x = option.rect.width() - MARGIN - about_size_hint.width();
        about_button->move(x, option.rect.height() / 2 - about_size_hint.height() / 2);

        QPushButton* configure_button = static_cast<QPushButton*>(widgets[1]);
        QSize configure_size_hint = configure_button->sizeHint();
        configure_button->resize(configure_size_hint);
        x = option.rect.width() - MARGIN * 2 - configure_size_hint.width() - about_size_hint.width();
        configure_button->move(x, option.rect.height() / 2 - configure_size_hint.height() / 2);

        if (!index.isValid())
        {
            check_box->setVisible(false);
            about_button->setVisible(false);
            configure_button->setVisible(false);
        }
        else
        {
            check_box->setChecked(index.model()->data(index, Qt::CheckStateRole).toBool());
            check_box->setEnabled(true);
            configure_button->setVisible(true);
            configure_button->setEnabled(index.model()->data(index, ScriptModel::ConfigurableRole).toBool());
        }
    }

    void ScriptDelegate::aboutClicked()
    {
        QModelIndex index = focusedIndex();
        QAbstractItemModel* model = (QAbstractItemModel*)index.model();
        model->setData(index, 0, ScriptModel::AboutRole);
    }

    void ScriptDelegate::settingsClicked()
    {
        QModelIndex index = focusedIndex();
        QAbstractItemModel* model = (QAbstractItemModel*)index.model();
        model->setData(index, 0, ScriptModel::ConfigureRole);
    }

    void ScriptDelegate::toggled(bool on)
    {
        QModelIndex index = focusedIndex();
        QAbstractItemModel* model = (QAbstractItemModel*)index.model();
        model->setData(index, on, Qt::CheckStateRole);
    }
}

