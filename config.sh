#!/bin/sh

# Minimum Profit autoconfiguration script

DRIVERS=""
TARGET="mp"
APPNAME="mp"

# gets program version
VERSION=3.3.18b

# default installation prefix
PREFIX=/usr/local

# parse arguments
while [ $# -gt 0 ] ; do

	case $1 in
	--with-included-regex)	WITH_INCLUDED_REGEX=1 ;;
	--without-pcre)		WITHOUT_PCRE=1 ;;
	--without-gettext)	WITHOUT_GETTEXT=1 ;;
	--with-mp5-keys)	WITH_MP5_KEYS=1 ;;
	--help)			CONFIG_HELP=1 ;;
	--prefix)		PREFIX=$2 ; shift ;;
	--prefix=*)		PREFIX=`echo $1 | sed -e 's/--prefix=//'` ;;
	esac

	shift
done

if [ "$CONFIG_HELP" = "1" ] ; then

	echo "Available options:"
	echo "--prefix=PREFIX       Installation prefix ($PREFIX)."
	echo "--with-included-regex Use included regex code (gnu_regex.c)."
	echo "--without-pcre        Disable PCRE library detection."
	echo "--without-gettext     Disable gettext (use workaround)."
	echo "--with-mp5-keys       Use version 5.x keybindings."

	echo
	echo "Environment variables:"
	echo "CC                    C Compiler."
	echo "CFLAGS                Compile flags (i.e., -O3)."
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

# add version
echo "#define VERSION \"${VERSION}\"" >> config.h
# add installation prefix
echo "#define CONFOPT_PREFIX \"$PREFIX\"" >> config.h

################################################################

# test for curses / ncurses library
echo -n "Testing for curses... "

echo "#include <curses.h>" > .tmp.c
echo "int main(void) { initscr(); endwin(); return 0; }" >> .tmp.c

TMP_LDFLAGS="-lncurses"

# try plain curses library
TMP_LDFLAGS="-lcurses"
$CC ${CFLAGS} .tmp.c ${LDFLAGS} $TMP_LDFLAGS -o .tmp.o 2>> .config.log
if [ $? = 0 ] ; then
	echo "#define CONFOPT_CURSES 1" >> config.h
	echo $TMP_LDFLAGS >> config.ldflags
	echo "OK (plain curses)"
	DRIVERS="curses $DRIVERS"
else
	exit "No curses library was found..."
	exit 1
fi

# regex
echo -n "Testing for regular expressions... "

if [ "$WITHOUT_PCRE" != 1 -a "$WITH_INCLUDED_REGEX" != 1 ] ; then
	# try first the pcre library
	TMP_LDFLAGS="-lpcre -lpcreposix"
	echo "#include <pcreposix.h>" > .tmp.c
	echo "int main(void) { regex_t r; regmatch_t m; regcomp(&r,\".*\",REG_EXTENDED|REG_ICASE); return 0; }" >> .tmp.c

	$CC ${CFLAGS} .tmp.c ${LDFLAGS} $TMP_LDFLAGS -o .tmp.o 2>> .config.log
	if [ $? = 0 ] ; then
		echo "OK (using pcre library)"
		echo "#define CONFOPT_PCRE 1" >> config.h
		echo "$TMP_LDFLAGS " >> config.ldflags
		REGEX_YET=1
	fi
fi

if [ "$REGEX_YET" != 1 -a "$WITH_INCLUDED_REGEX" != 1 ] ; then
	echo "#include <sys/types.h>" > .tmp.c
	echo "#include <regex.h>" >> .tmp.c
	echo "int main(void) { regex_t r; regmatch_t m; regcomp(&r,\".*\",REG_EXTENDED|REG_ICASE); return 0; }" >> .tmp.c

	$CC ${CFLAGS} .tmp.c ${LDFLAGS} -o .tmp.o 2>> .config.log
	if [ $? = 0 ] ; then
		echo "OK (using system one)"
		echo "#define CONFOPT_SYSTEM_REGEX 1" >> config.h
		REGEX_YET=1
	fi
fi

if [ "$REGEX_YET" != 1 ] ; then
	# if system libraries lack regex, try compiling the included gnu_regex.c
	$CC ${CFLAGS} -c -DREGEX gnu_regex.c ${LDFLAGS} -o .tmp.o 2>> .config.log
	if [ $? = 0 ] ; then
		echo "OK (using included gnu_regex.c)"
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
	$CC ${CFLAGS} .tmp.c ${LDFLAGS} -o .tmp.o 2>> .config.log
	if [ $? = 0 ] ; then
		echo "OK"
		echo "#define CONFOPT_GETTEXT 1" >> config.h
	else
		# try now with -lintl
		TMP_LDFLAGS="-lintl"

		$CC ${CFLAGS} .tmp.c ${LDFLAGS} $TMP_LDFLAGS -o .tmp.o 2>> .config.log
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
[ "$WITH_MP5_KEYS" = 1 ] && echo "#define CONFOPT_MP5_KEYS 1" >> config.h
[ -f .config.h ] && cat .config.h >> config.h

echo >> config.h
echo "#define CONFOPT_UNIX_LIKE 1" >> config.h
echo "TARGET=$TARGET" >> makefile.opts
echo "VERSION=$VERSION" >> makefile.opts
echo "PREFIX=\$(DESTDIR)$PREFIX" >> makefile.opts
echo "APPNAME=$APPNAME" >> makefile.opts
echo >> makefile.opts

cat makefile.opts makefile.in makefile.depend > Makefile

##############################################

# cleanup
rm -f .tmp.c .tmp.o

exit 0
