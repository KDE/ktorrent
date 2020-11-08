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

#ifndef NODE_OPERATIONS_H
#define NODE_OPERATIONS_H
#include <QString>
#include <QDir>
#include <QSet>


/**
 * A structure to represent a filetree in memory and a set of operations on it
 */
namespace kt
{

/**
 * A node in a linked list which represents directory or file
 */
struct FNode {
    FNode()
    {
        parent = prev = next = first_child = nullptr;
    }

    QString name;
    bool is_dir;

    FNode* parent;
    FNode* prev;
    FNode* next;
    FNode* first_child;
};

class NodeOperations
{
public:

    /**
     * Find a child node with a specified name (not recursive).
     *
     * @param root   A parent node.
     * @param name   A node name to find.
     * @param is_dir Are we looking for directory or file.
     *
     * @return pointer to node found or nullptr otherwise.
     */
    static FNode* getChild(FNode* root, const QString& name, bool is_dir);

    /**
     * Create a child node. Does not check if node already exists.
     *
     * @param root   A parent node.
     * @param name   A new node name.
     * @param is_dir Is new node a directory.
     *
     * @return pointer to new node.
     */
    static FNode* addChild(FNode* root, const QString& name, bool is_dir);

    /**
     * Remove and destroy node and all its children (recursive).
     * @param n   A node to remove.
     */
    static void removeNode(FNode* n);

    /**
     * Creates a subtree that represents a given filepath starting
     * from the given root node. Performs check for already existing
     * nodes and reuses them. Returns pointer to the node that
     * represents last file or folder in filepath.
     *
     * @param root   A parent node.
     * @param fname  A filepath to file or directory.
     * @param is_dir Is filepath points to directory or file.
     *
     * @return pointer to the node that represents last file or
     *         folder in filepath
     */
    static FNode* makePath(FNode* root, const QString& fname, bool is_dir);

    /**
     * Find a child node that corresponds to the folder or file with
     * a given filepath.
     *
     * @param root   A node to start search.
     * @param fname  A filepath to search.
     * @param is_dir Are we looking for directory or file.
     *
     * @return pointer to node found or nullptr otherwise.
     */
    static FNode* findChild(FNode* root, const QString& fname, bool is_dir);

    /**
     * Creates a subtree that represents a content of a directory
     * including subfolders. Does not check if node already exists.
     *
     * @param root   A parent node.
     * @param dir    A QDir that is set to directory whose content
     *               should be represented under the root.
     */
    static void fillFromDir(FNode* root, const QDir& dir);

    /**
     * Removes all file nodes in tree1 that are exist in tree2.
     * Folder nodes are ignored.
     *
     * @param tree1  Pointer to parent node of first filetree.
     * @param tree2  Pointer to parent node of subtracted filetree.
     */
    static void subtractTreesOnFiles(FNode* tree1, FNode* tree2);

    /**
     * Removes all folder nodes that contain no file
     * children nodes using tree2 as a mask. Required to remove
     * empty folders (and subfolders) that corresponds to the
     * torrents folders.
     *
     * @param tree1  Pointer to parent node of first filetree.
     * @param tree2  Pointer to parent node of mask filetree.
     */
    static void pruneEmptyFolders(FNode* tree1, FNode* tree2);

    /**
     * Add a filepath to every node in tree to the set.
     *
     * @param root   Pointer to parent node of filetree.
     * @param set    Reference to set that shall keep resulting
     *               filepaths.
     */
    static void printTree(FNode* root, QSet<QString>& set);

private:
    static void pruneEmptyFolders(FNode* start_folder);
    static void printTree(FNode* root, const QString& path, QSet<QString>& set);
};
}
#endif // NODE_OPERATIONS_H
