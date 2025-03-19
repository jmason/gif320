/* develop.c
 *
 * The funny development code with anti-aliasing and character packing
 * that allows all the nifty optimisation tricks.
 *
 * $Id: develop.c,v 1.3 1992/11/24 16:00:29 jmason Exp $
 *
 * $Log: develop.c,v $
 * Revision 1.3  1992/11/24  16:00:29  jmason
 * fixed an embarrassing speeling erorr.
 *
 * Revision 1.2  1992/10/30  16:29:43  jmason
 * Added VMS support; fixed bad raster errors.
 *
 * Revision 1.1  1992/10/30  11:09:42  jmason
 * Placed under CVS control.
 *
 * Revision 3.0  1992/07/04  01:27:16  sboyle
 * Initial RCS-aided revision
 *
 */

#include "gif320.h"
#include "pack_macros.h"

/* macros used for the on-screen progress meters */

#define METER_MAX (66)
#define METER_PROG ((int) (progress * METER_MAX) / totalArea)
#define METER_CELLS ((int) (curr_char * METER_MAX) / cells)

unsigned char curr_char;
int NColors;
static unsigned short colortable[256];
static int black_char, white_char;

extern int *i_leave;
extern void show_image(), clear_image(), plot(), drawMeters(),
  drawLines(), drawBox();

unsigned char scrmap[scrwidth][scrheight];
int dispindx[cells], dispindy[cells];
void pack_init();

/* Develop a image into grwidth*grheight bitmap using anti-aliasing and
 * the image optimisation stuff. Return early if character supply runs
 * out.
 */

int develop(grwidth, grheight, topLine)
  int grheight, grwidth, topLine;
{
  static long sum, area;
  static float tx_ratio, ty_ratio;
  static int x, end, row, start, endrow, grcol, grrow, mgrcol,
    mgrrow, progress, endgrcol, endgrrow, r_grcol_start, r_grcol_end,
    hexFull, hexHalf, width, totalArea, ditherHalf, ditherFull,
    cells_width, cells_height;

  extern int FullValue, HalfValue, top, left, bot, right,
    RealRight, RealLeft;
  extern unsigned char *raster;
  extern bool verbose;

  cells_width = grwidth / charwidth;
  cells_height = grheight / charheight;

  totalArea = cells_width * cells_height;
  ty_ratio = ((float) (bot - top)) / ((float) grheight);
  tx_ratio = ((float) (right - left)) / ((float) grwidth);
  hexFull = (FullValue * 0x100) / 100;
  hexHalf = (HalfValue * 0x100) / 100;
  progress = 0;
  pack_init();
  if (verbose) {
    drawBox(topLine, 1, topLine + 3, 80);
    mvcur(topLine, 3);
    printf(" (%d x %d) ", cells_width, cells_height);
    drawLines(topLine);
  }
  width = (int) (RealRight - RealLeft);

  if (verbose)
    linedraw(stdout,TRUE);

/*
 * mgrrow, mgrcol:
 *
 * The individual cells of the image.
 */
  for (mgrrow = (grheight / charheight) - 1; mgrrow >= 0; mgrrow--) {
    for (mgrcol = (grwidth / charwidth) - 1; mgrcol >= 0; mgrcol--) {
/*
 * grrow, grcol:
 *
 * The individual bits of a cell. grcol is done in twos for the 2-bit
 * dithering used.
 */
      endgrcol = (mgrcol + 1) * charwidth;
      endgrrow = (mgrrow + 1) * charheight;
      for (grrow = mgrrow * charheight; grrow < endgrrow; grrow ++) {
	ditherFull = (ditherHalf = (mgrcol + grrow) % 2) ^ 1;
	for (grcol = mgrcol * charwidth; grcol < endgrcol; grcol += 2) {
/*
 * [inner loop]:
 * 
 * Antialias the image bits that make up the cell bit.
 *
 * t[xy]_ratio are floats; row, etc. are ints. Therefore a kludge
 * to protect the precision is required.
 */

	  sum = area = 0;
	  r_grcol_start = grcol * tx_ratio + left;
	  r_grcol_end = (grcol + 2) * tx_ratio + left;
	  endrow = (grrow + 1) * ty_ratio + top;
	  for (row = grrow * ty_ratio + top; row <= endrow; row++) {
	    start = (i_leave ? i_leave[row] : row) * width + r_grcol_start;
	    end = (i_leave ? i_leave[row] : row) * width + r_grcol_end;
	    for (x = start; x <= end; x++)
	      sum += colortable[(int) (*(raster + x))];
	    area += (end - start) + 1;	/* area covered by this pass */
	  }
	  if (area)
	    sum /= area;
	  else
	    sum = colortable[(int) (*(raster + ((i_leave ? i_leave[row]
		: row) * width + r_grcol_start)))];

/*
 * sum is now the value of the pixel; plot it as either full or "half"
 * intensity.
 */
	  if (sum >= hexFull)
	    plot (ditherFull + grcol, grrow);
	  if (sum >= hexHalf)
	    plot (ditherHalf + grcol, grrow);
	}
      }
      if (pack(mgrrow, mgrcol)) {
	drawMeters(topLine, METER_PROG, METER_MAX);
	if (verbose)
	  linedraw(stdout,FALSE);
	return (TRUE);
      }
      progress++;
      drawMeters(topLine, METER_PROG, METER_CELLS);
    }
  }
  if (verbose)
    linedraw(stdout,FALSE);
  return (FALSE);
}

/* pack subsection: if OPTIMISE_PACKING is undefined, three subroutines
 * (check_black, check_white and check_other) are set up that return
 * whether a given cell in the display consists only of off pixels,
 * on pixels or is identical to another cell.
 */

extern unsigned char display [scrwidth][scrheight][groups][charwidth];

#ifndef OPTIMISE_PACKING
static bool cell_success;

bool check_black(xpos, ypos)
int xpos, ypos; {
  static int i;

  cell_success = TRUE;
  for (i = charwidth - 1; i; i--)
    if (!(cell_success &= (display[xpos][ypos][0][i]==PC_BLACK) &
	(display[xpos][ypos][1][i]==PC_BLACK)))
      return(FALSE);
  return(TRUE);
}

bool check_white(xpos, ypos)
int xpos, ypos; {
  static int i;

  cell_success = TRUE;
  for (i = charwidth - 1; i; i--)
    if (!(cell_success &= (display[xpos][ypos][0][i]==PC_WHITE) &
	(display[xpos][ypos][1][i]==PC_WHITE)))
      return(FALSE);
  return(TRUE);
}

#  ifdef CHECK_FOR_IDENTICAL
bool check_other(xpos, ypos, dx, dy)
int xpos, ypos, dx, dy; {
  static int i;

  cell_success = TRUE;
  for (i = charwidth - 1; i; i--)
    if (!(cell_success &= (display[xpos][ypos][0][i] == display[dx][dy][0][i])
	& (display[xpos][ypos][1][i] == display[dx][dy][1][i])))
      return(FALSE);
  return(TRUE);
}
#  endif
#endif

/* Nuts'n'bolts of the optimisation process. Check for duplicates of
 * earlier cells, and don't bother defining them in the dispind*[]
 * arrays if they're already there. Checks plain black and plain white
 * first. */

int pack (ypos, xpos)
int ypos, xpos;
{
  static char a, b, k;
  static int dx, dy;
  static bool success;

  a = display[xpos][ypos][0][0];
  b = display[xpos][ypos][1][0];
  success = FALSE;

/*
 * is the cell all black (all bits off)?
 *
 */

#ifdef OPTIMISE_PACKING
#  define Vopt_chk(group, slice) (display[xpos][ypos][group][slice]==PC_BLACK)
#  include "pack_macros.h"
#else
#  undef all_checks
#  define all_checks check_black(xpos, ypos)
#endif

  if ((a == PC_BLACK) && (b == PC_BLACK) && (all_checks)) {
    if (black_char == PC_UNDEF)
      black_char = curr_char;
    else {
      scrmap[xpos][ypos] = black_char;
      success = TRUE;
    }
  } else

/*
 * is the cell all white (all bits on)?
 *
 */

#ifdef OPTIMISE_PACKING
#  undef Vopt_chk
#  define Vopt_chk(group, slice) (display[xpos][ypos][group][slice]==PC_WHITE)
#  include "pack_macros.h"
#else
#  undef all_checks
#  define all_checks check_white(xpos, ypos)
#endif

  if ((a == PC_WHITE) && (b == PC_WHITE) && (all_checks)) {
    if (white_char == PC_UNDEF)
      white_char = curr_char;
    else {
      scrmap[xpos][ypos] = white_char;
      success = TRUE;
    }
  }

#ifdef CHECK_FOR_IDENTICAL
  else
/*
 * is the cell identical to another cell, then?
 *
 */

#  ifdef OPTIMISE_PACKING
#    undef Vopt_chk
#    define Vopt_chk(group, slice) (display[xpos][ypos][group][slice] == \
       display[dx][dy][group][slice])
#    include "pack_macros.h"
#  else
#    undef all_checks
#    define all_checks check_other(xpos, ypos, dx, dy)
#  endif

  {
    for (k = 0; k < curr_char; k++) {
      dx = dispindx[k];
      dy = dispindy[k];
      if ((display[dx][dy][0][0] == a) && (display[dx][dy][1][0] == b)
	  && (all_checks)) {
	scrmap[xpos][ypos] = k;
	success = TRUE;
      }
    }
  }
#endif

/*
 * a unique cell, then. Give it it's own space.
 *
 */

  if (success == FALSE) {
    scrmap[xpos][ypos] = curr_char;
    dispindx[curr_char] = xpos;
    dispindy[curr_char] = ypos;
    if ((++curr_char) >= cells)
      return (TRUE);		/* ran out of empty cells */
  }
  return (FALSE);
}

void pack_init() {	/* initialise pack's variables */
  curr_char = 0;
  black_char = white_char = PC_UNDEF;
}

/* Convert a color map (local or global) to an array of short integers
 * stored in colortable.  RGB is converted to 8-bit grayscale using
 * integer arithmetic.
 */

void initcolors(ncolors)
  int ncolors;
{
  register int i, lr, lg, lb;
  extern int RGB_r, RGB_g, RGB_b;
  extern unsigned char colormap[0x100][3];

  lr = RGB_r * (0x100 / 100);
  lg = RGB_g * (0x100 / 100);
  lb = RGB_b * (0x100 / 100);
  NColors = ncolors;
  for (i = 0; i < ncolors; i++)
    colortable[i] = (short) ((lr * colormap[i][0] +
      lg * colormap[i][1] + lb * colormap[i][2]) >> 8);
}

/* Optimise output by binary search until redundant chars -> 0. */

void optimise(maxX, maxY)
int maxX, maxY;
{
  register float minX, minY, diffX, diffY;
  extern int lastX, lastY;
  bool lastworked;

#define optDevelop() (develop(((int) (minX + diffX)) * charwidth, \
			((int) (minY + diffY)) * charheight, 20))

  {
    register float rat, chrs, target;
    extern float Ratio, msqrt();

    /* ensure min. size'll fit into 96 chars. */
    
    rat = XY_RATIO;
    target = chrs = (float) cells;
    do {
      minY = msqrt(chrs / rat);
      minX = rat * minY;
      chrs -= .1;
    } while (minY * minX > target);

    /* and then find the maximum size that'll fit the screen while still
     * keeping the ratio.
     */

    if (maxX && maxY) {
      if ((maxX < minX) || (maxY < minY))
	minX = minY = 2.0;
      diffX = ((float) maxX) - minX;
      diffY = ((float) maxY) - minY;
      if (diffX > ((float) scrwidth) - minX) {
	diffY *= ((((float) scrwidth) - minX) / diffX);
	diffX = ((float) scrwidth) - minX;
      }
      if (diffY > ((float) scrheight) - minY) {
	diffX *= ((((float) scrheight) - minY) / diffY);
	diffY = ((float) scrheight) - minY;
      }
    } else {
      diffX = (24.0 * rat) - minX;
      if (diffX > ((float) scrwidth) - minX) {
	diffY = (((float) scrwidth) / rat) - minY;
	diffX = ((float) scrwidth) - minX;
      } else {
	diffY = ((float) scrheight) - minY;
      }
    }
  }

  clear_image();
  if (optDevelop()) {
    diffX /= 2.0;
    diffY /= 2.0;
    do {
      optBoxes(minX, minY, diffX, diffY, TRUE);
      clear_image();
      if (!optDevelop()) {
	lastworked = TRUE;
	minX += diffX;
	minY += diffY;
      } else
	lastworked = FALSE;
      diffX /= 2.0;
      diffY /= 2.0;
    } while ((diffX > 1.0) || (diffY > 1.0) || (!lastworked));
  } else {
    minX += diffX;
    minY += diffY;
  }
  optBoxes(minX, minY, 0.0, 0.0, TRUE);
  lastX = (int) minX;
  lastY = (int) minY;
  show_image(stdout, lastX, lastY, curr_char, FALSE);
}
