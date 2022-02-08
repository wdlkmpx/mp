/*

    mp_wordp.c

    Wordprocessing functions.

    mp - Programmer Text Editor

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

#ifndef _WIN32

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

#endif

#include "mp_core.h"
#include "mp_conf.h"
#include "mp_lang.h"
#include "mp_video.h"

/******************
	Data
*******************/

/* spellchecking active flag */
int mpw_spellcheck=0;

/* ispell command */
char mpw_ispell_command[128]="ispell";

int mpw_spell_cache_size=4096;

#ifndef _WIN32

/* spellchecking cache controls */

struct spell_cache
{
	char * word;
	int color;
};

static struct spell_cache * sp_cache=NULL;
static int sp_cache_i=0;


/******************
	Code
*******************/

static int spell_cmp(const void * p1, const void * p2)
/* bsearch/qsort function for spellchecking */
{
	struct spell_cache * s1;
	struct spell_cache * s2;

	s1=(struct spell_cache *)p1;
	s2=(struct spell_cache *)p2;

	return(strcmp(s1->word, s2->word));
}


static int search_spell_cache(char * word)
/* searches the spell cache, returning the color if found or -1 otherwise */
{
	struct spell_cache s;
	struct spell_cache * r;

	s.word=word;

	r=bsearch(&s, sp_cache, sp_cache_i, sizeof(struct spell_cache), spell_cmp);

	return(r == NULL ? -1 : r->color);
}


static void add_spell_cache(char * word, int color)
/* adds a word to the spell cache */
{
	/* is cache full? */
	if(sp_cache_i == mpw_spell_cache_size)
	{
		int n;

		/* destroy everything */
		for(n=0;n < sp_cache_i;n++)
			free(sp_cache[n].word);

		free(sp_cache);
		sp_cache=NULL;
		sp_cache_i=0;
	}

	sp_cache=realloc(sp_cache, (sp_cache_i + 1) * sizeof(struct spell_cache));

	sp_cache[sp_cache_i].word=strdup(word);
	sp_cache[sp_cache_i].color=color;

	sp_cache_i++;

	/* sort */
	qsort(sp_cache, sp_cache_i, sizeof(struct spell_cache), spell_cmp);
}


#define FLUSH_LINE(i) { char c; do { if(read(i, &c, 1) == 0) break; } while(c != '\n'); }

/**
 * mpw_spellcheck_word - Tests if a word is correctly spelled
 * @word: the word to be checked
 *
 * Tests if @word is correctly spelled. Returns MP_COLOR_NORMAL in case it is
 * or MP_COLOR_MISSPELLED otherwise.
 */
int mpw_spellcheck_word(char * word)
{
	static int ipipe[2] = { -1, -1 };
	char c, d;
	int n, color;

	if(! mpw_spellcheck)
	{
		/* is ispell pipe still open? */
		if(ipipe[0] != -1)
		{
			int s;

			close(ipipe[0]);
			close(ipipe[1]);

			wait(&s);

			ipipe[0]=ipipe[1]=-1;
		}

		return(MP_COLOR_NORMAL);
	}

	/* open ispell pipe if necessary */
	if(ipipe[0] == -1)
	{
		int p1[2], p2[2];

		pipe(p1); pipe(p2);

		if(fork() == 0)
		{
			/* child process; redirect stdin and stdout */
			close(0); dup(p2[0]); close(p2[1]);
			close(1); dup(p1[1]); close(p1[0]);

			execlp(mpw_ispell_command, mpw_ispell_command, "-a", NULL);

			/* still here? exec failed; close pipes and exit */
			close(0); close(1);
			exit(0);
		}

		ipipe[0]=p1[0]; close(p1[1]);
		ipipe[1]=p2[1]; close(p2[0]);

		/* read first character; if pipe couldn't be open, fail */
		if(read(ipipe[0], &c, 1) == 0)
		{
			mpw_spellcheck=0;
			mpv_alert(_("Can't execute '%s'"), mpw_ispell_command);
			return(MP_COLOR_NORMAL);
		}

		FLUSH_LINE(ipipe[0]);
	}

	/* avoid feeding non-words to ispell */
	if(word[0] == '\0') return(MP_COLOR_NORMAL);

	for(n = 0;word[n] != '\0';n++)
		if(!isalpha(word[n]))
			return(MP_COLOR_NORMAL);

	/* is the word already in the cache? */
	if((color=search_spell_cache(word)) != -1)
		return(color);

	/* write the word */
	write(ipipe[1], word, strlen(word));
	write(ipipe[1], &"\n", 1);

	/* read first character's response */
	read(ipipe[0], &c, 1);

	/* read until the end of the line */
	FLUSH_LINE(ipipe[0]);

	/* flush possible subsequent commands, in case ispell
	   treats word as different words */
	do {
		read(ipipe[0], &d, 1);
		if(d != '\n') FLUSH_LINE(ipipe[0]);
	} while(d != '\n');

	color=(c == '*' || c == '+' ? MP_COLOR_NORMAL : MP_COLOR_MISSPELLED);

	/* add to cache */
	add_spell_cache(word, color);

	return(color);
}

#else /* WINDOWS */

int mpw_spellcheck_word(char * word)
/* dummy version */
{
	if(mpw_spellcheck)
	{
		mpv_alert(_("Spellchecking is only available under Unix systems."), NULL);
		mpw_spellcheck=0;
	}

	return(MP_COLOR_NORMAL);
}


#endif
