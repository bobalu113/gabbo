
# These lines are needed on some machines.
MAKE=make
SHELL=/bin/sh
INSTALL=/usr/bin/install -c
mkinstalldirs=$(SHELL) ../mkinstalldirs
#
CC=gcc -std=gnu99

prefix=/cygdrive/c/Users/bobal_000/work/gabbo
exec_prefix=${prefix}
datarootdir=${prefix}/share

SUBDIRS = indent erq
SED = sed

BINDIR=${exec_prefix}/bin
MUD_LIB=/cygdrive/c/Users/bobal_000/work/gabbo/mudlib

#PROFIL= -DOPCPROF -DVERBOSE_OPCPROF
#PROFIL=-p -DMARK
#PROFIL=-pg
PROFIL=
#Enable warnings from the compiler, if wanted.
WARN= # no warning options - will work with all compilers :-)
#WARN= -Wall -Wshadow -Dlint
#WARN= -Wall -Wshadow -Wno-parentheses # gcc settings
#
# Optimization and source level debugging options.
# adding a -fomit-frame-pointer on the NeXT (gcc version 1.93 (68k, MIT syntax))
# will corrupt the driver.
OPTIMIZE = -O4 -fomit-frame-pointer  -g # high optimization
#OPTIMIZE= -O2 -g # for better debugging
#OPTIMIZE= -g # no optimization; for frequent recompilations.
#
# debugging options: [-DDEBUG] [-DYYDEBUG|-DYYDEBUG=1] [-DTRACE_CODE]
# -DDEBUG: Enable run time debugging. It will use more time and space.
# When the flag is changed, be sure to recompile everything.
# Simply comment out this line if not wanted.
# If you change anything in the source, you are strongly encouraged to have
# DEBUG defined.
# If you will not change anything, you are still encouraged to have
# it defined, if you want to locate any game driver bugs.
#DEBUG=-DDEBUG -DTRACE_CODE
DEBUG=
#
MPATH=-DMUD_LIB='"$(MUD_LIB)"' -DBINDIR='"$(BINDIR)"'
#
CFLAGS=  -fwrapv -DMSWIN $(OPTIMIZE) $(DEBUG) $(WARN) $(MPATH) $(PROFIL)
#
LIBS=-lm  -lcrypt
#
LDFLAGS=



all:	subs

subs:
	for i in $(SUBDIRS) ;\
	do \
	(cd $$i ; echo "Making $$i." ;\
	$(MAKE) );\
	done

FORCE: install

install-subs:
	for i in $(SUBDIRS) ;\
	do \
	(cd $$i ; echo "Installing in $$i." ;\
	$(MAKE) $(MFLAGS) BINDIR=$(BINDIR) install);\
	done

# erq is installed with the subs (if needed). [Mubo]
#
install: install-subs
#	$(SED) -e "s;MUD_LIB;$(MUD_LIB);" \
#	-e "s;BINDIR;$(BINDIR);" restart_mud > $(BINDIR)/restart_mud
#	-chmod 775 $(BINDIR)/restart_mud

clean:
	-rm -f *.o *~
	for i in $(SUBDIRS) ;\
	do \
	(cd $$i ; echo "Cleaning in $$i." ;\
	$(MAKE) clean);\
	done
