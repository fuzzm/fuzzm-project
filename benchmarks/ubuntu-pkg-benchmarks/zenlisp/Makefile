# zenlisp Makefile
# By Nils M Holm, 2007, 2008
# See the file LICENSE for conditions of use.

V=	2

prefix?=/usr/local
PREFIX?=$(prefix)
BINOWN?=bin
BINGRP?=bin

BINDIR=	$(PREFIX)/bin
SHRDIR=	$(PREFIX)/share/zenlisp
MANDIR=	$(PREFIX)/man/man1
DOCDIR=	$(PREFIX)/share/doc/zenlisp
IMAGE=	$(PREFIX)/share/zenlisp/zenlisp

LIBS=	base.l imath.l iter.l nmath.l rmath.l

CPPFLAGS += -DDEFAULT_IMAGE="\"$(IMAGE)\""
LINTFLAGS=	-Wall -ansi -pedantic -Wmissing-prototypes -DLINT

all:	zl zenlisp

lint:
	$(CC) $(CFLAGS) $(LINTFLAGS) -o zl zl.c

zenlisp:	zl base.l
	echo '(load base) (dump-image zenlisp)' | ./zl -bi -n 12K

test:	zl
	rm -f delete-me
	ZENSRC=. ./zl -i <test.l | tee _test
	sed -i -e 's/^\* [0-9]*: /\* /' _test
	diff test.OK _test && rm _test delete-me

# Set $C to -c, if your system does not copy files by default.
C=
install: all
	strip zl
	install -d -m 0755 $(DESTDIR)$(SHRDIR)
	install -d -m 0755 $(DESTDIR)$(DOCDIR)
	install -d -m 0755 $(DESTDIR)$(BINDIR)
	install -d -m 0755 $(DESTDIR)$(MANDIR)
	install $C -m 0755 zl $(DESTDIR)$(BINDIR)/
	install $C -m 0644 zenlisp $(DESTDIR)$(SHRDIR)/
	install $C -m 0644 $(LIBS) $(DESTDIR)$(SHRDIR)/
	install $C -m 0644 src/*/*.l $(DESTDIR)$(SHRDIR)/
	install $C -m 0644 zl.1 $(DESTDIR)$(MANDIR)/
	install $C -m 0644 LICENSE $(DESTDIR)$(SHRDIR)/
	install $C -m 0644 zenlisp.txt $(DESTDIR)$(DOCDIR)/

clean:
	rm -f *.o *.a *.core core zl zenlisp _test delete-me \
		zenlisp$V.tgz
