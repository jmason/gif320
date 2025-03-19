/* main.c
 *
 * main(), the interactive line-oriented interface, and some other
 * appropriate bits.
 *
 * $Id: primary.c,v 1.2 1992/11/24 16:00:33 jmason Exp $
 *
 * $Log: primary.c,v $
 * Revision 1.2  1992/11/24  16:00:33  jmason
 * fixed an embarrassing speeling erorr.
 *
 * Revision 1.1  1992/10/30  16:29:52  jmason
 * Added VMS support; fixed bad raster errors.
 *
 * Revision 1.1  1992/10/30  11:09:42  jmason
 * Placed under CVS control.
 *
 * Revision 3.1  1992/08/02  17:58:08  sboyle
 * Fixed a problem caused by RCS ident strings.
 *
 * Revision 3.0  1992/07/04  01:27:16  sboyle
 * Initial RCS-aided revision
 *
 */

#include "gif320.h"

/* for zooms 'n' pans: the different types */

#define zoom_in		1
#define zoom_out	2
#define pan_left	3
#define pan_right	4
#define pan_up		5
#define pan_down	6
#define full_unzoom	7

static char *FName, *Args[4];
FILE *infile;
bool verbose, RedrawHeader, SketchDeveloped, Changed, Showing_Image,
  Optimising;

float Ratio;
extern unsigned char curr_char;
extern unsigned int RealTop, RealBot, RealLeft, RealRight,
  top, bot, left, right;

extern void readfile(), readscreen(), checksignature(), clear_image(),
  optimise(), show_image(), setup_vt(), change_colors(), drawBox();
extern bool getyn();
extern char *globtilde(), *clean_rcs_keyword();

int RGB_r = RGB_R_INIT, RGB_g = RGB_G_INIT, RGB_b = RGB_B_INIT,
  FullValue = FULL_VALUE_INIT, HalfValue = HALF_VALUE_INIT, lastX, lastY;

void commands_info(), domenu(), fatal(), show_header(),
  usage(), do_save_image();
int parse();

void usage() {
  fprintf(stderr, "usage: gif320 -p (pipe: optimises from GIF on stdin)\n");
  fprintf(stderr, "or:    gif320 <inputfile> ... (interactive)\n");
  exit(1);
}

void fatal(s)
  char *s; {
  setup_vt(stdout);
  trap_off();
  fprintf(stderr, "\n\ngif320: %s\n", s);
  exit(-1);
}

void die_messily() {
  setup_vt(stdout);
  trap_off();
  printf("\n\nUrk! EOF detected, exiting.\n");
  exit(2);
}

void commands_info()
{

#define SO ES_standout_on
#define SE ES_standout_off

  mvcur(9, 1);  printf("Commands:");
  mvcur(11, 1);  printf("%s(T)%shreshold <full> <half>", SO, SE);
  mvcur(11, 40); printf("%s(B)%salance <red> <green> <blue>", SO, SE);
  mvcur(12, 1);  printf("%s(return)%s develop sketch", SO, SE);
  mvcur(12, 40); printf("%s(H)%s, %s(J)%s, %s(K)%s, %s(L)%s pan image",
    SO, SE, SO, SE, SO, SE, SO, SE);
  mvcur(13, 1);  printf("%s(O)%sptimise [<cells x> <cells y>]", SO, SE);
  mvcur(13, 40); printf("%s(R)%satio <real>", SO, SE);
  mvcur(14, 1);  printf("%s(Z)%soom in", SO, SE);
  mvcur(14, 20); printf("%s(X)%soom out", SO, SE);
  mvcur(14, 40); printf("%s(F)%sull unzoom", SO, SE);
  mvcur(15, 1);  printf("%s(S)%save <file>", SO, SE);
  mvcur(15, 40); printf("%s(D)%soublesize <file>", SO, SE);
  mvcur(16, 1); printf("%s(?)%s program info", SO, SE);
  mvcur(16, 40); printf("%s(Q)%suit", SO, SE);
  fflush(stdout);
}

/* parse a command line into arguments separated by spaces. */

int parse(ArgLine)
  char *ArgLine;
{
  register int i;

  Args[0] = strtok(ArgLine, " \t");
  for (i = 1; i < 4; i++)
    Args[i] = strtok(NULL, " \t");
  if (Args[0] == NULL)
    Args[0] = "";
  for (i = 0; i < 4; i++)
    if (Args[i] == NULL)
      break;
  return i;
}

/* generic shift-calculation for all rezoning functions */

void do_zoom_n_pan(type, shift)
int type;
int shift; {
  int nt, nb, nl, nr, xshift, yshift;
  char err_str[255];

#define set_params(a, b, c, d, s) \
  nt = a; nb = b; nl = c; nr = d; strcpy(err_str, s);

  xshift = ((RealRight - RealLeft) / 200) * shift;
  yshift = ((RealBot - RealTop) / 200) * shift;
  switch (type) {
  case zoom_in:
    set_params(top + yshift, bot - yshift, left + xshift,
      right - xshift, "zoom in");
    break;
  case zoom_out:
    set_params(top - yshift, bot + yshift, left - xshift,
      right + xshift, "zoom out");
    break;
  case pan_left:
    set_params(top, bot, left - xshift, right - xshift, "pan left");
    break;
  case pan_right:
    set_params(top, bot, left + xshift, right + xshift, "pan right");
    break;
  case pan_up:
    set_params(top - yshift, bot - yshift, left, right, "pan up");
    break;
  case pan_down:
    set_params(top + yshift, bot + yshift, left, right, "pan down");
    break;
  case full_unzoom:
    set_params(RealTop, RealBot, RealLeft, RealRight,
      "full unzoom");	/* let's hope the string is never used ;-) */
    break;
  }

  if ((nt > nb) || (nl > nr) || (nt < (int) RealTop)	/* too much zooming */
      || (nb > (int) RealBot) || (nl < (int) RealLeft)
      || (nr > (int) RealRight)) {		/* keep inside the limits */
    printf("Can't %s any further.\n", err_str);
  } else {
    top = nt;
    bot = nb;
    left = nl;
    right = nr;
    Changed = TRUE;
    show_header();
  }
}

void do_change_rgb(r, g, b) {

  if ((r < 0) || (g < 0) || (b < 0) || (r > 100) || (g > 100) || (b > 100)) {
    printf("The RGB value range is 0<=val<=100.\n");
    return;
  }
  if (r + g + b != 100) {
    b = 100 - (r + g);
    if (b < 0) {
      printf("I can't change B so RGB totals 100!\n");
      return;
    } else {
      printf("Changing B to %d so RGB totals 100.\n", b);
    }
  }
  RGB_r = r;
  RGB_g = g;
  RGB_b = b;
  Changed = TRUE;
  show_header();
  change_colors();
}

void do_optimise(maxX, maxY)
int maxX, maxY; {
  char tmp[255];

  printf("%s%s", ES_home_cursor, ES_clear_to_end_of_screen);
  Optimising = TRUE;
  optimise(maxX, maxY);
  printf("\ncells used: %d/%d (%d%%) -- size: %dx%d ",
    curr_char, cells, (curr_char * 100) / cells, lastX, lastY);
  printf("-- save as? (return = cancel) ");
  fflush(stdout);
  my_gets(tmp);
  Changed = FALSE;
  SketchDeveloped = FALSE;
  Optimising = FALSE;
  RedrawHeader = TRUE;
  show_header();
  if (*tmp != '\0') 
    do_save_image(tmp, FALSE);
  else
    printf("Didn't save anything.\n");
}

void do_save_image(raw_name, doublesize)
char *raw_name;
bool doublesize; {
  char *cooked_name;
  FILE *outFile;

  if ((cooked_name = globtilde(raw_name)) == NULL)
    printf("I can't output to \"%s\"!\n", raw_name);
  else if (!(outFile = fopen(cooked_name, "w")))
    printf("Unable to open \"%s\".\n", cooked_name);
  else {
    setup_vt(outFile);
    show_image(outFile, lastX, lastY, curr_char, doublesize);
    fclose(outFile);
    printf("Output saved in \"%s\".\n", cooked_name);
  }
}

void develop_sketch() {
  if ((SketchDeveloped == FALSE) ||
	(lastX != 16) || (lastY != 6) || (Changed)) {
    SketchDeveloped = FALSE;
    show_header();
    clear_image();
    (void) develop(240, 72, MESSAGE_AREA + 1);
    lastX = 16;
    lastY = 6;
    SketchDeveloped = TRUE;
    Changed = FALSE;
    Showing_Image = TRUE;
    show_header();
    show_image(stdout, 16, 6, curr_char, FALSE);
    Showing_Image = FALSE;
    show_header();
  }
}

void redraw_screen() {
  if (!Optimising) {
    RedrawHeader = TRUE;
    show_header();
    show_image(stdout, 16, 6, curr_char, FALSE);
  }
}

void domenu()
{
  register int nargs;
  int i1, i2, i3;
  float d1;
  char ArgLine[255];

  develop_sketch();
  while (TRUE) {
    mvcur(23, 1);
    printf("%s\r%s", ES_clear_to_eol, PROMPT);
    my_gets(ArgLine);
    mvcur(MESSAGE_AREA, 1);
    puts(ES_clear_to_end_of_screen);
    nargs = parse(ArgLine);
    switch (my_tolower(*Args[0])) {
    case 'b':
      if ((nargs != 4) || (sscanf(Args[1], "%d", &i1) +
	    sscanf(Args[2], "%d", &i2) + sscanf(Args[3], "%d", &i3) != 3))
	printf("you must provide 3 integer RGB values!\n");
      else
	do_change_rgb(i1, i2, i3);
      break;

    case 'r':
      if ((nargs == 2) && (sscanf(Args[1], "%f", &d1)) && (d1 > 0)) {
	Ratio = d1;
	Changed = TRUE;
	show_header();
      } else {
	printf("The ratios must be a float, controlling the X:Y ratio of");
	printf("the sides of\nthe optimised image.\n");
      }
      break;

    case 'z':
      do_zoom_n_pan(zoom_in, (nargs == 2) ? atoi(Args[1]) : 10);
      break;

    case 'x':
      do_zoom_n_pan(zoom_out, (nargs == 2) ? atoi(Args[1]) : 10);
      break;

    case 'h':
      do_zoom_n_pan(pan_left, (nargs == 2) ? atoi(Args[1]) : 10);
      break;

    case 'j':
      do_zoom_n_pan(pan_down, (nargs == 2) ? atoi(Args[1]) : 10);
      break;

    case 'k':
      do_zoom_n_pan(pan_up, (nargs == 2) ? atoi(Args[1]) : 10);
      break;

    case 'l':
      do_zoom_n_pan(pan_right, (nargs == 2) ? atoi(Args[1]) : 10);
      break;

    case 'f':
      do_zoom_n_pan(full_unzoom, (nargs == 2) ? atoi(Args[1]) : 10);
      break;

    case 't':
      if ((nargs != 3) || (sscanf(Args[1], "%d", &i1) + sscanf(Args[2],
	    "%d", &i2) != 2))
	printf("you must provide 2 integer threshold values!\n");
      else if ((i1 >= 0) && (i2 >= 0) && (i1 <= 100) &&
	    (i2 <= 100) && (i1 >= i2)) {
	FullValue = i1;
	HalfValue = i2;
	Changed = TRUE;
	show_header();
      } else {
	printf("The thresholds must be in the range ");
	printf("0 <= half <= full <= 100.\n");
      }
      break;

    case 'q':
      if (Changed) {
	printf("Are you sure you want to quit? (y/n) [n] ");
	if (!getyn(FALSE))
	  break;
      }
      mvcur(MESSAGE_AREA,1);
      printf("%s\n", ES_clear_to_end_of_screen);
      return;

    case 'o':
      if ((lastX != 16) && (lastY != 6) && (!Changed)) {
	printf("Are you sure you want to re-optimise? (y/n) [y] ");
	if (!getyn(TRUE))
	  break;
      }
      if ((nargs == 3) && (sscanf(Args[1], "%d", &i1) + sscanf(Args[2],
	    "%d", &i2) == 2)) {
	do_optimise(i1, i2);
      } else {
	do_optimise(0, 0);
      }
      break;

    case 's':
      if (nargs != 2) {
	printf("You must provide a file name to output!\n");
      } else {
	if (Changed) {
	  printf("Redeveloping picture:\n");
	  develop_sketch();
	}
	do_save_image(Args[1], FALSE);
      }
      break;

    case 'd':
      if (nargs != 2) {
	printf("You must provide a file name to output!\n");
      } else {
	if (Changed) {
	  printf("Redeveloping picture:\n");
	  develop_sketch();
	}
	do_save_image(Args[1], TRUE);
      }
      break;

    case '?':
      clean_rcs_keyword(VERSION, ArgLine);
      printf("GIF320 version %s by %s\n", ArgLine, AUTHOR);
      clean_rcs_keyword(LASTMOD, ArgLine);
      printf("Last modified %s\n", ArgLine);
      printf("Bugs and comments to %s.\n", BUG_ADDRESS);
      break;

    case '\0':
      if (((lastX != 16) || (lastY != 6)) && (!Changed)) {
	printf("Are you sure you want to redevelop? (y/n) [n] ");
	if (!getyn(TRUE))
	  break;
      }
      develop_sketch();
    }
  }
}

/* Draw most of the normal screen: description text, settings, box for
 * the bitmap, and call commands_info().
 */

void show_header() {
  register int i;
  char tmp[20];
  float projX;
  static float Ratio_chk;
  static int RGB_chk, thr_chk, charsused_chk, Image_chk;
  static unsigned int Zoom_chk;
  extern int ImgX, ImgY, NColors;

  puts(ES_home_cursor);
  if (RedrawHeader) {
    puts(ES_clear_to_end_of_screen);
    mvcur(1, 51);
    printf("Filename: %19s", FName);
    mvcur(4, 53);
    clean_rcs_keyword(VERSION, tmp);
    printf("GIF320 %s by doctorgonzo", tmp);
    mvcur(5, 51);
    printf("based on GIFView by Greg Reid");
    commands_info();
  }
  if ((ImgX ^ ImgY ^ NColors != Image_chk) || (RedrawHeader)) {
    mvcur(2, 51);
    sprintf(tmp, "%d x %d x %d", ImgX, ImgY, NColors);
    printf("Image: %22s", tmp);
  }
  if ((top ^ left ^ bot ^ right != Zoom_chk) || (RedrawHeader)) {
    mvcur(1, 1);
    sprintf(tmp, "(%d,%d)/(%d,%d)", left, top, right, bot);
    printf("Zoom: %22s", tmp);
    Zoom_chk = top ^ left ^ bot ^ right;
  }
  if ((RGB_r ^ RGB_g ^ RGB_b != RGB_chk) || (RedrawHeader)) {
    mvcur(2, 1);
    sprintf(tmp, "%d/%d/%d", RGB_r, RGB_g, RGB_b);
    printf("Balance (R/G/B): %11s", tmp);
    RGB_chk = RGB_r ^ RGB_g ^ RGB_b;
  }
  if ((FullValue ^ HalfValue != thr_chk) || (RedrawHeader)) {
    mvcur(3, 1);
    sprintf(tmp, "%d/%d", FullValue, HalfValue);
    printf("Thresholds (F/H): %10s", tmp);
    thr_chk = FullValue ^ HalfValue;
  }
  if ((Ratio != Ratio_chk) || (RedrawHeader)) {
    mvcur(4, 1);
    printf("Output X:Y ratio: %10.2f", Ratio);
    Ratio_chk = Ratio;
  }
  i = (curr_char * 100) / cells;
  if ((i != charsused_chk) || (Ratio != Ratio_chk) ||
      (RedrawHeader)) {
    mvcur(7, 1);
    projX = estim_area(curr_char);
    if (projX) {
      sprintf(tmp, "%1.0f x %1.0f", projX, projX / XY_RATIO);
    } else {
      sprintf(tmp, "(unknown)");
    }
    printf("Approx optimise: %11s", tmp);
  }
  if ((i != charsused_chk) || (RedrawHeader)) {
    mvcur(6, 1);
    sprintf(tmp, "%d/%d (%d%%)", curr_char, cells, i);
    printf("Cells used: %16s", tmp);
    charsused_chk = i;
  }

  if (RedrawHeader)
    drawBox(1, 31, 8, 48);
  mvcur(7, 52);
  if (!SketchDeveloped) {
    printf("      (not developed)      ");
    standout(stdout,TRUE);
    linedraw(stdout,TRUE);
    for (i = 2; i < 8; i++) {
      mvcur(i, 32);
      puts("````````````````");
    }
    linedraw(stdout,FALSE);
    standout(stdout,FALSE);
  } else if (Changed)
    printf("(changed since development)");
  else if (Showing_Image)
    printf(" (drawing developed image) ");
  else
    printf("  (accurate development)   ");
  mvcur(MESSAGE_AREA, 1);
  puts(ES_clear_to_end_of_screen);
  fflush(stdout);
  RedrawHeader = FALSE;
}

main(argc, argv)
  int argc;
  char *argv[];
{
  int i, ch;

  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
    if (argv[i][1] == 'p') {
	verbose = FALSE;
	infile = stdin;
	checksignature();
	readscreen();
	trap_on();
	readfile(TRUE);
      } else
	usage();
    } else {
      verbose = TRUE;
      if ((infile = fopen(argv[i], "r"))== NULL) {
	perror((char *) strcat("gif320: ", argv[i]));
	exit(1);
      }
      if ((FName = strrchr(argv[i], '/')) == NULL)
	FName = argv[i];
      else
	FName++;
      checksignature();
      trap_on();
      setup_vt(stdout);
      RedrawHeader = TRUE;
      show_header();
      readscreen();
      readfile(FALSE);
    }
  }
  trap_off();
  return(0);
}
