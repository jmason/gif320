/*
 * config.h
 *
 * Various #defines.
 *
 * $Id: config.h,v 1.3 1992/11/24 16:00:28 jmason Exp $
 *
 * $Log: config.h,v $
 * Revision 1.3  1992/11/24  16:00:28  jmason
 * fixed an embarrassing speeling erorr.
 *
 * Revision 1.2  1992/10/30  16:29:41  jmason
 * Added VMS support; fixed bad raster errors.
 *
 * Revision 1.1  1992/10/30  11:09:43  jmason
 * Placed under CVS control.
 *
 * Revision 3.0  1992/07/04  01:27:16  sboyle
 * Initial RCS-aided revision
 *
 * -----------------------------------------------------------------------
 * Compile-time options: muck about with these if you like.
 *
 * DONT_USE_SIGNAL: if you're not on a platform which has <signal.h>.
 */

#undef DONT_USE_SIGNAL

/*
 * REAL_RATIO: if you want to use the images' real X:Y ratios
 *     (ie. if most of your images have square pixels).
 */

#undef REAL_RATIO

/*
 * OPTIMISE_PACK: if you want pack() to do all the checks one after the
 *     other, instead of cycling through them in a loop (ie. unrolling).
 *     This is heavy on memory and disk space, but faster. Also, some
 *     compilers may not like it. Of course, most compilers will do this
 *     anyway, given a high enough optimisation setting.
 */

#define OPTIMISE_PACK

/*
 * CHECK_OTHERS: if you want pack() to check for identical other cells,
 *     instead of just empty and full ones. This is slower, but improves
 *     your optimising ever-so-slightly.
 */

#define CHECK_OTHERS

/*
 * NO_UNIX_GLOBBING: if you don't want to allow tilde globbing as found on
 *     most UNIX systems (ie. "~/tmp" -> "/users/sboyle/tmp" or whatever).
 */

#undef NO_UNIX_GLOBBING

/*
 * SIGNALS_ARE_VOID: if signal trapping functions should be of type 'void'.
 */

#undef SIGNALS_ARE_VOID

/*
 * ON_VAX: define this if you're on a VAX/VMS system. 
 */

#undef ON_VAX

/* --------------------------------------------------------------------
 * The bit that changes from version to version.
 */

#define VERSION		"3.2"
#define AUTHOR		"doctorgonzo <jm@maths.tcd.ie>"
#define LASTMOD		"$Date: 1992/11/24 16:00:28 $"
#define BUG_ADDRESS	"<jm@maths.tcd.ie>"

/* -------------------------------------------------------------------- */

#define PROMPT		"GIF320> "

/* VT-320 soft font settings: based around a maximum of 96 definable
 * character cells and 2 groups of 6-bit-high 15-byte SIXELs for the
 * cell definition.
 *
 * scrwidth and scrheight are the maximum size of the scrmap[][] array.
 */

#define scrwidth	80
#define scrheight	24
#define cells		96
#define groups		2
#define charwidth	15
#define charheight	12
#define sixels_group	(charheight / groups)

#define XY_RATIO	(Ratio * (((float) scrwidth) / ((float) scrheight)))

/* RGB_[RGB]_INIT: the starting values of the RGB settings. */
#define RGB_R_INIT	30
#define RGB_G_INIT	40
#define RGB_B_INIT	10

/* *_VALUE_INIT: the starting values of the value thresholds. */
#define FULL_VALUE_INIT	50
#define HALF_VALUE_INIT	25

/* MESSAGE_AREA: top line of the area on the screen used for messages. */
#define MESSAGE_AREA	17

#ifdef SIGNALS_ARE_VOID
#  define sig_type void
#else
#  define sig_type int
#endif
