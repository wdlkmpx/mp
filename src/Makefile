
include ../config.mk

APP  = mp
# ls *.c | sed 's%\.c%.o%g' | tr '\n' ' ' >> Makefile
OBJS = gnu_regex.o mp_conf.o mp_core.o mp_func.o mp_iface.o mp_lang.o mp_synhi.o mp_tags.o mpv_curses.o mp_video.o mpv_unix_common.o mp_wordp.o
SRCS = gnu_regex.c mp_conf.c mp_core.c mp_func.c mp_iface.c mp_lang.c mp_synhi.c mp_tags.c mpv_curses.c mp_video.c mpv_unix_common.c mp_wordp.c

#CPPFLAGS += -I..
CFLAGS += -I..

LIBS     += $(NCURSES_LIBS)
CFLAGS   += $(NCURSES_CFLAGS)

all: $(APP)

$(APP): $(OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(OBJS) -o $(APP) $(LDFLAGS) $(LIBS)

strip: $(APP)
	$(STRIP) $(APP)

clean:
	rm -f *.o *~ $(APP) $(APP).exe

distclean: clean

install: $(APP)
	mkdir -p $(DESTDIR)$(bindir)
	install $(APP) $(DESTDIR)$(bindir)

install-strip: strip install

uninstall:
	rm -f $(DESTDIR)$(bindir)/$(APP)

makefile.dep:
	$(CC) -MM $(SRCS) > makefile.dep

# ------------------------------------------------------------------------
# make makefile.dep
gnu_regex.o: gnu_regex.c
mp_conf.o: mp_conf.c mp_core.h mp_video.h mp_conf.h mp_lang.h mp_func.h \
 mp_iface.h mp_tags.h mp_wordp.h
mp_core.o: mp_core.c mp_core.h
mp_func.o: mp_func.c mp_core.h mp_video.h mp_func.h mp_iface.h mp_synhi.h \
 mp_lang.h mp_conf.h mp_wordp.h mp_func_i.h
mp_iface.o: mp_iface.c mp_core.h mp_conf.h mp_video.h mp_synhi.h \
 mp_lang.h mp_func.h mp_iface.h mp_tags.h mp_wordp.h
mp_lang.o: mp_lang.c mp_core.h mp_lang.h
mp_lang_m.o: mp_lang_m.c
mp_synhi.o: mp_synhi.c mp_core.h mp_video.h mp_conf.h mp_synhi.h \
 mp_tags.h
mp_tags.o: mp_tags.c mp_core.h mp_iface.h mp_tags.h mp_lang.h mp_video.h
mpv_curses.o: mpv_curses.c mp_core.h mp_video.h mp_synhi.h mp_func.h \
 mp_iface.h mp_lang.h mp_conf.h
mp_video.o: mp_video.c mp_core.h mp_video.h mp_iface.h mp_func.h \
 mp_lang.h mp_synhi.h mp_conf.h mp_tags.h
mpv_unix_common.o: mpv_unix_common.c mp_core.h mp_video.h mp_iface.h \
 mp_lang.h mp_synhi.h mp_conf.h
mp_wordp.o: mp_wordp.c mp_core.h mp_conf.h mp_lang.h mp_video.h
