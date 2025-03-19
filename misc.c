/* misc.c
 *
 * some little bits and pieces used here and there.
 *
 * note: define NO_UNIX_GLOBBING if you're not on a system that supports
 *   the UNIX concept of "~" referring to the user's home directory.
 *
 * $Id: misc.c,v 1.3 1992/11/24 16:00:31 jmason Exp $
 *
 * $Log: misc.c,v $
 * Revision 1.3  1992/11/24  16:00:31  jmason
 * fixed an embarrassing speeling erorr.
 *
 * Revision 1.2  1992/10/30  16:29:48  jmason
 * Added VMS support; fixed bad raster errors.
 *
 * Revision 1.1  1992/10/30  11:09:42  jmason
 * Placed under CVS control.
 *
 * Revision 3.1  1992/08/02  17:58:43  sboyle
 * Added clean_rcs_keyword().
 *
 * Revision 3.0  1992/07/04  01:27:16  sboyle
 * Initial RCS-aided revision
 *
 */

#include "gif320.h"

/* Just a little function to glob tildes in file names - handy for UNIX
 * systems as ~ refers to the user's home directory. */

char *globtilde(name)
  char *name;
{
  char fname[255];

  if ((name == NULL) || (*name == '\0'))
    return (NULL);

#ifdef NO_UNIX_GLOBBING
  if (*name == '~') {
    name++;
    strcpy(fname, (char *)getenv("HOME"));
    (void)strcat(fname, name);
  } else
#endif

  {
    strcpy(fname, name);
  }
  return (fname);
}

bool getyn(dflt)
  bool dflt;
{
  char ArgLine[255];
  void setup_vt();

  if (gets(ArgLine) == NULL) {
    setup_vt(stdout);
    printf("Bye.\n");
    exit(0);
  }
  if (ArgLine[0] == 'y')
    return (TRUE);
  else
    if (ArgLine[0] == 'n')
      return (FALSE);
    else
      return (dflt);
}
  
/* Thanks to Marc-Michael Brandis <brandis@inf.ethz.ch> for this sqrt()
 * routine... saves having to include the maths library! */

float msqrt(x)
  float x;
{
  float y, q, lastq;

  y = 2.0;
  q = -1.0;
  do {
    lastq = q;
    q = x / y;
    y = (q + y) / 2.0;
  }
  while (q != lastq);
  return (q);
}

/* estimate the area gained through optimisation. */

int estim_area(used)
  unsigned char used;
{
  float szX;
  extern float Ratio;

  if (used == 0)		/* div by zero */
    szX = 0;			/* not developed yet, then */
  else
    szX = ((float)cells) * msqrt(XY_RATIO / used);
  return (szX * 1.15);
}

/* strip the crud from an RCS keyword. */

char *clean_rcs_keyword(keyword, new)
char *keyword, *new; {
  char *start, *end;
  int len;

  if ((start = strchr(keyword, ' ') + 1) &&
      (end = strrchr(keyword, ' '))) {	 /* make sure they're non-NULL */
    len = ((int) end) - ((int) start);
    strncpy(new, start, len);
    new[len] = '\0';
    return(new);
  } else {
    strcpy(new, keyword);
    return(new);		/* return an unclean keyword if error */
  }
}
