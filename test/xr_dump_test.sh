#!/usr/bin/env bash

TOP_DIR=$1

xrdb -retain -load ${TOP_DIR}/doc/test_xr.txt
rofi -config ${TOP_DIR}/doc/test_xr.txt -dump-xresources > temp.txt

if ! diff temp.txt ${TOP_DIR}/doc/test_xr.txt > /dev/null
then
    echo "Dump xresources does not match."
    exit 1;
fi

exit ${RETV}
