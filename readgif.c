/* readgif.c
 *
 * The GIF-reading code. Anyone want to mail me some analogous code for
 * JPEG, PBM, etc.?
 *
 * define REAL_RATIO if you want to use the images' real X:Y ratios
 *   (ie. if most of your images have square pixels). By the way,
 *   this is quite unlikely (imho).
 *
 * $Id: readgif.c,v 1.4 1992/11/24 16:00:34 jmason Exp $
 *
 * $Log: readgif.c,v $
 * Revision 1.4  1992/11/24  16:00:34  jmason
 * fixed an embarrassing speeling erorr.
 *
 * Revision 1.3  1992/10/30  16:34:50  jmason
 * Added STRICT_DECODE so bad GIFs can be (almost) viewed anyway.
 *
 * Revision 1.2  1992/10/30  16:29:54  jmason
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

/*
 * STRICT_DECODE means that, if an error is encountered in the GIF data,
 * gif320 will exit with an error message. This isn't always that desirable.
 */

#undef STRICT_DECODE

unsigned char colormap[256][3], *raster;
int ImgX, ImgY, *i_leave;

static codetype *codetable;
static int datasize, codesize, codemask, clear, eoi, colorbits;
static bool global;

extern float Ratio;
extern FILE *infile;

unsigned int RealTop, RealLeft, RealRight, RealBot,
  top, left, bot, right;

void change_colors(), checksignature(), free_image(), outcode(), process(),
  read_image(), readextension(), readfile(), readraster(), readscreen();

extern void domenu(), optimise(), clear_image(), setup_vt(),
  initcolors(), fatal();

/* Output the bytes associated with a code to the raster array. */

void outcode(p, fill)
  register codetype *p;
  register unsigned char **fill;
{
  if (p->prefix)
    outcode(p->prefix, fill);
  *(*fill)++ = p->suffix;
}

/* Process a compression code.  "clear" resets the code table.
 * Otherwise make a new code table entry, and output the bytes
 * associated with the code. */

void process(code, fill)
  register code;
  unsigned char **fill;
{
  static avail, oldcode;
  register codetype *p;

  if (code == clear) {
    codesize = datasize + 1;
    codemask = (1 << codesize) - 1;
    avail = clear + 2;
    oldcode = -1;
  } else if (code < avail) {
    outcode(&codetable[code], fill);
    if (oldcode != -1) {
      p = &codetable[avail++];
      p->prefix = &codetable[oldcode];
      p->first = p->prefix->first;
      p->suffix = codetable[code].first;
      if (((avail & codemask) == 0) && (avail < 4096)) {
	codesize++;
	codemask += avail;
      }
    }
    oldcode = code;
  } else if (code == avail && oldcode != -1) {
    p = &codetable[avail++];
    p->prefix = &codetable[oldcode];
    p->first = p->prefix->first;
    p->suffix = p->first;
    outcode(p, fill);
    if (((avail & codemask) == 0) && (avail < 4096)) {
      codesize++;
      codemask += avail;
    }
    oldcode = code;
  }
#ifdef STRICT_DECODE
  else {
    fatal("illegal code in raster data");
  }
#endif
}

/* Read a GIF extension block (and do nothing with it). */

void readextension()
{
  register int count;
  char buf[255];

  (void) getc(infile);		/* code */
  while (count = getc(infile))
    fread(buf, 1, count, infile);
}

/* Read image information (position, size, local color map, etc.) and
 * pass them to the menu. */

void read_image()
{
  unsigned char buf[9];
  bool local, interleaved;
  register int i, row;
  unsigned int height, width;

  fread(buf, 1, 9, infile);
  left = RealLeft = buf[0] + (buf[1] << 8);
  top = RealTop = buf[2] + (buf[3] << 8);
  width = buf[4] + (buf[5] << 8);
  height = buf[6] + (buf[7] << 8);
  RealBot = ImgY = bot = top + height;
  RealRight = ImgX = right = left + width;
#ifdef REAL_RATIO
  Ratio = ((float)ImgX) / ((float)ImgY);
#else
  Ratio = 0.8;		/* gives an approximately square image */
#endif
  local = buf[8] & 0x80;
  interleaved = buf[8] & 0x40;
  if (local)
    colorbits = (1 << (buf[8] & 0x7) + 1);
  else if (!global)
    fatal("no colormap present for image");
  change_colors();
  raster = (unsigned char *) malloc(width * height);
  if (!raster)
    fatal("not enough memory for image");
  readraster(width, height);
  if (interleaved) {
    i_leave = (int *) malloc(height * sizeof(int));
    if (!i_leave)
      fatal("not enough memory for interleave table");
    row = 0;
    for (i = top; i < top + height; i += 8)
      i_leave[i] = row++;
    for (i = top + 4; i < top + height; i += 8)
      i_leave[i] = row++;
    for (i = top + 2; i < top + height; i += 4)
      i_leave[i] = row++;
    for (i = top + 1; i < top + height; i += 2)
      i_leave[i] = row++;
  } else {
    i_leave = (int *) NULL;
  }
}

/* Free the relevant bits allocated by read_image(). */

void free_image() {
  if (i_leave) {
    free(i_leave);
  }
  free(raster);
}

/* Decode a raster image. [doc's note: I don't understand any of this
 * GIF business, but it works.] ;-) */

void readraster(width, height)
  unsigned width, height;
{
  unsigned char *fill = raster;
  unsigned char buf[255];
  register unsigned count, datum = 0;
  register unsigned char *ch;
  register int bits = 0, code, Max, Min;

#define METER_MAX (66)
#define METER_READ ((METER_MAX * (((int) fill) - Min)) / Max)

  datasize = getc(infile);
  clear = 1 << datasize;
  eoi = clear + 1;
  codesize = datasize + 1;
  codemask = (1 << codesize) - 1;
  codetable = (codetype *) malloc(4096 * sizeof(codetype));
  if (!codetable)
    fatal("not enough memory for code table");
  for (code = 0; code < clear; code++) {
    codetable[code].prefix = (codetype *) 0;
    codetable[code].first = code;
    codetable[code].suffix = code;
  }
  drawBox(19, 1, 21, 80);
  setupReadMeter(19);
  linedraw(stdout,TRUE);
  Max = width * height;
  Min = (int) raster;
  for (count = getc(infile); count > 0; count = getc(infile)) {
    fread(buf, 1, count, infile);
    for (ch = buf; count-- > 0; ch++) {
      datum += *ch << bits;
      bits += 8;
      while (bits >= codesize) {
	code = datum & codemask;
	datum >>= codesize;
	bits -= codesize;
	if (code == eoi)
	  goto exitloop;
	process(code, &fill);
	drawReadMeter(19, METER_READ);
      }
    }
  }
exitloop:
  linedraw(stdout,FALSE);
#ifdef STRICT_DECODE
  if (fill != raster + width * height)
    fatal("raster has the wrong size");
#endif
  free(codetable);
}

/* Get information which is global to all the images stored in the
 * file. */

void readscreen()
{
  unsigned char buf[7];
#ifndef lint
  unsigned int screenwidth, screenheight;	/* unused */
#endif

  fread(buf, 1, 7, infile);
#ifndef lint
  screenwidth = buf[0] + (buf[1] << 8);
  screenheight = buf[2] + (buf[3] << 8);
#endif
  global = buf[4] & 0x80;
  if (global) {
    colorbits = (1 << (buf[4] & 0x07) + 1);
    fread(colormap, 3, colorbits, infile);
  }
}

void change_colors()
{
  initcolors(colorbits);
}

void checksignature()
{
  char buf[6];

  fread(buf, 1, 6, infile);
  if (strncmp(buf, "GIF", 3))
    fatal("file is not a GIF file");
}

void readfile(pipe)
  bool pipe;
{
  bool quit = FALSE;
  int ch;

  do {
    ch = getc(infile);
    switch (ch) {
    case '\0':
      break;			/* this kludge for non-standard files */
    case ',':
      clear_image();
      read_image();
      if (pipe)
	optimise();
      else
	domenu();
      free_image();
      break;
    case ';':
      quit = TRUE;
      break;
    case '!':
      readextension();
      break;
    default:
      fatal("illegal GIF block type");
      break;
    }
  }
  while (!quit);
}
