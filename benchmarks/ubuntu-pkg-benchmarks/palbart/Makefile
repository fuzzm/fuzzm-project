# Makefile for palbart written by vlm@debian.org on Jan 16 2000 for Debian
# This makefile is released under the GPL
#

DESTDIR 	=
BIN		= $(DESTDIR)/usr/bin
CFLAGS		+= -Wall -Wstrict-prototypes


palbart:	palbart.o

install:	palbart
		install ./palbart "$(BIN)"

clean:
		rm -f palbart palbart.o

.PHONY: clean
