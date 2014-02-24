#define	_GNU_SOURCE

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <memory.h>
#include <getopt.h>

#include <xmmsctrl.h>
#include <xosd.h>

#include <gcc-compat.h>

typedef	enum	states_e	{
	state_unknown	= 0,
	state_stopped	= 1,
	state_paused	= 2,
	state_playing	= 3
} states_t;

typedef	struct	dict_s	{
	char const * const	spelling;
	int const		value;
} dict_t;

static	char const	font[] = "-misc-fixed-medium-r-normal--14-130-75-75-c-70-iso8859-1";
static	unsigned const	debug = 0;
static	unsigned int const usecs = 750000;	/* 0.75 seconds		 */

static	dict_t const	vert_dict[] =	{
	{ "top",	XOSD_top	},
	{ "center",	XOSD_center	},
	{ "bottom",	XOSD_bottom	}
};

static	dict_t const	horz_dict[] =	{
	{ "left",	XOSD_left	},
	{ "middle",	XOSD_middle	},
	{ "right",	XOSD_right	}
};

static	int		vert_choice = XOSD_bottom;
static	int		vert_offset = 2;
static	int		horz_choice = XOSD_middle;
static	int		horz_offset = -900;
#if	0
static	char const *	text_color  = "#e3f6f6";
#else
static	char const *	text_color  = "#FF0010";
#endif
static	int		timeout = 18;
static	int		shadow = 1;

static	void *
xalloc(
	size_t	n
)
{
	void * const	s = malloc( n );
	if( !s )	{
		perror( "malloc" );
		exit( 1 );
	}
	return( s );
}

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

static	char *
demangle(
	char *		msg
)
{
	size_t const	len = strlen( msg );
	size_t const	need = (len*2)+1;
	char *		new = xalloc( need );
	char *		in;
	char *		out;
	int		c;

	do	{
		/* First insert spaces on lc:uc transition		 */
		in = msg;
		out = new;
		while( (c = *in++ & 0xFF) != '\0' )	{
			if( islower(c) && isupper( *in ) )	{
				*out++ = c;
				*out++ = ' ';
				*out++ = *in++;
			} else	{
				*out++ = c;
			}
			/* Did we pass artist-title barrier?		 */
			if( c == '-' )	{
				break;
			}
		}
		/* We're in the title; ensure title case		 */
		while( (c = *in++ & 0xFF) != '\0' )	{
			if( c == '_' )	{
				/* Replace underbars with spaces	 */
				c = ' ';
			}
			if(
				(isspace( c ) | ispunct( c )) &&
				islower( *in )
				)	{
				/* Find title case transition		 */
				*out++ = c;
				*out++ = toupper( *in++ );
			} else if( islower(c) & isupper( *in ) )	{
				/* Separate words denoted by caps	 */
				*out++ = c;
				*out++ = ' ';
				*out++ = *in++;
			} else	{
				*out++ = c;
			}
		}
		*out++ = c;
	} while( 0 );
	free( msg );
	return( new );
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
	int		c;

	while( (c = getopt( argc, argv, "c:" )) != EOF )	{
		switch( c )	{
		default:
			fprintf(
				stderr,
				"dunno '-%c'.\n",
				c
			);
			exit( 1 );
		case 'c':
			text_color = optarg;
			break;
		}
	}
	if( optind < argc )	{
		fprintf( stderr, "Too many arguments.\n" );
		exit( 1 );
	}
	if( debug )	{
		printf( "Version %s\n", VERSION );
	}
	session = 0;		/* This is probably crap		 */
	last_pos = -1;
	last_vol = -1;
	state = state_unknown;
	/* Set up the XOSD window					 */
	xosd_set_align( osd, horz_choice );
	xosd_set_pos( osd, vert_choice );
	xosd_set_vertical_offset( osd, vert_offset );
	xosd_set_horizontal_offset( osd, horz_offset );
	xosd_set_font( osd, font );
	xosd_set_colour( osd, text_color );
	xosd_set_timeout( osd, timeout );
	xosd_set_shadow_offset( osd, shadow );
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
					msg = demangle( msg );
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
