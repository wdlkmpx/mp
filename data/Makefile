include ../config.mk

all:

strip:

install:
	mkdir -p $(DESTDIR)$(datadir)/applications
	$(INSTALL_DATA) mp.desktop $(DESTDIR)$(datadir)/applications
	mkdir -p $(DESTDIR)$(datadir)/pixmaps
	$(INSTALL_DATA) mp.png $(DESTDIR)$(datadir)/pixmaps
	mkdir -p $(DESTDIR)$(mandir)/man1
	$(INSTALL_DATA) mp.1 $(DESTDIR)$(mandir)/man1
	mkdir -p $(DESTDIR)$(sysconfdir)
	$(INSTALL_DATA) mprc.sample $(DESTDIR)$(sysconfdir)

install-strip: install

uninstall:
	rm -f $(DESTDIR)$(datadir)/applications/mp.desktop
	rm -f $(DESTDIR)$(datadir)/pixmaps/mp.png
	rm -f $(DESTDIR)$(mandir)/man1/mp1.1
	rm -f $(DESTDIR)$(sysconfdir)/mprc.sample

clean:

distclean:

