SUBDIRS=src

EXTRA_DIST=AUTHORS COPYING ChangeLog INSTALL NEWS README README.md     \
	 README.html TODO bootstrap.sh configure.ac install-sh missing \
	 LICENSE xmms-osd.1

.PHONY:	.FORCE

README README.html: README.md .FORCE
	markdown2 <README.md | tee README.html | lynx -dump -stdin >README

man_MANS = xmms-osd.1
