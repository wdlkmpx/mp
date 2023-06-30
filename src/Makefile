
include ../config.mk

APP  = mp$(EXE)

# ls *.c | sed 's%\.c%.o%g' | tr '\n' ' ' >> Makefile
OBJS = mp_conf.o mp_core.o mp_func.o mp_iface.o mp_lang.o mp_synhi.o mp_tags.o mpv_curses.o mp_video.o mpv_unix_common.o mp_wordp.o

#CPPFLAGS += -I..
CFLAGS += -I..

LIBS     += $(NCURSES_LIBS) $(LIBPCRE_LIBS)
CFLAGS   += $(NCURSES_CFLAGS) $(LIBPCRE_FLAGS)

all: $(APP)

$(APP): $(OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(OBJS) -o $(APP) $(LDFLAGS) $(LIBS)

strip: $(APP)
	$(STRIP) $(APP)

clean:
	rm -f *.o *~ $(APP)

distclean: clean

install: $(APP)
	mkdir -p $(DESTDIR)$(bindir)
	install $(APP) $(DESTDIR)$(bindir)

install-strip: strip install

uninstall:
	rm -f $(DESTDIR)$(bindir)/$(APP)
