#!/usr/bin/make -f
wzip : wzip.c
	$(CC) -o wzip wzip.c $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)

clean :
	rm -f wzip

install : wzip
	install -D -g root -o root -m 755 wzip $(DESTDIR)/usr/bin/wzip
