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

typedef	enum	states_e	{
	state_unknown	= 0,
	state_stopped	= 1,
	state_paused	= 2,
	state_playing	= 3
} states_t;

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
		char *	eos;
		/* Trim leading whitespace				 */
		for( ; *s && isspace( *s ); ++s );
		/* Trim trailing whitespace				 */
		eos = s + strlen( s );
		for(
			eos = s + strlen( s );
			(eos > s) && isspace( eos[-1] );
			*--eos = '\0'
		);
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
	states_t	state;

	if( debug )	{
		printf( "Version %s\n", VERSION );
	}
	session = 0;		/* This is probably crap		 */
	last_pos = -1;
	last_vol = -1;
	state = state_unknown;
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
		char *		msg;

		msg = NULL;
		do	{
			if( xmms_remote_is_playing( session ) )	{
				gint		pos;
				gint		vol;

				/* First, check if paused		 */
				if( xmms_remote_is_paused( session ) )	{
					/* Is pausing new?		 */
					if( state != state_paused )	{
						state = state_paused;
						msg = strdup( "-[pause]-" );
					}
					break;
				}
				/* Not paused now, was it?		 */
				if( state == state_paused )	{
					/* Force title display		 */
					last_pos = -1;
				}
				state = state_playing;
				pos = xmms_remote_get_playlist_pos( session );
				vol = xmms_remote_get_main_volume( session );
				if( pos != last_pos )	{
					last_pos = pos;
					msg = xmms_remote_get_playlist_title(
								session,
								pos
					);
					last_vol = vol;
					break;
				}
				if( vol != last_vol )	{
					char	buffer[ BUFSIZ + 1 ];

					last_vol = vol;
					sprintf( buffer, "-[%02d%%]-", vol );
					msg = strdup( buffer );
					break;
				}
			} else	{
				/* Not playing, see if it was last time	 */
				if( state == state_playing )	{
					state = state_stopped;
					msg = strdup( "-[stop]-" );
					break;
				}
			}
		} while( 0 );
		if( msg )	{
			display( osd, "%s", msg );
			free( msg );
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
