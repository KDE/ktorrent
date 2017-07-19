/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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

#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QPushButton>
#include <QVBoxLayout>

#include <KConfigGroup>
#include <KMessageBox>

#include <interfaces/coreinterface.h>
#include <groups/groupmanager.h>
#include "filtereditor.h"
#include "filter.h"
#include "filterlist.h"
#include "feedlist.h"
#include "feedwidgetmodel.h"

namespace kt
{

    FilterEditor::FilterEditor(Filter* filter, FilterList* filters, FeedList* feeds, CoreInterface* core, QWidget* parent)
        : QDialog(parent), filter(filter), core(core), feeds(feeds), filters(filters)
    {
        QWidget *mainWidget = new QWidget(this);
        QVBoxLayout *mainLayout = new QVBoxLayout;
        setLayout(mainLayout);
        mainLayout->addWidget(mainWidget);
        setupUi(mainWidget);
        setWindowTitle(i18n("Edit Filter"));
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
        okButton = buttonBox->button(QDialogButtonBox::Ok);
        okButton->setDefault(true);
        okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &FilterEditor::onOK);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &FilterEditor::reject);
        mainLayout->addWidget(buttonBox);

        m_name->setText(filter->filterName());
        m_match_case_sensitive->setChecked(filter->caseSensitive());
        m_all_words_must_match->setChecked(filter->allWordMatchesMustMatch());
        m_exclusion_case_sensitive->setChecked(filter->exclusionCaseSensitive());
        m_exclusion_all_must_match->setChecked(filter->exclusionAllMustMatch());
        m_use_se_matching->setChecked(filter->useSeasonAndEpisodeMatching());
        m_seasons->setEnabled(filter->useSeasonAndEpisodeMatching());
        m_seasons->setText(filter->seasonsToString());
        m_episodes->setEnabled(filter->useSeasonAndEpisodeMatching());
        m_episodes->setText(filter->episodesToString());
        m_download_matches->setChecked(filter->downloadMatching());
        m_download_non_matches->setChecked(filter->downloadNonMatching());
        m_se_no_duplicates->setEnabled(filter->useSeasonAndEpisodeMatching());
        m_se_no_duplicates->setChecked(filter->noDuplicateSeasonAndEpisodeMatches());

        QString group = filter->group();
        GroupManager* gman = core->getGroupManager();
        QStringList groups = gman->customGroupNames();

        m_add_to_group->setChecked(!group.isEmpty() && groups.count() > 0);
        m_add_to_group->setEnabled(groups.count() > 0);
        m_group->setEnabled(!group.isEmpty() && groups.count() > 0);
        m_group->addItems(groups);
        if (!group.isEmpty())
            m_group->setCurrentIndex(groups.indexOf(group));


        QString dl = filter->downloadLocation();
        m_use_custom_download_location->setChecked(!dl.isEmpty());
        m_custom_download_location->setEnabled(!dl.isEmpty());
        if (!dl.isEmpty())
            m_custom_download_location->setUrl(QUrl(dl));
        m_custom_download_location->setMode(KFile::Directory);

        QString mloc = filter->moveOnCompletionLocation();
        m_move_on_completion->setChecked(!mloc.isEmpty());
        m_move_on_completion_location->setEnabled(!mloc.isEmpty());
        if (!mloc.isEmpty())
            m_move_on_completion_location->setUrl(QUrl(mloc));
        m_move_on_completion_location->setMode(KFile::Directory);

        m_silently->setChecked(filter->openSilently());

        QList<QRegExp> re = filter->wordMatches();
        QStringList items;
        foreach (const QRegExp& r, re)
        {
            items.append(r.pattern());
        }

        m_word_matches->setItems(items);
        m_reg_exp_syntax->setChecked(filter->useRegularExpressions());

        re = filter->exclusionPatterns();
        items.clear();
        foreach (const QRegExp& r, re)
        {
            items.append(r.pattern());
        }

        m_exclusion_patterns->setItems(items);
        m_exclusion_reg_exp->setChecked(filter->exclusionUseRegularExpressions());

        connect(m_name, &QLineEdit::textChanged, this, &FilterEditor::checkOKButton);
        connect(m_seasons, &QLineEdit::textChanged, this, &FilterEditor::checkOKButton);
        connect(m_episodes, &QLineEdit::textChanged, this, &FilterEditor::checkOKButton);
        connect(m_word_matches, &KEditListWidget::added, this, &FilterEditor::checkOKButton);
        connect(m_word_matches, &KEditListWidget::changed, this, &FilterEditor::checkOKButton);
        connect(m_word_matches, &KEditListWidget::removed, this, &FilterEditor::checkOKButton);
        connect(m_use_se_matching, &QCheckBox::stateChanged, this, &FilterEditor::checkOKButton);
        okButton->setEnabled(okIsPossible());

        m_feed->setModel(feeds);
        m_test->setEnabled(feeds->rowCount(QModelIndex()) > 0);
        m_test_results->setEnabled(feeds->rowCount(QModelIndex()) > 0);
        connect(m_test, &QPushButton::clicked, this, &FilterEditor::test);
        test_model = 0;
        test_filter = new Filter();

        QHeaderView* hv = m_test_results->header();
        hv->setSectionResizeMode(QHeaderView::ResizeToContents);
    }


    FilterEditor::~FilterEditor()
    {
        delete test_filter;
    }

    void FilterEditor::test()
    {
        Feed* f = feeds->feedForIndex(feeds->index(m_feed->currentIndex(), 0));
        if (!f)
            return;

        applyOnFilter(test_filter);
        if (!test_model)
        {
            test_model = new FeedWidgetModel(this);
            test_model->setCurrentFeed(f);
            filter_model = new TestFilterModel(test_filter, test_model, this);
            m_test_results->setModel(filter_model);
        }
        else
        {
            if (test_model->currentFeed() != f)
                test_model->setCurrentFeed(f);

            test_filter->startMatching();
            filter_model->invalidate();
        }
    }

    void FilterEditor::checkOKButton()
    {
        okButton->setEnabled(okIsPossible());
    }

    bool FilterEditor::okIsPossible()
    {
        if (m_name->text().isEmpty())
            return false;

        if (m_word_matches->count() == 0)
            return false;

        if (m_use_se_matching->isChecked())
        {
            if (!Filter::validSeasonOrEpisodeString(m_seasons->text()) ||
                    !Filter::validSeasonOrEpisodeString(m_episodes->text()))
                return false;
        }

        return true;
    }

    void FilterEditor::applyOnFilter(Filter* f)
    {
        f->setFilterName(m_name->text());
        f->setCaseSensitive(m_match_case_sensitive->isChecked());
        f->setAllWordMatchesMustMatch(m_all_words_must_match->isChecked());
        f->setExclusionCaseSensitive(m_exclusion_case_sensitive->isChecked());
        f->setExclusionAllMustMatch(m_exclusion_all_must_match->isChecked());

        f->setSeasonAndEpisodeMatching(m_use_se_matching->isChecked());
        f->setSeasons(m_seasons->text());
        f->setEpisodes(m_episodes->text());

        f->setDownloadMatching(m_download_matches->isChecked());
        f->setDownloadNonMatching(m_download_non_matches->isChecked());
        f->setNoDuplicateSeasonAndEpisodeMatches(m_se_no_duplicates->isChecked());

        if (m_add_to_group->isChecked())
            f->setGroup(m_group->currentText());
        else
            f->setGroup(QString());

        if (m_use_custom_download_location->isChecked())
            f->setDownloadLocation(m_custom_download_location->url().toLocalFile());
        else
            f->setDownloadLocation(QString());

        if (m_move_on_completion->isChecked())
            f->setMoveOnCompletionLocation(m_move_on_completion_location->url().toLocalFile());
        else
            f->setMoveOnCompletionLocation(QString());

        f->setOpenSilently(m_silently->isChecked());

        f->clearWordMatches();
        for (int i = 0; i < m_word_matches->count(); i++)
        {
            QString p = m_word_matches->text(i);
            f->addWordMatch(QRegExp(p, filter->caseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive));
        }
        f->setUseRegularExpressions(m_reg_exp_syntax->isChecked());

        f->clearExclusionPatterns();
        for (int i = 0; i < m_exclusion_patterns->count(); i++)
        {
            QString p = m_exclusion_patterns->text(i);
            f->addExclusionPattern(QRegExp(p, filter->exclusionCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive));
        }
        f->setExclusionUseRegularExpressions(m_exclusion_reg_exp->isChecked());
    }

    void FilterEditor::onOK()
    {
        Filter* tmp = filters->filterByName(m_name->text());
        if (tmp && tmp != filter)
        {
            KMessageBox::error(this, i18n("There already is a filter named %1, filter names must be unique.", m_name->text()));
            return;
        }
        applyOnFilter(filter);
        accept();
    }

    ////////////////////////////////////////

    TestFilterModel::TestFilterModel(Filter* filter, FeedWidgetModel* source, QObject* parent) : QSortFilterProxyModel(parent), filter(filter), feed_model(source)
    {
        setSourceModel(source);
    }

    TestFilterModel::~TestFilterModel()
    {}

    bool TestFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
    {
        Syndication::ItemPtr item = feed_model->itemForIndex(feed_model->index(source_row, 0, source_parent));
        if (!item)
            return true;
        else
            return filter->match(item);
    }
}
