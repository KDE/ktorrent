/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "grouppolicydlg.h"
#include <groups/group.h>

namespace kt
{
GroupPolicyDlg::GroupPolicyDlg(Group *group, QWidget *parent)
    : QDialog(parent)
    , group(group)
{
    setupUi(this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &GroupPolicyDlg::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &GroupPolicyDlg::reject);
    setWindowTitle(i18n("Policy for the %1 group", group->groupName()));

    const Group::Policy &p = group->groupPolicy();
    m_default_location_enabled->setChecked(!p.default_save_location.isEmpty());
    m_default_location->setEnabled(!p.default_save_location.isEmpty());
    m_default_location->setUrl(QUrl::fromLocalFile(p.default_save_location));
    m_default_location->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);

    m_default_move_on_completion_enabled->setChecked(!p.default_move_on_completion_location.isEmpty());
    m_default_move_on_completion_location->setEnabled(!p.default_move_on_completion_location.isEmpty());
    m_default_move_on_completion_location->setUrl(QUrl::fromLocalFile(p.default_move_on_completion_location));
    m_default_move_on_completion_location->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);

    m_only_new->setChecked(p.only_apply_on_new_torrents);
    m_max_share_ratio->setValue(p.max_share_ratio);
    m_max_seed_time->setValue(p.max_seed_time);
    m_max_upload_rate->setValue(p.max_upload_rate);
    m_max_download_rate->setValue(p.max_download_rate);
}

GroupPolicyDlg::~GroupPolicyDlg()
{
}

void GroupPolicyDlg::accept()
{
    Group::Policy p;
    if (m_default_location_enabled->isChecked() && m_default_location->url().isValid())
        p.default_save_location = m_default_location->url().toDisplayString(QUrl::PreferLocalFile);

    if (m_default_move_on_completion_enabled->isChecked() && m_default_move_on_completion_location->url().isValid())
        p.default_move_on_completion_location = m_default_move_on_completion_location->url().toDisplayString(QUrl::PreferLocalFile);

    p.only_apply_on_new_torrents = m_only_new->isChecked();
    p.max_share_ratio = m_max_share_ratio->value();
    p.max_seed_time = m_max_seed_time->value();
    p.max_upload_rate = m_max_upload_rate->value();
    p.max_download_rate = m_max_download_rate->value();
    group->setGroupPolicy(p);
    QDialog::accept();
}

}
