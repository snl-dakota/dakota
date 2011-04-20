#!/bin/sh
#
# Analysis Driver Script for testing 3 piece file management
# Mike Eldred, 3/26/97
# Bourne shell changes, Shane Brown, 3/16/05
#
# cut # from "tb.in.#"
num=`echo $1 | cut -c 7-`
cd workdir.$num
#sleep 2
text_book tb.in tb.out
