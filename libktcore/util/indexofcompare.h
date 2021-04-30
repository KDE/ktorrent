/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_INDEXOFCOMPARE_HH
#define KT_INDEXOFCOMPARE_HH

namespace kt
{
template<class Container, class Item> struct IndexOfCompare {
    IndexOfCompare(Container *container)
        : container(container)
    {
    }

    bool operator()(Item *a, Item *b)
    {
        return container->indexOf(a) < container->indexOf(b);
    }

    Container *container;
};
}

#endif
