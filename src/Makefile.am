bin_PROGRAMS	= xmms-osd
# man_MANS	= vlm-tool.1

xmms_osd_SOURCES=xmms-osd.c gcc-compat.h

AM_CPPFLAGS	=-std=gnu99 `xmms-config --cflags` `xosd-config --cflags` -DVERSION=\"@VERSION@\" -I. -Wall -Wextra -Werror -pedantic -g
AM_CFLAGS	=-std=gnu99 `xmms-config --cflags` `xosd-config --cflags` -DVERSION=\"@VERSION@\" -I. -Wall -Wextra -Werror -pedantic -g
AM_LDFLAGS	=`xmms-config --libs` `xosd-config --libs` -g

# EXTRA_DIST=vlm-tool.1
