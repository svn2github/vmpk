#!/bin/bash
DIR=~/QtSDK/Desktop/Qt/474/gcc/bin/
LUPDATE=$DIR/lupdate
LRELEASE=$DIR/lrelease
$LUPDATE -no-obsolete *.cpp qml/*.qml -ts i18n/*.ts
$LRELEASE i18n/*.ts
