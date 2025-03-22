# GIF320

GIF320 is a GIF viewer for DEC VT320 terminals:

![Photo of the app in action](http://jmason.org/software/vt320/photos/08.jpg)

Thanks to C Powers, who sent on this photo of GIF320 in action on a still-extant VT320, in 2024!
Apparently this ancient code still compiles on a modern Ubuntu.

The original README, from 33 years previously, is below.

------------------------------------------------------------------------

GIF320 3.0 is a gif file viewer for use with VT-320 terminals. It
should be easily portable to any platform as it uses only printf and
several other stdio commands; some signal trapping and tilde-globbing
is also provided, but can be configured out.

Note that GIF320 uses some obscure character-set definition escape
sequences, so if you use a VT-320 emulator or a VT-320 clone, it may
not work. It will not work on a 220 or a 420, although the 420
claims to emulate a 320. Unfortunately, I don't have access to a
420, so I'm looking for someone with a VT-420, a manual, and intricate
knowledge of escape sequences to hack on one for me and find out why
not. Anyone fitting that description out there?

A manual page is provided; if you are running GIF320 on an nroff-less
system, a pre-formatted manual page, gif320.txt, is also enclosed.

If you are on a system running VAX/VMS, a Makefile-emulation script
VMS-MAKE.COM is provided; see README.VMS for more information.

See config.h for configuration options.

-- doctorgonzo <jm@maths.tcd.ie>
------------------------------------------------------------------------

Architectures GIF320 has compiled successfully (and worked ;) on:

	Tahoe running 4.3 BSD UNIX
	MIPS Magnum running RISC/os 4.51
	Sparc running SunOS 4.1.2
