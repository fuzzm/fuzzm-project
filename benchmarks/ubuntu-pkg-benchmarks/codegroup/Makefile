 
PROGS = codegroup
# CFLAGS = -O
BIN = $(DESTDIR)/usr/bin/

all:	$(PROGS)

clean:
	rm -f $(PROGS) *.o *.bak *.zip core *.out

codegroup: codegroup.o
	$(CC) codegroup.o -o codegroup $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)

install:	${ALL}
	install codegroup ${BIN}

#   Create zipped archive

RELFILES = Makefile codegroup.1 codegroup.c \
	   codegroup.html codegroup.jpg codegroup.man codegrp.exe

release:
	rm -f codegroup.zip
	zip codegroup.zip $(RELFILES)

#   View manual page

manpage:
	nroff -man codegroup.1 | $(PAGER)

#   Print manual page

printman:
	ptroff -man codegroup.1

# Test it

# Need ./ notation, most paths don't include . - vlm 2004-08-21

test:	codegroup
	./codegroup -e <codegroup >/tmp/codegroup1.bak
	./codegroup -d </tmp/codegroup1.bak >/tmp/codegroup2.bak
	cmp codegroup /tmp/codegroup2.bak
	./codegroup -e codegroup /tmp/codegroup1.bak
	./codegroup -d /tmp/codegroup1.bak /tmp/codegroup2.bak
	cmp codegroup /tmp/codegroup2.bak
#	rm /tmp/codegroup1.bak /tmp/codegroup2.bak
