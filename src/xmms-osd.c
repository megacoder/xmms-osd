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

static	char const *	me = "xmms-osd";
static	char const *	font = "-misc-fixed-medium-r-normal--14-130-75-75-c-70-iso8859-1";
static	unsigned	debug = 0;
static	unsigned int const usecs = 750000;	/* 0.75 seconds		 */

static	dict_t const	vert_dict[] =	{
	{ "top",	XOSD_top	},
	{ "middle",	XOSD_middle	},
	{ "bottom",	XOSD_bottom	},
	{ NULL,		0		}
};

static	dict_t const	horz_dict[] =	{
	{ "left",	XOSD_left	},
	{ "center",	XOSD_center	},
	{ "right",	XOSD_right	},
	{ NULL,		0		}
};

static	int		vert_choice = XOSD_bottom;
static	int		vert_offset = 2;
static	int		horz_choice = XOSD_center;
static	int		horz_offset = -900;
static	char const *	text_color  = "#A0A0FF";
static	int		timeout = 18;
static	int		shadow = 1;

static	void
usage(
	char const *	fmt,
	...
)
{
	if( fmt )	{
		va_list	ap;

		fprintf( stderr, "%s: ", me );
		va_start( ap, fmt );
		vfprintf( stderr, fmt, ap );
		va_end( ap );
		fprintf( stderr, ".\n" );
	}
	fprintf(
		stderr,
		"usage: %s"
		" [-H hoff]"
		" [-V voff]"
		" [-c color]"
		" [-f font]"
		" [-h hpos]"
		" [-v vpos]"
		"\n",
		me
	);
	fprintf(
		stderr,
		"  o Text color can be specified as a color name or hex value.\n"
	);
	fprintf(
		stderr,
		"  o Horizonal positions are 'left', 'middle', or 'right'.\n"
	);
	fprintf(
		stderr,
		"  o Vertical positions are 'top', 'center', or 'bottom'.\n"
	);
}

static	int
search_dict(
	dict_t const *	dict,
	char const *	s
)
{
	do	{
		dict_t const *	thumb;

		for( thumb = dict; thumb->spelling; ++thumb )	{
			if( !strcmp( thumb->spelling, s ) )	{
				return( thumb->value );
			}
		}
	} while( 0 );
	usage( "spelling not recognized '%s'", s );
	exit( 1 );
}

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

	while( (c = getopt( argc, argv, "c:Df:h:H:v:V:" )) != EOF )	{
		switch( c )	{
		default:
			usage( NULL );
			exit( 1 );
		case 'c':
			text_color = optarg;
			break;
		case 'D':
			++debug;
			break;
		case 'f':
			font = optarg;
			break;
		case 'h':
			horz_choice = search_dict( horz_dict, optarg );
			break;
		case 'H':
			horz_offset = strtol( optarg, 0, 10 );
			break;
		case 'v':
			vert_choice = search_dict( vert_dict, optarg );
			break;
		case 'V':
			vert_offset = strtol( optarg, 0, 10 );
			break;
		}
	}
	if( optind < argc )	{
		usage( "too many arguments" );
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
	xosd_set_horizontal_offset( osd, horz_offset );
	xosd_set_pos( osd, vert_choice );
	xosd_set_vertical_offset( osd, vert_offset );
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
					char	buffer[ BUFSIZ + 1 ];
					char *	info;

					last_pos = pos;
					info = xmms_remote_get_playlist_title(
						session,
						pos
					);
					sprintf(
						buffer,
						"(%u) %s",
						(unsigned) pos,
						demangle( info )
					);
					msg = strdup( buffer );
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
