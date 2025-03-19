/*
 * vtgraph.h - escape sequences for vt320s and assoc. macros.
 *
 *
 * $Id: vtgraph.h,v 1.2 1992/10/30 16:29:57 jmason Exp $
 *
 */

/* ES_init sets up vt320 mode (vt400 on same), sets 24 lines per page,
 * puts term into 80 cols, and sets light text/dark screen mode. */
#define ES_init "\033[63;1\"p\033[24*|\033[?3l\033[?5l"

#define ES_home_cursor "\033[H"
#define ES_clear_to_end_of_screen "\033[J"
#define ES_move_cursor "\033[%d;%df"
#define ES_move_cursor_right_N "\033[%dC"

/* ES_clear_soft_font is a bug workaround. */
#define ES_clear_soft_font "\033P1;1;0;12;1;1;12;0{ @"

#define ES_define_soft_font "\033P1;0;0;15;1;2;12;1{ @"

/* ES_end_soft_font_defn ends the definition and puts the defined
 * font into G1. */
#define ES_end_soft_font_defn "\033\\\033) @"

#define ES_clear_to_eol "\033[K"
#define ES_doublesize_top "\033#3"
#define ES_doublesize_bot "\033#4"

/* ES_soft_font_on is SO; means put G1 into GL to display */
#define ES_soft_font_on "\016"

/* ES_soft_font_off is SI; means put G0 into GL to display */
#define ES_soft_font_off "\017"

#define ES_standout_on "\033[1m"
#define ES_standout_off "\033[22m"
#define ES_line_drawing_chars_on "\033(0"
#define ES_line_drawing_chars_off "\033(B"

#define mvcur(y, x) printf (ES_move_cursor, y, x);
#define fmvcur(f, y, x) fprintf (f, ES_move_cursor, y, x);
#define mvcurright(x) printf (ES_move_cursor_right_N, x)
