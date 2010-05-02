#!/bin/bash

SVNROOT=svn+ssh://guisson@svn.kde.org/home/kde

svn co -N $SVNROOT/trunk/www/areas/extragear/
cd extragear
svn update inc
svn update -N apps
svn update -N apps/ktorrent
