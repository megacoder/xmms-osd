#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

#include <xosd.h>

#include <gcc-compat.h>

static	char const	font[] = "-misc-fixed-medium-r-normal--13-120-75-75-c-70-iso8859-1";

int
main(
	int		argc		_unused,
	char * *	argv		_unused
)
{
	xosd * const	osd = xosd_create( 1 );
	char		path[ PATH_MAX + 1 ];
	FILE *		fyle;
	char		buffer[ BUFSIZ + 1 ];
	int		c;
	char *		bp;

	printf( "Version %s\n", VERSION );
	/* Set up the XOSD window					 */
	xosd_set_align( osd, XOSD_center );
	xosd_set_pos( osd, XOSD_bottom );
	xosd_set_vertical_offset( osd, 2 );
	xosd_set_horizontal_offset( osd, -900 );
	xosd_set_font( osd, font );
	xosd_set_colour( osd, "#e3f6f6" );
	xosd_set_timeout( osd, 18 );
	xosd_set_shadow_offset( osd, 1 );
	/* Open pipe and show it by lines				 */
	sprintf( path, "%s/%s", getenv( "HOME" ), "xmms-fifo" );
	/* Daemonize and start doing "the right thing" (tm)		 */
	if( daemon( 0, 0 ) )	{
		perror( "daemon( 0, 0 )" );
		exit(1);
	}
Again:
	fyle = fopen( path, "rt" );
	if( ! fyle )	{
		perror( path );
		exit( 1 );
	}
	c = ' ';
	bp = buffer;
	for( ; ; )	{
		/* Read until we get a non-blank			 */
		while( isspace( c ) )	{
			c = fgetc( fyle );
			if( c == -1 )	{
				fclose( fyle );
				goto Again;
			}
		}
		/* Read until we get a newline				 */
		do	{
			*bp++ = c;
			c = fgetc( fyle );
			if( c == -1 )	{
				puts( "Goner!" );
				exit( 0 );
			}
		} while( (c != '\n') && (bp < buffer+sizeof(buffer)) );
		*bp = '\0';
		puts( buffer );
		xosd_display(
			osd,
			0,
			XOSD_string,
			buffer
		);
	}
	puts( "Bye!" );
	exit( 0 );
}
