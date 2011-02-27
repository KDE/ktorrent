#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.ui -o -name \*.rc -o -name \*.kcfg | grep -v "/kio-magnet"` >> rc.cpp
$XGETTEXT `find . -name \*.cpp -o -name \*.cc -o -name \*.h | grep -v "/kio-magnet"` rc.cpp -o $podir/ktorrent.pot
rm -f rc.cpp
