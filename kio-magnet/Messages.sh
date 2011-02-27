#! /usr/bin/env bash
$EXTRACTRC *.kcfg >> rc.cpp
$XGETTEXT *.cpp  -o $podir/kio_magnet.pot
