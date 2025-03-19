/* vtgraph.c
 *
 * Started off being generic VT320 soft-font definition code; went on to
 * to become optimised for GIFView 2.0; and then became all the box-drawing,
 * line-drawing, bells'n'whistles code for GIF320.
 *
 * $Id: vtgraph.c,v 1.4 1992/11/24 16:00:35 jmason Exp $
 *
 * $Log: vtgraph.c,v $
 * Revision 1.4  1992/11/24  16:00:35  jmason
 * fixed an embarrassing speeling erorr.
 *
 * Revision 1.3  1992/11/24  15:57:16  jmason
 * added latest sugggestion from Kelly Dean.
 *
 * Revision 1.2  1992/10/30  16:29:56  jmason
 * Added VMS support; fixed bad raster errors.
 *
 * Revision 1.1  1992/10/30  11:09:43  jmason
 * Placed under CVS control.
 *
 * Revision 3.0  1992/07/04  01:27:16  sboyle
 * Initial RCS-aided revision
 *
 */

#ifndef DONT_USE_SIGNAL
#  include <signal.h>

void reset_bits_on(), reset_bits_off();

#  ifdef ON_VAX
    /*
     * this is a fix provided by Kelly Dean
     * <dean%sol.dnet@sirius.cira.colostate.edu>, to get around two signals
     * missing from the VAX/VMS signal.h header file.
     */

#    define SIGSTOP	17	/* sendable stop signal not from tty */
#    define SIGTSTP	18	/* stop signal from tty */
#  endif
#endif

#include "vtgraph.h"
#include "gif320.h"

unsigned char display [scrwidth][scrheight][groups][charwidth];

void trap_ignore(), trap_on(), trap_off(), linedraw(), standout(),
  def_soft(), softfont();

/* set terminal up for soft-font programming.  */

void setup_vt(out)
  FILE *out;
{
  fprintf(out, "%s%s", ES_init, ES_clear_soft_font);
  def_soft(out,FALSE);
  softfont(out,FALSE);
  standout(out,FALSE);
  linedraw(out,FALSE);
  fflush(out);
}

/* Clear out full bitmap image. */

void clear_image()
{
  int row, col, group, slice;

  for (row = 0; row < scrheight; row++) {
    for (col = 0; col < scrwidth; col++) {
      for (group = 0; group < groups; group++) {
	for (slice = 0; slice < charwidth; slice++) {
	  display[col][row][group][slice] = 0;
	}
      }
    }
  }
}

/* Define and draw bitmap. */

void show_image(outF, charsX, charsY, curr_char, doublesize)
  FILE *outF;
  int charsX, charsY;
  unsigned char curr_char;
  bool doublesize;
{
  int row, col, group, slice, ypos, xpos, i;
  extern unsigned char scrmap[scrwidth][scrheight];
  extern int dispindx[cells], dispindy[cells];

  /* Define full cell 96 character font */
  def_soft(outF,TRUE);
  putc('\n', outF);
  for (i = 0; i < curr_char; i++) {
    row = dispindy[i];
    col = dispindx[i];
    for (group = 0; group < groups; group++) {
      for (slice = 0; slice < charwidth; slice++) {
	putc((int) (display[col][row][group][slice] + 0x3f), outF);
      }
      if (!group)
	putc('/', outF);
    }
    fprintf(outF, ";\n");
    fflush(outF);		/* buffering troubles */
  }
  def_soft(outF,FALSE);
  softfont(outF,TRUE);
  standout(outF,TRUE);
  putc('\n', outF);

  /* Draw characters in softset font */
  if (doublesize) {
    for (ypos = 0; ypos < charsY; ypos++) {
      for (i = 0; i < 2; i++) {
	if (!i)
	  fprintf(outF, ES_doublesize_top);
	else
	  fprintf(outF, ES_doublesize_bot);
	fmvcur(outF, 2 + ypos * 2 + i, ((scrwidth / 2) - charsX) / 2);
	for (xpos = 0; xpos < charsX; xpos++)
	  putc((int) (' ' + scrmap[xpos][ypos]), outF);
	putc('\n', outF);
      }
    }
  } else {
    for (ypos = 0; ypos < charsY; ypos++) {
      fmvcur(outF, 2 + ypos, (scrwidth - charsX) / 2);
      for (xpos = 0; xpos < charsX; xpos++)
	putc((int) (' ' + scrmap[xpos][ypos]), outF);
      putc('\n', outF);
    }
  }
  softfont(outF,FALSE);
  standout(outF,FALSE);
  fprintf(outF, "%s%s", ES_soft_font_off, ES_standout_off);
  fflush(outF);
}

/* Plot a single point into the graphics window at (xpos, ypos). */

void plot(xpos, ypos)
  int xpos, ypos;
{
  register int row, col, group, slice, sixel, tmp;
  static char pmask[sixels_group] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20 };

  if ((xpos < 0) || (ypos < 0) || (xpos > scrwidth * charwidth)
    || (ypos > scrheight * charheight))
    return;

  col = xpos / charwidth;
  slice = xpos % charwidth;

  row = ypos / charheight;
  tmp = ypos % charheight;

  group = tmp / sixels_group;
  sixel = tmp % sixels_group;

  display[col][row][group][slice] |= (pmask[sixel]);
}

void dump_char(col, row)
int col, row;
{
  int group, slice, ypos, xpos, sixel;
  char map[charwidth][charheight];
  static char pmask[sixels_group] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20 };

  for (group = 0; group < groups; group++) {
    for (slice = 0; slice < charwidth; slice++) {
      for (sixel = 0; sixel < 6; sixel++) {
	if (display[col][row][group][slice] & pmask[sixel])
	  map[slice][group * 6 + sixel] = '@';
	else
	  map[slice][group * 6 + sixel] = '.';
      }
    }
  }
  mvcur(1,1);
  for (ypos = 0; ypos < charheight; ypos++) {
    printf("  [");
    for (xpos = 0; xpos < charwidth; xpos++)
      putchar(map[xpos][ypos]);
    printf("]  \n");
  }
}

int handleMeter(old, new, ypos)
  int old, new, ypos;
{
  register int i;

  if (new > old) {
    mvcur(ypos + 1, 13 + old);
    for (i = old; i < new; i++)
      putchar('a');
  }
  return (new);
}

void drawLines(ypos)
  int ypos;
{
  extern bool verbose;

#define DO_LINE(text) printf("%-11s%s%s%s\n", (text), \
  ES_line_drawing_chars_on, \
  "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~", \
  ES_line_drawing_chars_off);

  if (verbose) {
    mvcur(ypos + 1, 2);
    DO_LINE("Progress:");
    mvcur(ypos + 2, 2);
    DO_LINE("Cells:");
    fflush(stdout);
  }
}

void setupReadMeter(ypos)
  int ypos;
{
  extern bool verbose;

  if (verbose) {
    mvcur(ypos + 1, 2);
    DO_LINE("Reading:");
    fflush(stdout);
  }
}

void drawMeters(ypos, done, used)
  int ypos, done, used;
{
  static int oldDone, oldUsed;
  extern bool verbose;

  if (verbose) {
    oldDone = handleMeter(oldDone, done, ypos);
    oldUsed = handleMeter(oldUsed, used, ypos + 1);
    fflush(stdout);
  }
}

void drawReadMeter(ypos, done)
  int ypos, done;
{
  static int oldDone;
  extern bool verbose;

  if (verbose) {
    oldDone = handleMeter(oldDone, done, ypos);
    fflush(stdout);
  }
}

void drawBox(t, l, b, r)
  int t, l, b, r;
{
  extern bool verbose;
  register int i;

  if (verbose) {
    linedraw(stdout,TRUE);
    mvcur(t, l);
    putchar('l');
    for (i = (r - l) - 1; i; i--)
      putchar('q');
    putchar('k');
    for (i = t + 1; i < b; i++) {
      mvcur(i, l);
      putchar('x');
      mvcurright((r - l) - 1);
      putchar('x');
    }
    mvcur(b, l);
    putchar('m');
    for (i = (r - l) - 1; i; i--)
      putchar('q');
    putchar('j');
    linedraw(stdout,FALSE);
  }
}

optBoxes(X, Y, dX, dY, clear)
  float X, Y, dX, dY;
  bool clear;
{
  extern bool verbose;
  static int aX, aY, bX, bY;
  register int i;

  if (verbose) {
    if (clear) {
      printf(ES_home_cursor);
      for (i = bY + 1; i; i--)
	printf("%s\n", ES_clear_to_eol);
    }
    aX = (int)X;
    bX = (int)(X + 2 * dX);
    aY = (int)Y;
    bY = (int)(Y + 2 * dY);
    drawBox(2, (scrwidth - aX) / 2, aY + 1, (scrwidth + aX) / 2 - 1);
    drawBox(2, (scrwidth - bX) / 2, bY + 1, (scrwidth + bX) / 2 - 1);
  }
}

bool softfont_mode, linedraw_mode, standout_mode;

#ifndef DONT_USE_SIGNAL
void reset_bits_off() {
  if (softfont_mode)
    printf(ES_soft_font_off);
  if (linedraw_mode)
    printf(ES_line_drawing_chars_off);
  if (standout_mode)
    printf(ES_standout_off);
}

void reset_bits_on() {
  extern bool RedrawHeader;

  redraw_screen();
  if (softfont_mode)
    printf(ES_soft_font_on);
  if (linedraw_mode)
    printf(ES_line_drawing_chars_on);
  if (standout_mode)
    printf(ES_standout_on);
}

sig_type reset_and_intr() {

  reset_bits_off();
  exit(128 + 2);		/* exit code for SIGINT ;-P */
}

sig_type reset_and_stop() {
  reset_bits_off();
  kill(getpid(), SIGSTOP);
  reset_bits_on();
}
#endif

void trap_ignore() {
#ifndef DONT_USE_SIGNAL
  signal(SIGINT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
#endif
}

void trap_on() {
#ifndef DONT_USE_SIGNAL
  signal(SIGINT, reset_and_intr);
  signal(SIGTSTP, reset_and_stop);
#endif
}

void trap_off() {
#ifndef DONT_USE_SIGNAL
  signal(SIGINT, SIG_DFL);
  signal(SIGTSTP, SIG_DFL);
#endif
}

void linedraw(file,state)
FILE *file;
bool state; {
  if (state) {
    linedraw_mode = TRUE;
    fprintf(file, ES_line_drawing_chars_on);
  } else {
    linedraw_mode = FALSE;
    fprintf(file, ES_line_drawing_chars_off);
  }
}

void standout(file,state)
FILE *file;
bool state; {
  if (state) {
    standout_mode = TRUE;
    fprintf(file, ES_standout_on);
  } else {
    standout_mode = FALSE;
    fprintf(file, ES_standout_off);
  }
}

void softfont(file,state)
FILE *file;
bool state; {
  if (state) {
    softfont_mode = TRUE;
    fprintf(file, ES_soft_font_on);
  } else {
    softfont_mode = FALSE;
    fprintf(file, ES_soft_font_off);
  }
}

void def_soft(file,state)
FILE *file;
bool state; {
  if (state) {
    trap_ignore();
    fprintf(file, ES_define_soft_font);
  } else {
    trap_on();
    fprintf(file, ES_end_soft_font_defn);
  }
}
