# Makefile for "gif320" program.
#
# Tested on MIPS Magnum, Tahoe BSD.
# Should be portable since it only uses stdio and a
# couple of signal trapping calls.

#CFLAGS		= -O2
CFLAGS		= -g

CC		= gcc
SRCS		= develop.c primary.c misc.c readgif.c vtgraph.c
OBJS		= develop.o primary.o misc.o readgif.o vtgraph.o
TARGET		= gif320

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET)

lint:
	lint $(DEFINES) $(SRCS)

$(OBJS): vtgraph.h gif320.h config.h
