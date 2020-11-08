/***************************************************************************
 *   Copyright (C) 2020 by Alexander Trufanov                              *
 *   trufanovan@gmail.com                                                  *
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

#include "nodeoperations.h"

#include <QThread>

namespace kt
{

FNode* NodeOperations::getChild(FNode* root, const QString &name, bool is_dir)
{
    FNode* child = root->first_child;
    while (child && (child->name != name || child->is_dir != is_dir)) {
        child = child->next;
    }
    return child;
}

FNode* NodeOperations::addChild(FNode* root, const QString &name, bool is_dir)
{
    FNode* n = new FNode();
    n->parent = root;
    n->name = name;
    n->is_dir = is_dir;
    if (!root->first_child) {
        root->first_child = n;
    } else {
        FNode* last_child = root->first_child;
        while (last_child->next) last_child = last_child->next;
        last_child->next = n;
        n->prev = last_child;
    }
    return n;
}

void NodeOperations::removeNode(FNode* n)
{
    while (n->first_child) {
        removeNode(n->first_child);
    }

    if (n->parent) {
        if (n->parent->first_child == n) {
            n->parent->first_child = n->next;
        }
    }

    if (n->prev) {
        n->prev->next = n->next;
    }

    if (n->next) {
        n->next->prev = n->prev;
    }

    free(n);
}

FNode* NodeOperations::makePath(FNode* root, const QString &fname, bool is_dir)
{
    int idx = fname.indexOf(QLatin1Char('/'));
    FNode* existing;

    if (idx == -1) {
        existing = getChild(root, fname, is_dir);
        if (existing) return existing;
        return addChild(root, fname, is_dir);
    } else {
        existing = getChild(root, fname.left(idx), true);
        if (!existing)
            existing = addChild(root, fname.left(idx), true);
        return makePath(existing, fname.right(fname.size() - 1 - idx), is_dir);
    }
}

FNode* NodeOperations::findChild(FNode* root, const QString &fname, bool is_dir)
{
    int idx = fname.indexOf(QLatin1Char('/'));
    if (idx == -1) {
        return getChild(root, fname, is_dir);
    } else {
        FNode* n = getChild(root, fname.left(idx), true);
        if (n)
            n = findChild(n, fname.right(fname.size() - 1 - idx), is_dir);
        return n;
    }
}

void NodeOperations::fillFromDir(FNode* root, const QDir& dir)
{
    if (QThread::currentThread()->isInterruptionRequested()) {
        return;
    }

    // QStringLists must be const to suppress "warning: c++11 range-loop might detach Qt container"
    const QStringList sl_f = dir.entryList(QDir::NoDotAndDotDot | QDir::Files | QDir::Hidden);
    for (const QString& s : sl_f) {
        addChild(root, s, false);
    }

    const QStringList sl_d = dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Hidden);
    QDir next_dir;
    for (const QString& s : sl_d) {
        FNode* d = addChild(root, s, true);
        next_dir.setPath(dir.path() + QLatin1String("/") + s);
        fillFromDir(d, next_dir);
    }
}

void NodeOperations::subtractTreesOnFiles(FNode* tree1, FNode* tree2)
{
    if (QThread::currentThread()->isInterruptionRequested()) {
        return;
    }

    FNode* c = tree2->first_child;
    while (c) {
        FNode* f = getChild(tree1, c->name, c->is_dir);
        if (f) {
            if (c->is_dir)
                subtractTreesOnFiles(f, c);
            else
                removeNode(f);
        }
        c = c->next;
    }
}

void NodeOperations::pruneEmptyFolders(FNode* start_folder)
{
    FNode* c = start_folder->first_child;
    while (c) {
        if (c->is_dir)
            pruneEmptyFolders(c);
        c = c->next;
    }

    if (!start_folder->first_child) {
        removeNode(start_folder);
    }
}

void NodeOperations::pruneEmptyFolders(FNode* tree1, FNode* tree2)
{
    if (QThread::currentThread()->isInterruptionRequested()) {
        return;
    }

    FNode* c = tree2->first_child;
    while (c) {
        if (c->is_dir) {
            FNode* f = getChild(tree1, c->name, c->is_dir);
            if (f) {
                pruneEmptyFolders(f, c);
            }
        }
        c = c->next;
    }

    if (!tree2->first_child) {
        pruneEmptyFolders(tree1);
    }
}

void NodeOperations::printTree(FNode* root, const QString& path, QSet<QString>& set)
{
    if (QThread::currentThread()->isInterruptionRequested()) {
        return;
    }

    QString new_path;
    if (!root->name.isEmpty()) {
        new_path = path + QLatin1String("/") + root->name;
        set += new_path;
    }

    FNode* c = root->first_child;

    while (c) {
        if (c->is_dir) {
            printTree(c, new_path, set);
        } else {
            set += new_path + QLatin1String("/") + c->name;
        }
        c = c->next;
    }
}

void NodeOperations::printTree(FNode* root, QSet<QString>& set)
{
    QString path;
    printTree(root, path, set);
}

}
