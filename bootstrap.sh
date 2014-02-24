#!/bin/sh
markdown2 README.md | tee README.html | lynx -dump -stdin >README
mkdir -p m4
autoreconf -fvim -I m4
./configure
make
make dist
rm -rf RPM
mkdir -p RPM/{SOURCES,RPMS,SRPMS,BUILD,SPECS}
rpmbuild								\
	-D"_topdir ${PWD}/RPM"						\
	-D"_sourcedir ${PWD}"						\
	-ba								\
	xmms-osd.spec
