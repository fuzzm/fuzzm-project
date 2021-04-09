# you may need to change these for your system
CFLAGS	= -g
MAKE	= make
#CC	= cc
RM	= /bin/rm

TEX	= userguide music
ABC	= English Jigs Reels Strspys

install: abc2mtex sort_in search
	$(RM) -f *.o

abc2mtex: fields.o abc.o tex.o index.o
	$(CC) $(CFLAGS) -o $@ fields.o abc.o tex.o index.o

sort_in: sort_in.o index.o
	$(CC) $(CFLAGS) -o $@ sort_in.o index.o

search: search.o abc.c tex.o index.o
	$(CC) $(CFLAGS) -o $@ search.o abc.o tex.o index.o

index: *.abc
	abc2mtex -i *.abc
	sort_in

ps: $(ABC:=.ps)

tidy:
	$(RM) -f *.o index abc2mtex.log music.tex userguide.toc transpose.abc
	$(RM) -f $(TEX:=.dvi) $(TEX:=.log) $(TEX:=.ps) $(TEX:=.aux)
	$(RM) -f $(ABC:=.tex) $(ABC:=.dvi) $(ABC:=.log) $(ABC:=.ps) $(ABC:=.aux)

clean: tidy
	$(RM) -f abc2mtex sort_in search

.SUFFIXES:  .ps .dvi .tex .abc

.abc.tex:
	abc2mtex -o $*.tex $*

.abc.dvi:
	abc2mtex -o $*.tex $*
	tex $*

.abc.ps:
	abc2mtex -o $*.tex $*
	tex $*
	dvips -o $*.ps $*.dvi -D 600

.tex.dvi:
	tex $*

.tex.ps:
	tex $*
	dvips -o $*.ps $*.dvi -D 600

.dvi.ps:
	dvips -o $*.ps $*.dvi -D 600

