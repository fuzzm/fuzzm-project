Description: Makefile for building and installing palbart
---
This patch header follows DEP-3: http://dep.debian.net/deps/dep3/
Index: git/Makefile
===================================================================
--- /dev/null	1970-01-01 00:00:00.000000000 +0000
+++ git/Makefile	2017-07-09 03:34:23.651616641 +0200
@@ -0,0 +1,18 @@
+# Makefile for palbart written by vlm@debian.org on Jan 16 2000 for Debian
+# This makefile is released under the GPL
+#
+
+DESTDIR 	=
+BIN		= $(DESTDIR)/usr/bin
+CFLAGS		+= -Wall -Wstrict-prototypes
+
+
+palbart:	palbart.o
+
+install:	palbart
+		install ./palbart "$(BIN)"
+
+clean:
+		rm -f palbart palbart.o
+
+.PHONY: clean
