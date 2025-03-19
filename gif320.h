/* gif320.h
 *
 * header file for gif320 utility
 *
 * $Id: gif320.h,v 1.3 1992/11/24 16:00:30 jmason Exp $
 *
 * $Log: gif320.h,v $
 * Revision 1.3  1992/11/24  16:00:30  jmason
 * fixed an embarrassing speeling erorr.
 *
 * Revision 1.2  1992/10/30  16:29:45  jmason
 * Added VMS support; fixed bad raster errors.
 *
 * Revision 1.1  1992/10/30  11:09:43  jmason
 * Placed under CVS control.
 *
 * Revision 3.0  1992/07/04  01:27:16  sboyle
 * Initial RCS-aided revision
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "vtgraph.h"
#include "config.h"

#define min(x, y)	((x)<(y) ? (x) : (y))
#define my_gets(s)	if (gets(s) == NULL) die_messily()
#define my_tolower(c)	((isupper(c)) ? tolower(c) : (c))

typedef char bool;

#define FALSE ((bool) 0)
#define TRUE ((bool) 1)

typedef struct codestruct {
  struct codestruct *prefix;
  unsigned char first, suffix;
} codetype;
