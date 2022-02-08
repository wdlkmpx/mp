#!/bin/sh

# Minimum Profit autoconfiguration script

DRIVERS=""
TARGET="mp"
APPNAME="mp"

# gets program version
VERSION=`cut -f2 -d\" VERSION`

# default installation prefix
PREFIX=/usr/local

# parse arguments
while [ $# -gt 0 ] ; do

	case $1 in
	--without-curses)	WITHOUT_CURSES=1 ;;
	--without-gtk)		WITHOUT_GTK=1 ;;
	--without-gtk1)		WITHOUT_GTK1=1 ;;
	--without-gtk2)		WITHOUT_GTK2=1 ;;
	--without-win32)	WITHOUT_WIN32=1 ;;
	--without-synhi)	WITHOUT_SYNHI=1 ;;
	--without-i18n)		WITHOUT_I18N=1 ;;
	--with-included-regex)	WITH_INCLUDED_REGEX=1 ;;
	--without-unix-glob)	WITHOUT_UNIX_GLOB=1 ;;
	--without-pcre)		WITHOUT_PCRE=1 ;;
	--without-gettext)	WITHOUT_GETTEXT=1 ;;
	--with-mp5-keys)	WITH_MP5_KEYS=1 ;;
	--help)			CONFIG_HELP=1 ;;

	--debian)		BUILD_FOR_DEBIAN=1
				PREFIX=/usr
				APPNAME=mped
				;;

	--prefix)		PREFIX=$2 ; shift ;;
	--prefix=*)		PREFIX=`echo $1 | sed -e 's/--prefix=//'` ;;
	esac

	shift
done

if [ "$CONFIG_HELP" = "1" ] ; then

	echo "Available options:"
	echo "--prefix=PREFIX       Installation prefix ($PREFIX)."
	echo "--without-curses      Disable curses (text) interface detection."
	echo "--without-gtk         Disable GTK (any version) interface detection."
	echo "--without-gtk1        Disable GTK 1.2.x interface detection."
	echo "--without-gtk2        Disable GTK 2.x interface detection."
	echo "--without-win32       Disable win32 interface detection."
	echo "--without-synhi       Don't include syntax highlight code."
	echo "--without-i18n        Don't include language support (english only)."
	echo "--with-included-regex Use included regex code (gnu_regex.c)."
	echo "--without-unix-glob   Disable glob.h usage (use workaround)."
	echo "--without-pcre        Disable PCRE library detection."
	echo "--without-gettext     Disable gettext (use workaround)."
	echo "--debian              Build for Debian ('make deb')."
	echo "--with-mp5-keys       Use version 5.x keybindings."

	echo
	echo "Environment variables:"
	echo "CC                    C Compiler."
	echo "CFLAGS                Compile flags (i.e., -O3)."
	echo "WINDRES               MS Windows resource compiler."

	exit 1
fi

echo "Configuring..."

echo "/* automatically created by config.sh - do not modify */" > config.h
echo "# automatically created by config.sh - do not modify" > makefile.opts
> config.ldflags
> config.cflags
> .config.log

# set compiler
if [ "$CC" = "" ] ; then
	CC=cc
	# if CC is unset, try if gcc is available
	which gcc > /dev/null && CC=gcc
fi

echo "CC=$CC" >> makefile.opts

# set cflags
if [ "$CFLAGS" = "" ] ; then
	CFLAGS="-g -Wall"
fi

echo "CFLAGS=$CFLAGS" >> makefile.opts

# Add CFLAGS to CC
CC="$CC $CFLAGS"

# add version
cat VERSION >> config.h

# add installation prefix
echo "#define CONFOPT_PREFIX \"$PREFIX\"" >> config.h

################################################################

# test for curses / ncurses library
echo -n "Testing for curses... "

if [ "$WITHOUT_CURSES" = "1" ] ; then
	echo "Disabled by user"
else
	echo "#include <curses.h>" > .tmp.c
	echo "int main(void) { initscr(); endwin(); return 0; }" >> .tmp.c

	TMP_CFLAGS="-I/usr/local/include"
	TMP_LDFLAGS="-L/usr/local/lib -lncurses"

	$CC $TMP_CFLAGS .tmp.c $TMP_LDFLAGS -o .tmp.o 2>> .config.log
	if [ $? = 0 ] ; then
		echo "#define CONFOPT_CURSES 1" >> config.h
		echo $TMP_CFLAGS >> config.cflags
		echo $TMP_LDFLAGS >> config.ldflags
		echo "OK (ncurses)"
		DRIVERS="curses $DRIVERS"
	else
		# try plain curses library
		TMP_LDFLAGS="-L/usr/local/lib -lcurses"
		$CC $TMP_CFLAGS .tmp.c $TMP_LDFLAGS -o .tmp.o 2>> .config.log
		if [ $? = 0 ] ; then
			echo "#define CONFOPT_CURSES 1" >> config.h
			echo $TMP_CFLAGS >> config.cflags
			echo $TMP_LDFLAGS >> config.ldflags
			echo "OK (plain curses)"
			DRIVERS="curses $DRIVERS"
		else
			echo "No"
			WITHOUT_CURSES=1
		fi
	fi
fi

if [ "$WITHOUT_CURSES" != "1" ] ; then
	# test for transparent colors in curses
	echo -n "Testing for transparency support in curses... "

	echo "#include <curses.h>" > .tmp.c
	echo "int main(void) { initscr(); use_default_colors(); endwin(); return 0; }" >> .tmp.c

	$CC  .tmp.c `cat ./config.ldflags` -o .tmp.o 2>> .config.log
	if [ $? = 0 ] ; then
		echo "#define CONFOPT_TRANSPARENCY 1" >> config.h
		echo "OK"
	else
		echo "No"
	fi
fi

# GTK
echo -n "Testing for GTK... "

if [ "$WITHOUT_GTK" = "1" ] ; then
	echo "Disabled by user"
	WITHOUT_GTK1=1
	WITHOUT_GTK2=1
	GTK_YET=1
fi

if [ "$WITHOUT_GTK2" != "1" ] ; then
	echo "#include <gtk/gtk.h>" > .tmp.c
	echo "#include <gdk/gdkkeysyms.h>" >> .tmp.c
	echo "int main(void) { gtk_main(); return 0; } " >> .tmp.c

	# Try first GTK 2.0
	TMP_CFLAGS=`pkg-config --cflags gtk+-2.0 2>/dev/null`
	TMP_LDFLAGS=`pkg-config --libs gtk+-2.0 2>/dev/null`

	$CC $TMP_CFLAGS .tmp.c $TMP_LDFLAGS -o .tmp.o 2> .config.log
	if [ $? = 0 ] ; then
		echo "#define CONFOPT_GTK 2" >> config.h
		echo "$TMP_CFLAGS " >> config.cflags
		echo "$TMP_LDFLAGS " >> config.ldflags
		echo "OK (2.0)"
		DRIVERS="gtk $DRIVERS"
		GTK_YET=1
	fi
fi

if [ "$GTK_YET" != 1 -a "$WITHOUT_GTK1" != "1" ] ; then
	echo "#include <gtk/gtk.h>" > .tmp.c
	echo "#include <gdk/gdkkeysyms.h>" >> .tmp.c
	echo "int main(void) { gtk_main(); return 0; } " >> .tmp.c

	TMP_CFLAGS=`gtk-config --cflags 2>/dev/null`
	TMP_LDFLAGS=`gtk-config --libs 2>/dev/null`

	$CC $TMP_CFLAGS .tmp.c $TMP_LDFLAGS -o .tmp.o 2>> .config.log
	if [ $? = 0 ] ; then
		echo "#define CONFOPT_GTK 1" >> config.h
		echo "$TMP_CFLAGS " >> config.cflags
		echo "$TMP_LDFLAGS " >> config.ldflags
		echo "OK (1.2)"
		DRIVERS="gtk $DRIVERS"
		GTK_YET=1
	fi
fi

if [ "$GTK_YET" != 1 ] ; then
	echo "No"
fi

# Win32
echo -n "Testing for win32... "
if [ "$WITHOUT_WIN32" = "1" ] ; then
	echo "Disabled by user"
else
	echo "#include <windows.h>" > .tmp.c
	echo "#include <commctrl.h>" >> .tmp.c
	echo "int STDCALL WinMain(HINSTANCE h, HINSTANCE p, LPSTR c, int m)" >> .tmp.c
	echo "{ return 0; }" >> .tmp.c

	TMP_LDFLAGS="-mwindows -lcomctl32"
	$CC .tmp.c $TMP_LDFLAGS -o .tmp.o 2>> .config.log

	if [ $? = 0 ] ; then
		echo "#define CONFOPT_WIN32 1" >> config.h
		echo "$TMP_LDFLAGS " >> config.ldflags
		echo "OK"
		DRIVERS="win32 $DRIVERS"
		WITHOUT_UNIX_GLOB=1
		TARGET=wmp.exe
	else
		echo "No"
	fi
fi

# glob.h support
if [ "$WITHOUT_UNIX_GLOB" != 1 ] ; then
	echo -n "Testing for unix-like glob.h... "
	echo "#include <stdio.h>" > .tmp.c
	echo "#include <glob.h>" >> .tmp.c
	echo "int main(void) { glob_t g; g.gl_offs=1; glob(\"*\",GLOB_MARK,NULL,&g); return 0; }" >> .tmp.c

	$CC .tmp.c -o .tmp.o 2>> .config.log

	if [ $? = 0 ] ; then
		echo "#define CONFOPT_GLOB_H 1" >> config.h
		echo "OK"
	else
		echo "No; activated workaround"
	fi
fi

# regex
echo -n "Testing for regular expressions... "

if [ "$WITHOUT_PCRE" != 1 -a "$WITH_INCLUDED_REGEX" != 1 ] ; then
	# try first the pcre library
	TMP_CFLAGS="-I/usr/local/include"
	TMP_LDFLAGS="-L/usr/local/lib -lpcre -lpcreposix"
	echo "#include <pcreposix.h>" > .tmp.c
	echo "int main(void) { regex_t r; regmatch_t m; regcomp(&r,\".*\",REG_EXTENDED|REG_ICASE); return 0; }" >> .tmp.c

	$CC $TMP_CFLAGS .tmp.c $TMP_LDFLAGS -o .tmp.o 2>> .config.log

	if [ $? = 0 ] ; then
		echo "OK (using pcre library)"
		echo "#define CONFOPT_PCRE 1" >> config.h
		echo "$TMP_CFLAGS " >> config.cflags
		echo "$TMP_LDFLAGS " >> config.ldflags
		REGEX_YET=1
	fi
fi

if [ "$REGEX_YET" != 1 -a "$WITH_INCLUDED_REGEX" != 1 ] ; then
	echo "#include <sys/types.h>" > .tmp.c
	echo "#include <regex.h>" >> .tmp.c
	echo "int main(void) { regex_t r; regmatch_t m; regcomp(&r,\".*\",REG_EXTENDED|REG_ICASE); return 0; }" >> .tmp.c

	$CC .tmp.c -o .tmp.o 2>> .config.log

	if [ $? = 0 ] ; then
		echo "OK (using system one)"
		echo "#define CONFOPT_SYSTEM_REGEX 1" >> config.h
		REGEX_YET=1
	fi
fi

if [ "$REGEX_YET" != 1 ] ; then
	# if system libraries lack regex, try compiling the
	# included gnu_regex.c

	$CC -c -DSTD_HEADERS -DREGEX gnu_regex.c -o .tmp.o 2>> .config.log

	if [ $? = 0 ] ; then
		echo "OK (using included gnu_regex.c)"
		echo "#define HAVE_STRING_H 1" >> config.h
		echo "#define STDC_HEADERS 1" >> config.h
		echo "#define REGEX 1" >> config.h
		echo "#define CONFOPT_INCLUDED_REGEX 1" >> config.h
	else
		echo "#define CONFOPT_NO_REGEX 1" >> config.h
		echo "No (No usable regex library)"
	fi
fi

# gettext support
echo -n "Testing for gettext... "

if [ "$WITHOUT_GETTEXT" = "1" ] ; then
	echo "Disabled by user"
else
	echo "#include <libintl.h>" > .tmp.c
	echo "#include <locale.h>" >> .tmp.c
	echo "int main(void) { setlocale(LC_ALL, \"\"); gettext(\"hi\"); return 0; }" >> .tmp.c

	# try first to compile without -lintl
	$CC .tmp.c -o .tmp.o 2>> .config.log

	if [ $? = 0 ] ; then
		echo "OK"
		echo "#define CONFOPT_GETTEXT 1" >> config.h
	else
		# try now with -lintl
		TMP_LDFLAGS="-lintl"

		$CC .tmp.c $TMP_LDFLAGS -o .tmp.o 2>> .config.log

		if [ $? = 0 ] ; then
			echo "OK (libintl needed)"
			echo "#define CONFOPT_GETTEXT 1" >> config.h
			echo "$TMP_LDFLAGS" >> config.ldflags
		else
			echo "No"
			WITHOUT_GETTEXT=1
		fi
	fi
fi

# test again for gettext support to enable workaround
if [ "$WITHOUT_GETTEXT" = "1" ] ; then
	echo "LANG_MSG_O=mp_lang_m.o" >> makefile.opts
	echo "INSTALL_MSG=" >> makefile.opts
else
	echo "LANG_MSG_O=" >> makefile.opts
	echo "INSTALL_MSG=install-mo" >> makefile.opts
fi

# final setup
[ "$WITHOUT_SYNHI" = 1 ] && echo "#define CONFOPT_WITHOUT_SYNHI 1" >> config.h
[ "$WITHOUT_I18N" = 1 ] && echo "#define CONFOPT_WITHOUT_I18N 1" >> config.h
[ "$WITH_MP5_KEYS" = 1 ] && echo "#define CONFOPT_MP5_KEYS 1" >> config.h
[ -f .config.h ] && cat .config.h >> config.h

echo >> config.h
echo "#if defined(CONFOPT_CURSES) || defined(CONFOPT_GTK)" >> config.h
echo "#define CONFOPT_UNIX_LIKE 1" >> config.h
echo "#endif" >> config.h

echo "TARGET=$TARGET" >> makefile.opts
echo "VERSION=$VERSION" >> makefile.opts
echo "WINDRES=$WINDRES" >> makefile.opts
echo "PREFIX=\$(DESTDIR)$PREFIX" >> makefile.opts
echo "APPNAME=$APPNAME" >> makefile.opts
echo >> makefile.opts

cat makefile.opts makefile.in makefile.depend > Makefile

##############################################

if [ "$DRIVERS" = "" ] ; then

	echo
	echo "*ERROR* No usable drivers (interfaces) found"
	echo "See the README file for the available options."

	exit 1
fi

echo
echo "Configured drivers:" $DRIVERS
echo
echo "Type 'make' to build Minimum Profit."

# cleanup
rm -f .tmp.c .tmp.o

exit 0
