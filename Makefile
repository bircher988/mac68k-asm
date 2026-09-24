# mac68k-asm - 68k assembler toolchain for the classic Macintosh
CC      ?= cc
CFLAGS  ?= -O2 -std=c99 -Wall -Wextra -Wno-unused-parameter
PREFIX  ?= /usr/local
BINDIR   = $(PREFIX)/bin
DATADIR  = $(PREFIX)/share/mac68k-asm

SRC = src/main.c src/util.c src/resfork.c src/rescomp.c src/asm.c src/link.c src/build.c
OBJ = $(SRC:.c=.o)

all: mac68k-asm

mac68k-asm: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

src/%.o: src/%.c src/*.h
	$(CC) $(CFLAGS) -c -o $@ $<

install: mac68k-asm
	install -d $(DESTDIR)$(BINDIR) $(DESTDIR)$(DATADIR)/inc $(DESTDIR)$(DATADIR)/example
	install -m 755 mac68k-asm $(DESTDIR)$(BINDIR)/mac68k-asm
	install -m 644 inc/* $(DESTDIR)$(DATADIR)/inc/
	install -m 644 example/* $(DESTDIR)$(DATADIR)/example/

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/mac68k-asm
	rm -rf $(DESTDIR)$(DATADIR)

test: mac68k-asm
	tests/run.sh

clean:
	rm -f mac68k-asm $(OBJ)

.PHONY: all install uninstall test clean
