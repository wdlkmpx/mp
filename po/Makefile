
include ../config.mk

MSGFMT = msgfmt
XGETTEXT = xgettext
MSGMERGE = msgmerge

LINGUAS = $(LINGUASX)

all: all-$(USE_NLS)
all-no:

all-yes:
	@sh zzpo.sh $(MSGFMT) $(XGETTEXT) $(MSGMERGE) mo $(LINGUAS)

$(GETTEXT_PACKAGE).pot:
	@sh zzpo.sh $(MSGFMT) $(XGETTEXT) $(MSGMERGE) pot $(GETTEXT_PACKAGE)

install: install-data-$(USE_NLS)
install-data-no: all
install-data-yes: all
	@catalogs="$(LINGUAS)"; \
	for cat in $$catalogs; do \
	  dir=$(DESTDIR)$(localedir)/$$cat/LC_MESSAGES; \
	  mkdir -p $$dir; \
	  $(INSTALL_DATA) $$cat.mo $$dir/$(GETTEXT_PACKAGE).mo; \
	  echo "installing $$cat.mo as $$dir/$(GETTEXT_PACKAGE).mo"; \
	done

strip:

install-strip: install

uninstall:
	@catalogs="$(LINGUAS)"; \
	for cat in $$catalogs; do \
	  echo "rm -f $(DESTDIR)$(localedir)/$$cat/LC_MESSAGES/$(GETTEXT_PACKAGE).mo"; \
	  rm -f $(DESTDIR)$(localedir)/$$cat/LC_MESSAGES/$(GETTEXT_PACKAGE).mo; \
	done

mostlyclean:
	rm -f *.new.po *.mo *.gmo
clean: mostlyclean
distclean: mostlyclean

update-po:
	@sh zzpo.sh $(MSGFMT) $(XGETTEXT) $(MSGMERGE) po $(GETTEXT_PACKAGE) $(LINGUAS)

