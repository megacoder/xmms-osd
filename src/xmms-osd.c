#define	_GNU_SOURCE

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#include <xmmsctrl.h>
#include <xosd.h>

#include <gcc-compat.h>

static	char const	font[] = "-misc-fixed-medium-r-normal--13-120-75-75-c-70-iso8859-1";
static	unsigned const	debug = 0;
static	unsigned int const usecs = 750000;	/* 0.75 seconds		 */

static	void			_printf(2,3)
display(
	xosd * const		osd,
	char const *		fmt,
	...
)
{
	va_list			ap;
	char *			s;

	va_start( ap, fmt );
	if( vasprintf( &s, fmt, ap ) > 0 )	{
		/* Show our work					 */
		if( debug )	{
			puts( s );
		}
		xosd_display(
			osd,
			0,
			XOSD_string,
			s
		);
		free( s );
	}
	va_end( ap );
}

int
main(
	int		argc		_unused,
	char * *	argv		_unused
)
{
	xosd * const	osd = xosd_create( 1 );
	gint		session;
	gint		last_pos;
	gint		last_vol;

	if( debug )	{
		printf( "Version %s\n", VERSION );
	}
	session = 0;		/* This is probably crap		 */
	last_pos = -1;
	last_vol = -1;
	/* Set up the XOSD window					 */
	xosd_set_align( osd, XOSD_center );
	xosd_set_pos( osd, XOSD_bottom );
	xosd_set_vertical_offset( osd, 2 );
	xosd_set_horizontal_offset( osd, -900 );
	xosd_set_font( osd, font );
	xosd_set_colour( osd, "#e3f6f6" );
	xosd_set_timeout( osd, 18 );
	xosd_set_shadow_offset( osd, 1 );
	for( ; ; )	{
		if(xmms_remote_is_playing( session ) )	{
			gint		pos;
			gint		vol;

			pos = xmms_remote_get_playlist_pos( session );
			vol = xmms_remote_get_main_volume( session );
			if( pos != last_pos )	{
				last_pos = pos;
				char *	title = xmms_remote_get_playlist_title(
							session,
							pos
						);
				char *		bp;

				last_pos = pos;
				/* Drop trailing blanks			 */
				for(
					bp = title + strlen( title );
					(bp > title) && bp[-1] && isspace( bp[-1] );
					*--bp = '\0'
				);
				/* Step over leading blanks		 */
				for( bp = title; *bp && isspace( *bp ); ++bp );
				display( osd, "%s", bp );
				free( title );
				/* Prevent volume overwrite	 */
				last_vol = vol;
			} else if( vol != last_vol )	{
				last_vol = vol;
				display( osd, "Volume: %02d%%", vol );
			}
		}
		if(usleep( usecs ) == -1 )	{
			break;
		}
	}
	if( debug )	{
		puts( "Bye!" );
	}
	exit( 0 );
}
