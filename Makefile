ALL=fail

CFLAGS=-g -O1 -Wall -Wno-switch -Wextra -Wwrite-strings -fPIE
LDFLAGS=-pie

DESTDIR=
PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
MANDIR=$(PREFIX)/share/man

all: $(ALL)

README: fail.1
	mandoc -Tutf8 $< | col -bx >$@

clean: FRC
	rm -f $(ALL)

install: FRC all
	mkdir -p $(DESTDIR)$(BINDIR) $(DESTDIR)$(MANDIR)/man1
	install -m0755 $(ALL) $(DESTDIR)$(BINDIR)
	install -m0644 $(ALL:=.1) $(DESTDIR)$(MANDIR)/man1

FRC:
