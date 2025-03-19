$ !
$ ! VMS-MAKE.COM: written from instructions for compilation on a VAXstation
$ !   provided by Kelly Dean <dean%sol.dnet@sirius.cira.colostate.edu>, based
$ !   on the VMS-CC.MAK file from the GhostScript distribution, which is...
$ !
$ !    Copyright (C) 1989, 1992 Aladdin Enterprises.  All rights reserved.
$ !    Distributed by Free Software Foundation, Inc.
$ !
$ !
$ IF P1 .EQS. "CLEAN" THEN GOTO CLEAN
$ !
$ ! Give ourself a healthy search list for C include files
$ !
$ DEFINE C$INCLUDE 'F$ENVIRONMENT("DEFAULT"),SYS$LIBRARY
$ DEFINE VAXC$INCLUDE C$INCLUDE
$ DEFINE SYS SYS$LIBRARY
$ !
$ ! Then get compiling...
$ !
$ WRITE SYS$OUTPUT "DEVELOP.C:"
$ CC DEVELOP.C /OPT
$ WRITE SYS$OUTPUT "PRIMARY.C:"
$ CC PRIMARY.C /OPT
$ WRITE SYS$OUTPUT "MISC.C:"
$ CC MISC.C /OPT
$ WRITE SYS$OUTPUT "READGIF.C:"
$ CC READGIF.C /OPT
$ WRITE SYS$OUTPUT "VTGRAPH.C:"
$ CC VTGRAPH.C /OPT /DEFINE=ON_VAX
$ !
$ ! And link.
$ !
$ WRITE SYS$OUTPUT "Linking GIF320:"
$ LINK'LDEF/EXE=GIF320.EXE DEVELOP,PRIMARY,-
MISC,READGIF,VTGRAPH,SYS$INPUT/OPT
SYS$SHARE:VAXCRTL/SHARE
$ !
$ ! De-assign the search lists.
$ !
$ DEASSIGN C$INCLUDE
$ DEASSIGN VAXC$INCLUDE
$ DEASSIGN SYS
$ !
$ ! Target for CLEAN.
$ !
$ CLEAN:
$ !
$ WRITE SYS$OUTPUT "Cleaning:"
$ DELETE *.OBJ;*
