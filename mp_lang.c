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

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mp_core.h"
#include "mp_lang.h"

#ifdef CONFOPT_GETTEXT

#include <locale.h>
#include <libintl.h>

#else /* CONFOPT_GETTEXT */

extern void po2c_setlang(char * lang);
extern char * po2c_gettext(char * msgid);

#endif /* CONFOPT_GETTEXT */

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
char * L(char * msgid)
{
#ifdef CONFOPT_GETTEXT
	return(gettext(msgid));
#else
	return(po2c_gettext(msgid));
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
#ifdef CONFOPT_GETTEXT

	if(setlocale(LC_ALL, langname) == NULL)
	{
		mp_log("Can't set locale '%s'\n", langname);
		setlocale(LC_ALL, "C");
	}

	bindtextdomain("minimum-profit", CONFOPT_PREFIX "/share/locale");
	textdomain("minimum-profit");

#else

	if(*langname == '\0')
	{
		/* if langname is "", get from the environment */
		if((langname=getenv("LANG")) == NULL)
		if((langname=getenv("LC_ALL")) == NULL)
		if((langname=getenv("LC_MESSAGES")) == NULL)
			langname="";
	}

	po2c_setlang(langname);
#endif
}
