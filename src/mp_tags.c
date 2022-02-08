/*

    mp - Programmer Text Editor

    Tag management.

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
#include "mp_iface.h"
#include "mp_tags.h"
#include "mp_lang.h"
#include "mp_video.h"

/*******************
	Data
********************/

/* tag index */

static struct tag_index * _tag_index=NULL;
static int _tag_index_size=0;

/* the ctags command */
char _mpt_ctags_cmd[128]="ctags *";


/*******************
	Code
********************/

static int _tag_cmp(const void * s1, const void * s2)
/* bsearch/qsort function for tags */
{
	struct tag_index * t1;
	struct tag_index * t2;

	t1=(struct tag_index *)s1;
	t2=(struct tag_index *)s2;

	return(strcmp(t1->tag, t2->tag));
}


static void _reset_tags(void)
/* clears the tag index */
{
	int n;

	/* frees all tags */
	for(n=0;n < _tag_index_size;n++)
	{
		struct tag_index * ti=&_tag_index[n];

		if(ti->tag != NULL) free(ti->tag);
		if(ti->file != NULL) free(ti->file);
		if(ti->target != NULL) free(ti->target);
	}

	/* frees and reset the index itself */
	free(_tag_index);
	_tag_index=NULL;
	_tag_index_size=0;
}


static void _add_tag(char * tag, char * file, char * target)
/* adds a new tag to the database */
{
	struct tag_index * ti;

	/* expands */
	_tag_index_size++;

	_tag_index=realloc(_tag_index, _tag_index_size *
		sizeof(struct tag_index));

	/* stores */
	ti=&_tag_index[_tag_index_size - 1];

	ti->tag=strdup(tag);
	ti->file=strdup(file);
	ti->target=strdup(target);
}


static void _sort_tags(void)
/* sorts the tag database */
{
	if(_tag_index_size > 0)
		qsort(_tag_index, _tag_index_size,
		sizeof(struct tag_index), _tag_cmp);
}


static int _read_tags_file(int force)
/* reads and parses a tags file, filling the database */
{
	FILE * f;
	char line[4096];
	char * file;
	char * target;
	char * ptr;

	if((f=fopen("tags","r")) == NULL)
	{
		if(force)
		{
			/* no 'tags' file; try to create one */
			system(_mpt_ctags_cmd);

			f=fopen("tags","r");
		}
	}

	/* it tags does not exist, return */
	if(f == NULL)
		return(0);

	/* reset the index */
	_reset_tags();

	MP_SAVE_STATE();

	while(fgets(line,sizeof(line),f) != NULL)
	{
		/* ignore tagfile comments */
		if(line[0] == '!')
			continue;

		/* find first tab */
		if((ptr=strchr(line,'\t')) == NULL)
			continue;

		*ptr='\0';
		ptr++;
		file=ptr;

		if((ptr=strchr(file,'\t')) == NULL)
			continue;

		*ptr='\0';
		ptr++;
		target=ptr;

		/* if it's a regular expression, treat it as a string */
		if(*target == '/')
		{
			/* ignore / and ^ */
			target++; target++;

			/* ignore possible spaces */
			for(;*target==' ' || *target=='\t';target++);

			/* go on until a separator is found */
			for(ptr=target;*ptr!='$' && *ptr!='\t'
				&& *ptr!='/' && *ptr!='\\' && *ptr;ptr++);
			*ptr='\0';
		}
		else
		{
			/* it is a line number */
			for(ptr=target;*ptr>='0' && *ptr<='9';ptr++);
			*ptr='\0';
		}

		/* index the tag */
		_add_tag(line, file, target);
	}

	fclose(f);

	_sort_tags();

	MP_RESTORE_STATE();

	return(1);
}


/**
 * mpt_seek_tag - Seeks a tag
 * @tag: tag to be search
 *
 * Seeks a tag. Returns the structure defining the tag, or NULL if
 * the tag is not found.
 */
struct tag_index * mpt_seek_tag(char * tag)
{
	struct tag_index t;

	if(_tag_index_size == 0) return(NULL);

	t.tag=tag;

	return(bsearch(&t, _tag_index, _tag_index_size,
		sizeof(struct tag_index), _tag_cmp));
}


/**
 * mpt_select_tag - Selects from a list of tags given a partial string
 * @partial_tag: Partial tag string to search
 *
 * Makes a partial seek in the tag database and shows a list for
 * the user to select a tag. If no tag is found, an error message is
 * shown and NULL returned; if the user cancel on the list, NULL is
 * also returned. Otherwise, a struct tag_index is returned.
 */
struct tag_index * mpt_select_tag(char * partial_tag)
{
	int n;
	mp_txt * txt;
	mp_txt * itxt;
	char tmp[16];
	struct tag_index * ti;
	int matches=0;

	/* if 'tags' file cannot be read, it's done */
	if(! _read_tags_file(1))
		return(NULL);

	/* create the helping texts */
	txt=mp_create_sys_txt("<tags>");
	itxt=mp_create_sys_txt("<tags:i>");

	MP_SAVE_STATE();

	/* fills the text with tag and file */
	for(n=0;n < _tag_index_size;n++)
	{
		ti=&_tag_index[n];

		/* if tag is not contained, skip */
		if(strstr(ti->tag, partial_tag) == NULL)
			continue;

		/* store tag */
		mp_put_str(txt, ti->tag, 1);

		/* store file */
		mp_put_char(txt, '\t', 1);
		mp_put_str(txt, ti->file, 1);

		/* store target */
		mp_put_char(txt, '\t', 1);
		mp_put_str(txt, ti->target, 1);

		mp_put_char(txt, '\n', 1);

		/* store the tag subscript */
		snprintf(tmp, sizeof(tmp), "%d", n);
		mp_put_str(itxt, tmp, 1);
		mp_put_char(itxt, '\n', 1);

		matches++;
	}

	/* deletes the last \n */
	mp_move_left(txt);
	mp_delete_char(txt);

	ti=NULL;

	/* show the list */
	if(matches)
	{
		if((n=mpv_list(L("Tag list"), txt, 0)) != -1)
		{
			/* move to line and get tag index */
			mp_move_xy(itxt, 0, n);
			mp_get_word(itxt, tmp, sizeof(tmp));

			ti=&_tag_index[atoi(tmp)];
		}
	}
	else
		mpv_alert(L("Tag(s) not found."), partial_tag);

	mp_delete_sys_txt(txt);
	mp_delete_sys_txt(itxt);

	MP_RESTORE_STATE();

	return(ti);
}


/**
 * mpt_open_tag - Selects a tag from a list and opens its location
 * @partial_tag: Partial tag string to search
 *
 * Shows a list of tags matching @partial_tag and, if the user selects
 * one, it's opened.
 */
void mpt_open_tag(char * partial_tag)
{
	struct tag_index * ti;
	int n;

	if((ti=mpt_select_tag(partial_tag)) == NULL)
		return;

	/* open without reopening */
	mpi_open(ti->file, 0);

	/* is it a line number or a string to search? */
	if(*ti->target >= '0' && *ti->target <= '9')
	{
		n=atoi(ti->target) - 1;
			mp_move_xy(_mp_active, 0, n);
	}
	else
	{
		/* copies it into search buffer
		   to return to tag by pressing ^L */
		/* strncpy(_mpi_search_text, ti->target,
			sizeof(_mpi_search_text)); */

		mp_log("mpt_open_tag: %s\n", ti->target);

		mp_move_bof(_mp_active);
		mp_seek_plain(_mp_active, ti->target);
	}
}


void mpt_startup(void)
/* tag startup function */
{
	_read_tags_file(0);
}


void mpt_shutdown(void)
/* tag shutdown function */
{
}
