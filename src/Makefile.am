bin_PROGRAMS	= xmms-osd
# man_MANS	= vlm-tool.1

xmms_osd_SOURCES=xmms-osd.c gcc-compat.h

AM_CPPFLAGS	=-std=gnu99 `xosd-config --cflags` -DVERSION=\"@VERSION@\" -I. -Wall -Wextra -Werror -pedantic -g
AM_CFLAGS	=-std=gnu99 `xosd-config --cflags` -DVERSION=\"@VERSION@\" -I. -Wall -Wextra -Werror -pedantic -g
AM_LDFLAGS	=`xosd-config --libs` -g

# EXTRA_DIST=vlm-tool.1
