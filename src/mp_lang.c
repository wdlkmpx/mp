/*

    mp - Programmer Text Editor

    i18n.

    Copyright (C) 1991-2005 Angel Ortega <angel@triptico.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    http://www.triptico.com

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mp_core.h"
#include "mp_lang.h"

#if defined(HAVE_GETTEXT)
#include <locale.h>
#include <libintl.h>
#elif defined(BUILTIN_NLS)
extern void po2c_setlang(char * lang);
extern char * po2c_gettext(char * msgid);
#endif

/*******************
	Data
********************/

/*******************
	Code
********************/

/**
 * L - Translate a string
 * @msgid: the string to be translated
 *
 * Translate @msgid using the current language info. If no
 * translation string is found, the same @msgid is returned.
 */
char * _(char * msgid)
{
#if defined(HAVE_GETTEXT)
	return(gettext(msgid));
#elif defined(BUILTIN_NLS)
	return(po2c_gettext(msgid));
#else
	return msgid;
#endif
}


/**
 * mpl_set_language - Sets the current language
 * @langname: name of the language to be set
 *
 * Sets the current language for language-dependent strings.
 * The @langname can be a two letter standard or the english
 * name for the language. If the required language is not
 * found in the internal database, english is set.
 */
void mpl_set_language(char * langname)
{
#if defined(HAVE_GETTEXT)

	if (setlocale(LC_ALL, langname) == NULL)
	{
		mp_log("Can't set locale '%s'\n", langname);
		setlocale(LC_ALL, "");
	}

	textdomain ("minimum-profit");
	bindtextdomain ("minimum-profit", LOCALEDIR);
	bind_textdomain_codeset ("minimum-profit", "UTF-8");

#elif defined(BUILTIN_NLS)

	if (*langname == '\0')
	{
		/* if langname is "", get from the environment */
		if ((langname=getenv("LANG")) == NULL)
		if ((langname=getenv("LC_ALL")) == NULL)
		if ((langname=getenv("LC_MESSAGES")) == NULL)
			langname="";
	}
	po2c_setlang (langname);
#endif
}
