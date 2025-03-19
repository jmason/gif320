/* pack_macros.h
 *
 * the token for an undefined black_char or white_char, the actual byte
 * values of same, and the macros used to optimise speed of pack subroutine.
 *
 * $Id: pack_macros.h,v 1.3 1992/11/24 16:00:32 jmason Exp $
 *
 * $Log: pack_macros.h,v $
 * Revision 1.3  1992/11/24  16:00:32  jmason
 * fixed an embarrassing speeling erorr.
 *
 * Revision 1.2  1992/10/30  16:29:51  jmason
 * Added VMS support; fixed bad raster errors.
 *
 * Revision 1.1  1992/10/30  11:09:43  jmason
 * Placed under CVS control.
 *
 * Revision 3.0  1992/07/04  01:27:16  sboyle
 * Initial RCS-aided revision
 *
 */

/*
 * undefine the stuff as this file is repeatedly included to redefine
 * the macros opt_chk() and all_checks based on opt_chk_sub().
 */

#undef PC_WHITE
#undef PC_BLACK
#undef PC_UNDEF

#define PC_WHITE ((unsigned char) 0x00) /* all bits off */
#define PC_BLACK ((unsigned char) 0x3f) /* all bits on */
#define PC_UNDEF ((unsigned char) 0xff)	/* pack char undefined */

#ifdef OPTIMISE_PACKING

/* all this is really obsolete, come to think of it.
 * It'd want to be a pretty brain-damaged compiler that
 * didn't optimise this.
 */

#  if charwidth != 15
	CRASH -- add a few more opt_chks in pack_macros.h as its
	optimised for 15 charwidths. if n = charwidth, there should
	be optchks from 1 to n-1.
#  endif

/*
 * Vopt_chk must be defined as the test to be performed.
 */

#  define opt_chk(slice) (Vopt_chk(0, slice)) && (Vopt_chk(1, slice))
#  define all_checks \
     (opt_chk(1) && opt_chk(9) && opt_chk(3) && opt_chk(11) && \
     opt_chk(5) && opt_chk(13) && opt_chk(7) && opt_chk(14) && \
     opt_chk(2) && opt_chk(10) && opt_chk(4) && opt_chk(12) && \
     opt_chk(6) && opt_chk(8))

#endif
