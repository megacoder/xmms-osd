#!/bin/sh
if [ -d .git ]; then
	git clean -d -f
else
	rm -f Makefile Makefile.in src/Makefile src/Makefile.in
	rm -f xmms-osd.spec configure
fi
markdown2 README.md | tee README.html | lynx -dump -stdin >README
mkdir -p m4
autoreconf -fvim -I m4
./configure
make dist
