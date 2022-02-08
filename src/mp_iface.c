/*

    mp_iface.c

    Interface.

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
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mp_core.h"
#include "mp_conf.h"
#include "mp_video.h"
#include "mp_synhi.h"
#include "mp_lang.h"
#include "mp_func.h"
#include "mp_iface.h"
#include "mp_tags.h"
#include "mp_wordp.h"

/********************
	 Data
 ********************/

/* word buffer */
static char _draw_word[128];

/* insert flag */
int _mpi_insert=1;

/* text to search */
static char _mpi_search_text[4096]="";

char * MP_LICENSE=
"\nMinimum Profit " VERSION " - Programmer Text Editor\n\n\
Copyright (C) 1991-2005 Angel Ortega <angel@triptico.com>\n\
\n\
This program is free software; you can redistribute it and/or\n\
modify it under the terms of the GNU General Public License\n\
as published by the Free Software Foundation; either version 2\n\
of the License, or (at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
See the GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program; if not, write to the Free Software Foundation,\n\
Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.\n\
\n\
Home page: http://www.triptico.com/software/mp.html\n\
Mailing list: mp-subscribe@lists.triptico.com\n";


/* macro */

#ifndef MAX_MACRO_SIZE
#define MAX_MACRO_SIZE 1024
#endif

struct
{
	int key;
	char * funcname;
} _mpi_macro[MAX_MACRO_SIZE];
int _mpi_macro_index=0;
int _mpi_macro_recording=0;

/* instant position (mouse click or so) */
int _mpi_instant_x=-1;
int _mpi_instant_y=-1;

/* mark column 80 */
int _mpi_mark_column_80=0;

/* exit requested */
int _mpi_exit_requested=0;

/* the template file */
char _mpi_template_file[1024]="~/.mp_templates";

/* move selecting flag */
int mpi_move_selecting=0;


/* experimental ftt code */

static char * _mpi_ftt_key="\x20\x5f\x2f\x7c\x5c\x28\x29\x6f\x2d";

static char * _mpi_ftt_challenge=
	"\xDB\xC6\xE5\xE1\xF2\xA0\xF4\xE8\xE5\xA0\xD4\xF2\xE9\xE3\xE5\xF2"
	"\xE1\xF4\xEF\xF0\xF3\xDD";

static unsigned char * _mpi_ftt_data=(unsigned char *)
	"\x00\x00\x11\x11\x11\x11\x00\x00\x00\x12\x00\x00\x00\x00\x41\x00"
	"\x02\x00\x34\x00\x00\x23\x00\x40\x30\x00\x30\x40\x02\x03\x00\x03"
	"\x30\x00\x57\x00\x00\x76\x00\x03\x04\x10\x04\x02\x40\x20\x01\x20"
	"\x03\x04\x10\x20\x04\x01\x20\x30\x03\x00\x33\x08\x80\x33\x00\x30"
	"\x03\x00\x30\x41\x12\x03\x00\x30\x56\x56\x56\x00\x00\x56\x56\x56\xFF";

/* number of lines to move up over current visible line,
   to take into account possible start of comments many lines above */
int _mpi_preread_lines=60;

/* monochrome mode */
int mpi_monochrome=0;

/* readline history */
mp_txt * _mpi_history[MPR_LAST + 1];

/* tag target over the cursor */
static char * _mpi_tag_target=NULL;

/* the status line format string */
char _mpi_status_line_f[128]="%m %x,%y [%l] %R%O %s %t";

/* the strftime format string */
char _mpi_strftime_f[128]="%x";

/* break hardlinks flag */
int _mpi_break_hardlinks=0;

/* user-set language */
char _mpi_lang[128]="";

/* move to line on open */
int _mpi_move_to_line=0;

/* seek to line */
int _mpi_seek_to_line = -1;

/*******************
	Code
*******************/

static char * _mpi_format_status_line(mp_txt * txt)
{
	static char str[1024];
	char tmp[128];
	char * ptr;
	int n, m;

	ptr=_mpi_status_line_f;

	for(n=0;*ptr != '\0' &&	n < sizeof(str) - 1;ptr++)
	{
		/* if it's not a format mark, continue */
		if(*ptr != '%')
		{
			str[n++]=*ptr;
			continue;
		}

		ptr++;

		switch(*ptr)
		{
		case 'm':
			/* %m: modified flag */
			tmp[0]=txt->mod ? '*': '\0';
			tmp[1]='\0';
			break;

		case 'x':
			/* %x: X coord */
			sprintf(tmp, "%d", txt->x + 1);
			break;

		case 'y':
			/* %y: Y coord */
			sprintf(tmp, "%d", txt->y + 1);
			break;

		case 'l':
			/* %l: number of lines */
			sprintf(tmp, "%d", txt->lasty + 1);
			break;

		case 'R':
			/* %R: 'recording mode' flag */
			tmp[0]=_mpi_macro_recording ? 'R': '\0';
			tmp[1]='\0';
			break;

		case 'O':
			/* %O: 'overwrite' text flag */
			tmp[0]=_mpi_insert ? '\0' : 'O';
			tmp[1]='\0';
			break;

		case 's':
			/* %s: Synhi description */
			if(txt->synhi)
				strncpy(tmp, _mps_synhi[txt->synhi - 1].type, sizeof(tmp));
			else
				tmp[0]='\0';

			break;

		case 'd':
			/* %d: date */
			{
				time_t t;

				t=time(NULL);
				strftime(tmp, sizeof(tmp) - 1,
					_mpi_strftime_f, 
					localtime(&t));
			}

			break;

		case 't':
			/* %t: tag */
			if(_mpi_tag_target != NULL)
				strncpy(tmp, _mpi_tag_target, sizeof(tmp));
			else
				tmp[0]='\0';
			break;

		default:
			tmp[0]=*ptr;
			tmp[1]='\0';
			break;
		}

		/* transfer */
		for(m=0;tmp[m] != '\0' && n < sizeof(str) - 1;m++,n++)
			str[n]=tmp[m];
	}

	str[n]='\0';

	return(str);
}


/**
 * mpi_color_draw_all - Draws the document window using colors.
 * @txt: the text
 *
 * Draws the document window, with syntax highlighting decorations and
 * other colors, if applicable.
 */
void mpi_color_draw_all(mp_txt * txt)
{
	int vx,vy;
	mp_txt * wrk;
	int n,m,r;
	int c,wc,color,rcolor;
	int wi;
	int spcs;
	int ry;
	int xcursor=0,ycursor=0;

	mpv_title(NULL);

	mp_adjust(txt,_mpv_x_size,_mpv_y_size - 1);
	mp_match_bracket(txt, _mpv_x_size * _mpv_y_size);

	vx=txt->vx;
	vy=txt->vy;

	wrk=mp_get_tmp_txt(txt);

	mp_move_bol(wrk);

	/* moves up to first visible line */
	while(wrk->y > (vy - _mpi_preread_lines))
	{
		if(! mp_move_up(wrk))
			break;
	}

	_in_comment=-1;
	_mpi_tag_target=NULL;

	/* line loop */
	for(;;)
	{
		/* column loop */

		ry=wrk->y;

		/* move this outside the loop to allow
		   multiline quoted strings (you shouldn't) */
		_draw_quoting='\0';

		/* end if below the last line */
		if(ry >= (vy + _mpv_y_size))
			break;

		c=mp_get_char(wrk);

		/* move drawing cursor to beginning of line */
		if(ry >= vy)
			mpv_goto(0,ry - vy);

		for(m=r=0;;)
		{
			/* word loop */

			wi=0;
			spcs=mps_is_sep(c,txt->synhi);

			while(wi<sizeof(_draw_word)-1)
			{
				if(c=='\n' || c=='\0')
					break;

				_draw_word[wi++]=c;

				c=mp_get_char(wrk);

				if(spcs != mps_is_sep(c,txt->synhi))
				{
					if(_draw_word[wi - 1]!='\\')
						break;
				}
			}

			_draw_word[wi]='\0';

			rcolor=mps_word_color(txt->synhi,_draw_word, m, ry);

			if(rcolor == MP_COLOR_NORMAL)
				rcolor=mpw_spellcheck_word(_draw_word);

			if(c=='\n' || c=='\0')
			{
				_draw_word[wi++]=' ';
				_draw_word[wi]='\0';
			}

			/* draws the visible chars of the word */
			for(wi=0;(wc=_draw_word[wi])!='\0';wi++,m++)
			{
				color=mps_quoting(wc,rcolor,txt->synhi);

				/* if inside selection block... */
				if(ry > txt->mby && ry <= txt->mey)
					color=MP_COLOR_SELECTED;

				if(ry == txt->mby && m >= txt->mbx)
					color=MP_COLOR_SELECTED;

				if(ry == txt->mey && m >= txt->mex)
					color=rcolor;

				/* if over the cursor... */
				if(m == txt->x && ry == txt->y)
				{
					if(rcolor == MP_COLOR_LOCAL)
					{
						/* gets current tag target */
						_mpi_tag_target=
							_mps_last_tag_target;
					}

					xcursor=m-vx;
					ycursor=ry-vy;
					color=MP_COLOR_CURSOR;
				}

				/* if over the matching bracket... */
				if(m == txt->brx && ry == txt->bry)
					color=MP_COLOR_BRACKET;

				if(_mpi_mark_column_80 && r==80)
					color=MP_COLOR_SELECTED;

				if(txt->type == MP_TYPE_LIST && ry == txt->y)
					color=MP_COLOR_CURSOR;

				/* finally draws */
				if(wc=='\t')
				{
					int i;

					for(i=MP_REAL_TAB_SIZE(r);i > 0;i--,r++)
					{
						if(r >= vx && r < (vx+_mpv_x_size))
							mpv_char(' ', color);

						if(txt->type != MP_TYPE_LIST &&
						   color != MP_COLOR_SELECTED)
							color=MP_COLOR_NORMAL;
					}
				}
				else
				{
					if(r >= vx && r < (vx+_mpv_x_size))
						mpv_char(wc,color);
					r++;
				}
			}

			if(c=='\n' || c=='\0')
				break;
		}

		/* spaces to end of line */
		if(r < vx)
			r=vx;

		for(;r < vx+_mpv_x_size;r++)
		{
			if(_mpi_mark_column_80 && r==80)
				mpv_char(' ',MP_COLOR_SELECTED);
			else
			if(txt->type == MP_TYPE_LIST &&
			   ry == txt->y)
				mpv_char(' ',MP_COLOR_CURSOR);
			else
				mpv_char(' ',MP_COLOR_NORMAL);
		}

		/* if last read char is '\0', it's the end */
		if(c=='\0')
			break;

		/* if we are not at the end of the line, move there */
		if(c!='\n')
		{
			mp_move_bol(wrk);

			if(! mp_move_down(wrk))
				break;
		}
	}

	/* the rest of lines are drawn as blanks */
	for(n=ry - vy;n <= _mpv_y_size;n++)
	{
		mpv_goto(0,n+1);

		for(m=0;m < _mpv_x_size;m++)
		{
			if(_mpi_mark_column_80 && m==80)
				mpv_char(' ',MP_COLOR_SELECTED);
			else
				mpv_char(' ',MP_COLOR_NORMAL);
		}
	}

	mp_end_tmp_txt();

	/* scrollbar */
	mpv_scrollbar(txt->vy+1,_mpv_y_size,txt->lasty+1);

	/* status line */
	mpv_status_line(_mpi_format_status_line(txt));

	mpv_goto(0,_mpv_y_size);
	mpv_cursor(xcursor,ycursor);

	mpv_refresh();
}


/**
 * mpi_draw_all - Draws the document window.
 * @txt: the text
 *
 * Draws the document window.
 */
void mpi_draw_all(mp_txt * txt)
{
	if(txt != NULL)
		mpi_color_draw_all(txt);
}


/**
 * mpi_move_wheel_up - Moves preferred number of rows up
 * @txt: the text
 *
 * Moves preferred number of rows up.
 * Moves a full page if preferred is 0 or greater than a page of rows.
 */
int mpi_move_wheel_up(mp_txt * txt)
{
	int n;
	
	if ((_mp_wheel_scroll_rows) > 0 && (_mp_wheel_scroll_rows < _mpv_y_size)) {
		for (n = 0; n < _mp_wheel_scroll_rows; n++)
			mp_move_up(txt);
	} else
		mpi_move_page_up(txt);
	return(1);
}

 
/**
 * mpi_move_page_up - Moves one page up
 * @txt: the text
 *
 * Moves one page up.
 */
int mpi_move_page_up(mp_txt * txt)
{
	int n;

	for(n=0;n < _mpv_y_size;n++)
		mp_move_up(txt);

	return(1);
}


/**
 * mpi_move_wheel_down - Moves preferred number of rows down
 * @txt: the text
 *
 * Moves preferred number of rows down.
 * Moves a full page if preferred is 0 or greater than a page of rows.
 */
int mpi_move_wheel_down(mp_txt * txt)
{
	int n;
	
	if ((_mp_wheel_scroll_rows) > 0 && (_mp_wheel_scroll_rows < _mpv_y_size)) {
		for (n = 0; n < _mp_wheel_scroll_rows; n++)
			mp_move_down(txt);
	} else
		mpi_move_page_down(txt);
	return(1);
}

 
/**
 * mpi_move_page_down - Moves one page down
 * @txt: the text
 *
 * Moves one page down.
 */
int mpi_move_page_down(mp_txt * txt)
{
	int n;

	for(n=0;n < _mpv_y_size;n++)
		mp_move_down(txt);

	return(1);
}


/**
 * mpi_goto - Moves to a line by its number
 * @txt: the txt
 *
 * Asks for a line number and moves the cursor there.
 */
int mpi_goto(mp_txt * txt)
{
	char * ptr;
	int n;

	if((ptr=mpv_readline(MPR_GOTO,_("Line to go to: "),NULL))!=NULL)
	{
		n=atoi(ptr);
		mp_move_xy(txt,0,n-1);
	}

	return(1);
}


/**
 * mpi_new - Creates a new text
 *
 * Creates an empty unnamed text, unless an unnamed text
 * already exists; in that case, it's made the current one.
 */
int mpi_new(void)
{
	mp_txt * txt;

	if((txt=mp_find_txt(_("<unnamed>")))!=NULL)
		_mp_active=txt;
	else
	{
		mp_create_txt(_("<unnamed>"));
		mps_auto_synhi(_mp_active);
	}

	return(2);
}

/**
 * mpi_open - Loads a file.
 * @name: the file name.
 * @reopen: if reopening is allowed.
 *
 * Opens a file and sets it as the current one, or
 * otherwise communicates the error to the user.
 */
void mpi_open(char * name, int reopen)
{
	mp_txt * txt;
	FILE * f;

	/* if no name, ask for it */
	if(name == NULL &&
		(name=mpv_readline(MPR_OPEN,_("Enter file name: "),NULL)) == NULL)
		return;

	/* if file is already open and don't want to reopen, just select it */
	if(!reopen && (txt=mp_find_txt(name)) != NULL)
	{
		_mp_active=txt;
		return;
	}

	mp_create_txt(name);
	if((f=mpv_fopen(name, "r")) != NULL)
	{
		/* if file is encrypted... */
		if(mp_get_encryption_format(f) > 0)
		{
			char * passwd;

			/* ...ask for a password... */
			if((passwd=mpv_readline(MPR_PASSWORD,
				_("Password:"), NULL)) == NULL ||
				passwd[0] == '\0')
			{
				/* cancelled: close everything */
				fclose(f);
				mp_delete_txt(_mp_active);

				return;
			}

			/* ...and set it */
			mp_set_password(_mp_active, passwd);
		}

		mp_load_file(_mp_active,f);
		fclose(f);

		_mp_active->mod=0;
		mps_auto_synhi(_mp_active);
	}
	else
	{
#ifdef CLASSIC_BEHAVIOUR
		mpv_alert(_("File '%s' not found."), name);
		mp_delete_txt(_mp_active);
#else
		mp_log("Can't open '%s'... creating new.\n", name);
#endif
	}

}


/**
 * mpi_save_as - Asks for a name for the text and save.
 * @txt: the text
 *
 * Asks for a name for the text and saves it. If the
 * text already has a name, it is replaced.
 * On error, communicates it to the user.
 */
int mpi_save_as(mp_txt * txt)
{
	char * name;

	if(txt->type == MP_TYPE_READ_ONLY) return(0);

	if((name=mpv_readline(MPR_SAVE,_("Enter file name: "),
		*txt->name == '<' ? NULL : txt->name)) == NULL)
		return(0);

	mp_name_txt(txt,name);
	mps_auto_synhi(txt);

	return(mpi_save(txt));
}


/**
 * mpi_save - Saves the text.
 * @txt: the text
 *
 * Saves the text. If it already has a name, saves directly
 * to disk, otherwise asks for one using mpi_save_as().
 */
int mpi_save(mp_txt * txt)
{
	FILE * f;

	if(txt->type == MP_TYPE_READ_ONLY) return(0);

	if(txt->name[0] == '<')
		return(mpi_save_as(txt));

	mps_auto_synhi(txt);

	if((f=mpv_fopen(txt->name, "w")) != NULL)
	{
		mp_save_file(txt, f);
		fclose(f);

		txt->mod=0;
	}
	else
	{
		mpv_alert(_("Can't create file '%s'."), txt->name);
		mp_name_txt(txt,_("<unnamed>"));
	}

	return(2);
}


/**
 * mpi_sync - Synchronizes the modified texts to disk
 *
 * Synchonizes the modified texts saving its contents to disk.
 */
void mpi_sync(void)
{
	mp_txt * txt;

	for(txt=_mp_txts;txt!=NULL;txt=txt->next)
	{
		if(txt->mod)
		{
			mpv_status_line(txt->name);
			mpi_save(txt);
		}
	}
}


/**
 * mpi_close - Closes the text.
 * @txt: the text
 *
 * Closes the text. If it has changed, asks user if
 * file must be saved; if ok, file is saved using mpi_save().
 */
int mpi_close(mp_txt * txt)
{
	if(txt->mod)
	{
		if(mpv_confirm(_("File has changed. Save changes?")))
			mpi_save(txt);
	}

	mp_delete_txt(txt);
	return(2);
}


int mpi_history_size(int mode)
{
	return(_mpi_history[mode]->lasty);
}


int mpi_history_get(int mode, int index, char * buf, int size)
{
	if(index < 0 || index > _mpi_history[mode]->lasty)
		return(0);

	mp_move_xy(_mpi_history[mode], 0, index);
	mp_get_str(_mpi_history[mode], buf, size, '\n');

	return(1);
}


void mpi_history_add(int mode, char * str)
{
	/* don't add empty strings */
	if(*str=='\0') return;

	/* never add history to passwords */
	if(mode == MPR_PASSWORD) return;

	mp_move_eof(_mpi_history[mode]);
	mp_move_eol(_mpi_history[mode]);
	mp_put_char(_mpi_history[mode], '\n', 1);
	mp_put_str(_mpi_history[mode], str, 1);
}


/**
 * mpi_exec - Executes a command related to text.
 * @txt: the text
 * @cmd: the command to execute
 *
 * Asks for a system command to be executed if none was specified
 * in @cmd.
 * If the command is preceded by the | (pipe) char, the complete text
 * is sent as the command's standard input;
 * If the command is preceded by the < char, the standard output is
 * displayed in a new read-only buffer.  If the character following <
 * is another <, then the text between a < > will be used as the buffer
 * title.
 * If the command is preceded by the @ char, the command is executed
 * as a spawned. i.e. the command gets full control of the screen.
 * Otherwise, the standard output of the command is written into
 * cursor position.
 */
int mpi_exec(mp_txt * txt, char *cmd)
{
	int res = -1;
	int ret = 0;

	/* if no cmd, ask for one */
	if (cmd == NULL)
		cmd=mpv_readline(MPR_EXEC, _("System command: "), NULL);

	/* cancel or empty; return now */
	if(cmd == NULL || *cmd == '\0')
		return(0);

	switch (*cmd)
	{
	case '|':
		/* send text to program's input */
      		res=mpv_syscmd(txt, cmd + 1, "w");
		break;

	case '<':
		/* read standard output into a new read-only buffer */
		{
		char tmp[1024];

		if (cmd[1] == '<')
		{
			char *p = tmp;

			++cmd;
			while (*cmd != '>')
				*(p++) = *(cmd++);

			*(p++) = *cmd;
			*p = 0;
		}
		else
	  		snprintf(tmp, sizeof(tmp), _("<Output of \"%s\">"), cmd + 1);

		txt = mp_find_txt(tmp);
		if (txt) mp_delete_txt(txt);
		txt = mp_create_txt(tmp);
		}

		res = mpv_syscmd(txt, cmd + 1, "r");

		txt->type=MP_TYPE_READ_ONLY;
		txt->mod=0;
		ret = 2;

		break;

	case '@':
		/* Let the external command take control of the screen */
		res = mpv_syscmd(NULL, cmd + 1, NULL);
		ret = 0;

		break;

	default:
		/* insert program's output into cursor position */
		res = mpv_syscmd(txt, cmd, "r");
		ret = 1;
		break;
	}

	if(res)
		mpv_alert(_("Error executing command."), cmd);

	return(ret);
}


/**
 * mpi_current_list - Shows a selection list with the opened files
 *
 * Shows a selection list with the opened files. Selecting one of
 * them makes it the active one.
 */
void mpi_current_list(void)
{
	mp_txt * list;
	mp_txt * txt;
	int l, pos;
	char tmp[1024];

	/* no texts, no list; no woman, no cry */
	if(_mp_txts==NULL) return;

	MP_SAVE_STATE();

	list=mp_create_sys_txt(_("<open files>"));

	/* travels the open file list */
	for(txt=_mp_txts,l=pos=0;txt!=NULL;txt=txt->next, l++)
	{
		if(txt->mod)
			mp_put_str(list, "* ", 1);

		mp_put_str(list, txt->name, 1);
		mp_put_char(list, '\n', 1);

		if(txt == _mp_active)
			pos=l;
	}

	mp_move_left(list);
	mp_delete_char(list);

	MP_RESTORE_STATE();

	if((l=mpv_list(_("Open documents"), list, pos))!=-1)
	{
		char * ptr;

		mp_move_xy(list, 0, l);
		mp_get_str(list, tmp, sizeof(tmp), '\n');

		ptr=tmp;

		/* skip possible modified marks */
		if(*ptr == '*' && *(ptr + 1) == ' ')
			ptr += 2;

		if((txt=mp_find_txt(ptr)) != NULL)
			_mp_active=txt;
	}

	mp_delete_sys_txt(list);
}


/**
 * mpi_insert_template - Shows a list with the available templates
 *
 * Shows a list with the available templates to select one.
 * If ENTER is pressed, the template is inserted into cursor
 * position.
 */
int mpi_insert_template(void)
{
	mp_txt * txt;
	mp_txt * t;
	FILE * f;
	char line[1024];
	int n,l;

	if((f=fopen(_mpi_template_file,"r"))==NULL)
	{
		mpv_alert(_("Template file not found (%s)"),_mpi_template_file);
		return(0);
	}

	t=_mp_active;

	txt=mp_create_sys_txt(_mpi_template_file);

	MP_SAVE_STATE();

	/* inserts all titles in the list */
	while(fgets(line,sizeof(line),f)!=NULL)
	{
		if(line[0]=='%' && line[1]=='%')
			mp_put_str(txt,&line[2],1);
	}

	fclose(f);

	mp_move_left(txt);
	mp_delete_char(txt);

	if((l=mpv_list(_("Select template"), txt, 0))!=-1)
	{
		/* template has been selected: find and insert */
		_mp_active=t;

		f=fopen(_mpi_template_file,"r");

		for(n=-1;n < l;)
		{
			if(fgets(line,sizeof(line),f)==NULL)
				break;

			if(line[0]=='%' && line[1]=='%')
			{
				if(++n==l)
					break;
			}
		}

		if(n==l)
		{
			/* insert into current text */
			while(fgets(line,sizeof(line),f)!=NULL)
			{
				if(line[0]=='%' && line[1]=='%')
					break;

				mp_put_str(_mp_active,line,1);
			}
		}

		fclose(f);
	}

	MP_RESTORE_STATE();

	mp_delete_sys_txt(txt);

	return(2);
}


static void _mpi_insert_ftt_data(mp_txt * txt, char * key,
	unsigned char * ftt)
/* ftt code insertion (experimental) */
{
	int n,c;

	for(n=0;(c=ftt[n])!=0xff;n++)
	{
		if(n%8==0) mp_insert_line(txt);

		mp_put_char(txt,key[(c&0xf0)>>4],1);
		mp_put_char(txt,key[(c&0x0f)],1);
	}

	mp_insert_line(txt);
}


static void _mpi_test_ftt(int c)
/* ftt code testing (experimental) */
{
	static int ftt=0;

	if(c=='\0') return;

	/* test ftt insertion */
	if((_mpi_ftt_challenge[ftt] & 0x7f) == c)
	{
		ftt++;

		if(_mpi_ftt_challenge[ftt]=='\0')
		{
			_mpi_insert_ftt_data(_mp_active,
				_mpi_ftt_key, _mpi_ftt_data);
			ftt=0;
		}
	}
	else
		ftt=0;
}


/**
 * mpi_seek - Seeks a text
 * @txt: the text
 *
 * Asks for a string to be searched and searches the text,
 * positioning the cursor there if it's found or signalling
 * the user otherwise.
 */
int mpi_seek(mp_txt * txt)
{
	char * ptr;

	if((ptr=mpv_readline(MPR_SEEK,_("Text to seek: "),NULL))!=NULL)
	{
		strncpy(_mpi_search_text,ptr,sizeof(_mpi_search_text));

		if(!mp_seek(txt,_mpi_search_text))
			mpv_alert(_("Text not found."),_mpi_search_text);
		else
		if(_mpi_seek_to_line > 0)
			if((txt->vy = txt->y - _mpi_seek_to_line) < 0)
				txt->vy = txt->y;
	}

	return(1);
}


/**
 * mpi_seek_next - Seeks a text again
 * @txt: the text
 *
 * Seeks again the previously entered string in a text,
 * positioning the cursor there if it's found or signalling
 * the user otherwise.
 */
int mpi_seek_next(mp_txt * txt)
{
	if(!mp_seek(txt,_mpi_search_text))
		mpv_alert(_("Text not found."),_mpi_search_text);
	else
	if(_mpi_seek_to_line > 0)
		if((txt->vy = txt->y - _mpi_seek_to_line) < 0)
			txt->vy = txt->y;

	return(1);
}


/**
 * mpi_replace - Searchs and replaces a text
 * @txt: the txt
 *
 * Asks for two strings, one to be searched and the other to
 * replace the first one if found. It also asks if this replacement
 * should be done to the end of the text. If found, the cursor
 * is left there, or the user is informed otherwise.
 */
int mpi_replace(mp_txt * txt)
{
	char * ptr;

	if((ptr=mpv_readline(MPR_REPLACETHIS,
		_("Replace text: "),NULL))!=NULL)
	{
		strncpy(_mpi_search_text,ptr,sizeof(_mpi_search_text));

		if((ptr=mpv_readline(MPR_REPLACEWITH,
			_("Replace with: "),NULL))!=NULL)
		{
			if(mpv_confirm(_("To end of file?")))
			{
				int c=0;

				while(mp_replace(txt,
					_mpi_search_text,ptr))
					c++;

				mp_log("replace: %d replaces\n",c);
			}
			else
			{
				if(!mp_replace(txt,
					_mpi_search_text,ptr))
					mpv_alert(_("Text not found."),
					_mpi_search_text);
			}
		}
	}

	return(1);
}


/**
 * mpi_replace_all - Replaces a string in all open files
 *
 * Asks for two strings, one to be searched and the other to
 * replace the first one if found. The search+replace is
 * performed onto all the currently open texts. No information
 * is shown to the user.
 */
int mpi_replace_all(void)
{
	char * ptr;

	if((ptr=mpv_readline(MPR_REPLACETHIS,
		_("Replace text: "),NULL))!=NULL)
	{
		strncpy(_mpi_search_text,ptr,sizeof(_mpi_search_text));

		if((ptr=mpv_readline(MPR_REPLACEWITH,
			_("Replace with: "),NULL))!=NULL)
		{
			mp_txt * t;

			for(t=_mp_txts;t!=NULL;t=t->next)
			{
				mp_move_bof(t);

				mpv_status_line(t->name);
				mpv_refresh();

				while(mp_replace(t,_mpi_search_text,ptr));
			}
		}
	}

	return(1);
}


/**
 * mpi_grep - Greps (searches text) in several files
 *
 * Asks for a string and a file spec and scans the files
 * searching for the string. A list with the search hits
 * is shown, asking the user to select one; the file
 * will be open (if necessary) and the cursor moved to
 * the selected line.
 */
int mpi_grep(void)
{
	char * ptr;
	mp_txt * txt;
	mp_txt * hits;
	char tmp[1024];
	char file[1024];
	FILE * f;
	int line, l;
	int files, matches;

	/* gets the word over the cursor */ 
	mp_get_word(_mp_active,_mpi_search_text,sizeof(_mpi_search_text));

	/* ask for the search string */
	if((ptr=mpv_readline(MPR_SEEK,_("Text to seek: "),
		_mpi_search_text))==NULL || *ptr=='\0')
		return(0);

	strncpy(_mpi_search_text,ptr,sizeof(_mpi_search_text));

	/* ask for the file spec */
	if((ptr=mpv_readline(MPR_GREPFILES,_("Files to grep (empty, all): "),NULL))==NULL)
		return(0);

	if((txt=mpv_glob(ptr))==NULL)
	{
		mpv_alert(_("File '%s' not found."), ptr);
		return(0);
	}

	if((hits=mp_create_sys_txt("<grep_hits>"))==NULL)
	{
		mp_delete_sys_txt(txt);
		return(0);
	}

	mp_move_bof(txt);

	/* loops the files */
	files=matches=0;
	while(mp_get_str(txt,file,sizeof(file),'\n'))
	{
		if((f=fopen(file,"r"))==NULL)
			continue;

		files++;
		line=1;

		/* loops each file */
		while(fgets(tmp,sizeof(tmp),f)!=NULL)
		{
			if(tmp[strlen(tmp)-1]=='\n')
			{
				tmp[strlen(tmp)-1]='\0';
				l=1;
			}
			else
				l=0;

			/* FIXME: this should be a regular
			   expression (bug #1031) */
			if(strstr(tmp, _mpi_search_text)!=NULL)
			{
				mp_put_str(hits,tmp,1);
				mp_move_bol(hits);

				mp_put_str(hits,file,1);

				sprintf(tmp,"\t%d\t",line);
				mp_put_str(hits,tmp,1);

				mp_move_eol(hits);
				mp_put_char(hits,'\n',1);

				matches++;
			}

			/* increment line number only if the line
			   fitted into tmp */
			if(l) line++;
		}

		fclose(f);
	}

	/* destroy the file globbing */
	mp_delete_sys_txt(txt);

	if(files == 0)
	{
		/* no matching file was found */
		mpv_alert(_("File '%s' not found."), ptr);
		return(2);
	}

	if(matches == 0)
	{
		/* no file matched the string */
		mpv_alert(_("Text not found."), tmp);
		return(2);
	}

	mp_move_left(hits);
	mp_delete_char(hits);

	if((line=mpv_list(_("grep"), hits, 0))==-1)
	{
		mp_delete_sys_txt(hits);
		return(2);
	}

	/* list was accepted: pick file and line */
	mp_move_xy(hits,0,line);
	mp_get_str(hits,file,sizeof(file),'\t');
	mp_get_str(hits,tmp,sizeof(tmp),'\t');
	line=atoi(tmp);

	/* open the file */
	mpi_open(file,0);

	/* move to that line */
	mp_move_xy(_mp_active,0,line-1);

	mp_delete_sys_txt(hits);

	return(2);
}


/**
 * mpi_help - Cries for help
 * @txt: the text
 *
 * Takes the word below the cursor and asks the system
 * for help.
 */
int mpi_help(mp_txt * txt)
{
	mp_get_word(txt, _draw_word, sizeof(_draw_word));

	mpv_help(_draw_word,txt->synhi);

	return(2);
}


/**
 * mpi_find_tag - Asks for a tag and finds it
 * @txt: the text
 *
 * Asks for a tag and searches it. If tag is found, the
 * file that has it is shown, or the user is informed otherwise.
 */
int mpi_find_tag(mp_txt * txt)
{
	char * ptr;

	mp_get_word(txt,_draw_word,sizeof(_draw_word));

	if((ptr=mpv_readline(MPR_TAG,_("Tag to seek: "),_draw_word)) != NULL)
		mpt_open_tag(ptr);

	return(2);
}


/**
 * mpi_set_word_wrap - Asks for a word wrapping value
 *
 * Asks for a word wrapping value and sets it.
 */
int mpi_set_word_wrap(void)
{
	char * ptr;

	if((ptr=mpv_readline(MPR_WORDWRAP,_("Word wrap on column (0, no word wrap): "),NULL))!=NULL)
	{
		_mp_word_wrap=atoi(ptr);

		if(_mp_word_wrap < 0)
			_mp_word_wrap=0;
	}

	return(0);
}


/**
 * mpi_set_tab_size - Asks for a tab size value
 *
 * Asks for a tab size value and sets it.
 */
int mpi_set_tab_size(void)
{
	char * ptr;

	if((ptr=mpv_readline(MPR_TABSIZE,_("Tab size: "),NULL))!=NULL)
	{
		_mp_tab_size=atoi(ptr);

		if(_mp_tab_size <= 0 || _mp_tab_size > 40)
			_mp_tab_size=DEFAULT_TAB_SIZE;
	}

	return(2);
}


/**
 * mpi_completion - Tries to complete a symbol
 * @txt: the text
 *
 * Takes the word below the cursor and tries to find it in
 * the tags. The matching tags are shown in a list,
 * and the user is asked for selection; upon user confirmation,
 * the word below the cursor is replaced by the selected tag.
 */
int mpi_completion(mp_txt * txt)
{
	struct tag_index * ti;
	char tmp[128];
	int n;

	mp_move_left(txt);
	mp_get_word(txt,tmp,sizeof(tmp));
	mp_move_right(txt);

	if((ti=mpt_select_tag(tmp)) != NULL)
	{
		for(n=strlen(tmp);n > 0;n--)
		{
			mp_move_left(txt);
			mp_delete_char(txt);
		}

		mp_put_str(txt, ti->tag, 1);
	}

	return(1);
}


static void mpi_store_in_macro(int c, char * funcname)
{
	if(_mpi_macro_recording)
	{
		if(funcname!=NULL &&
		   (strcmp(funcname,"record-macro")==0 ||
		    strcmp(funcname,"play-macro")==0))
			return;

		_mpi_macro[_mpi_macro_index].key=c;
		_mpi_macro[_mpi_macro_index].funcname=funcname;

		if(++_mpi_macro_index == MAX_MACRO_SIZE)
			_mpi_macro_recording=0;
	}
}


/**
 * mpi_record_macro - Starts/stops recording a macro
 *
 * Starts or stops recording a macro.
 */
void mpi_record_macro(void)
{
	if(! _mpi_macro_recording)
		_mpi_macro_index=0;

	_mpi_macro_recording^=1;
}


/**
 * mpi_play_macro - Plays the previously recorded macro
 *
 * Plays the previously recorded macro.
 */
void mpi_play_macro(void)
{
	int n;

	if(_mpi_macro_recording) return;

	for(n=0;n < _mpi_macro_index;n++)
		mpi_process(_mpi_macro[n].key, NULL,
			    _mpi_macro[n].funcname);
}


/**
 * mpi_exec_function - Executes an editor function
 *
 * Asks for an internal editor function name and
 * executes it on the currently active text. Returns
 * the function's exit code.
 */
int mpi_exec_function(void)
{
	char * funcname;
	int ret=0;

	/* fills the 'history' with all available editor functions */
	mpf_get_funcnames(_mpi_history[MPR_EXECFUNCTION]);

	if((funcname=mpv_readline(MPR_EXECFUNCTION,
		_("Function to execute: "),NULL))!=NULL)
	{
	  char **args = mpf_makeargs(funcname);
		
	  if (args) {
	    ret = mpf_call_func_by_funcname(args[0], args[1] ? args+1 : NULL);
	    if (ret == -1) {
	      ret = 0;
	      mpv_alert(_("Function not found (%s)"),funcname);
	    }
	    free(args);
	  }
	}
	return(ret);
}


/**
 * mpi_process - Main action processing function
 * @c: character pressed
 * @key_name: name of the key to process
 * @func_name: name of the function to execute
 *
 * Main action processing function. If @keyname is not null,
 * the function associated to it is executed; else, if @funcname
 * is not null, the function with that name is executed;
 * otherwise, the @c character is inserted into the
 * currently active text. Based upon the function return
 * value, a complete or partial redraw is ordered to the
 * underlying driver; other actions as macro processing
 * or menu toggling are done.
 */
int mpi_process(int c, char * key_name, char * func_name)
{
	int ret=0;
	int * i;

	/* if all is known is the keyname, resolve the function */
	if(func_name==NULL && key_name!=NULL)
	{
		if((func_name=mpf_get_funcname_by_keyname(key_name))==NULL)
		{
			mp_log("Unbound key '%s'\n", key_name);
			return(0);
		}
	}

	/* store char or function in macro,
	   if recording */
	mpi_store_in_macro(c, func_name);

	/* process shift+movement key selection */
	if(mpi_move_selecting)
	{
		if(func_name!=NULL && strncmp(func_name,"move-",5)==0)
		{
			if(!mp_marked(_mp_active))
				mp_mark(_mp_active);
		}
		else
			mpi_move_selecting=0;
	}

	/* get the real function */
	if(func_name != NULL)
	{
		ret = mpf_call_func_by_funcname(func_name,NULL);
		if (ret == -1) ret = 0;
	}
	else
	if(c!='\0')
		/* write the char in the active text */
		ret=mp_put_char(_mp_active, c, _mpi_insert);

	if(ret==2)
	{
		/* if no text exists, create a new empty one */
		if(!_mp_active)
		{
			mp_create_txt(_("<unnamed>"));
			mps_auto_synhi(_mp_active);
		}

		/* anyway, redraw title */
		mpv_title(NULL);

		/* redraw open file tabs, as status
		   has probably changed */
		mpv_filetabs();
	}

	if(mpi_move_selecting)
		mp_mark(_mp_active);

	_mpi_test_ftt(c);

	if(ret)
		mpi_draw_all(_mp_active);

	if(func_name != NULL && (i=mpf_toggle_function_value(func_name))!=NULL)
		mpv_check_menu(func_name, *i);

	return(ret);
}


/**
 * mpi_args_1 - Command line argument processing, 1st pass
 * @argc: argument count
 * @argv: the arguments
 *
 * First pass to argument processing, mainly on/off switches.
 * Returns -1 if usage printing needed, -2 if version number,
 * 0 otherwise.
 */
int mpi_args_1(int argc, char * argv[])
{
	int n;

	/* first pass: general switches, previous to initialization */
	for(n=1;n < argc;n++)
	{
		if(argv[n] == NULL)
			continue;

		if(strcmp(argv[n],"--col80") == 0)
			_mpi_mark_column_80=1;
		else
		if(strcmp(argv[n],"--autoindent") == 0 ||
		   strcmp(argv[n],"-ai") == 0)
			_mp_auto_indent ^= 1;
		else
		if(strcmp(argv[n], "--monochrome") == 0 ||
		   strcmp(argv[n], "-bw") == 0)
			mpi_monochrome=1;
		else
		if(strcmp(argv[n],"-l") == 0 ||
		   strcmp(argv[n],"--lang") == 0)
		{
			if(n < argc - 1)
			{
				argv[n++]=NULL;
				strncpy(_mpi_lang, argv[n], sizeof(_mpi_lang));
			}
		}
		else
		if(strcmp(argv[n],"-tx") == 0 ||
		   strcmp(argv[n],"--text") == 0)
			_mpv_text=1;
		else
		if(strcmp(argv[n], "-sp") == 0 ||
		   strcmp(argv[n], "--spellcheck") == 0)
			mpw_spellcheck=1;
		else
		if(strcmp(argv[n], "-i") == 0 ||
		   strcmp(argv[n], "--interface") == 0)
		{
			/* undocumented */
			if(n < argc - 1)
			{
				argv[n++]=NULL;
				strncpy(_mpv_interface, argv[n],
					sizeof(_mpv_interface));
			}
		}
		else
		if(argv[n][0] == '+')
		{
			/* line number to go to in the first open file */
			_mpi_move_to_line=atoi(&argv[n][1]);
		}
		else
			continue;

		argv[n]=NULL;
	}

	return(0);
}


/**
 * mpi_args_2 - Command line argument processing, 2nd pass
 * @argc: argument count
 * @argv: the arguments
 *
 * Second pass to argument processing, commands and files to load.
 * Returns -1 if an invalid mode is requested, -2 if non-
 * existing tags are requested, 0 otherwise.
 */
int mpi_args_2(int argc, char * argv[])
{
	int n;

	/* second pass: switches with args and files */
	for(n=1;n < argc;n++)
	{
		if(argv[n] == NULL)
			continue;

		if(strcmp(argv[n],"--help") == 0 ||
		   strcmp(argv[n],"-h") == 0)
			return(-1);
		else
		if(strcmp(argv[n],"-t")==0 ||
		   strcmp(argv[n],"--tag")==0)
		{
			if(n < argc - 1)
			{
				n++;
				mpt_open_tag(argv[n]);
			}
		}
		else
		if(strcmp(argv[n],"-w")==0 ||
		   strcmp(argv[n],"--word-wrap")==0)
		{
			if(n < argc - 1)
			{
				n++;
				_mp_word_wrap=atoi(argv[n]);
			}
		}
		else
		if(strcmp(argv[n],"--tab-size")==0 ||
		   strcmp(argv[n],"-ts")==0)
		{
			if(n < argc-1)
			{
				n++;
				_mp_tab_size=atoi(argv[n]);
			}
		}
		else
		if(strcmp(argv[n],"-m")==0 ||
		   strcmp(argv[n],"--mode")==0)
		{
			if(n < argc-1)
			{
				n++;
				if(! mps_set_override_mode(argv[n]))
					return(-2);
			}
		}
		else
		{
			mpi_open(argv[n],0);

			/* move to line, if it's the first open file */
			if(_mp_active != NULL && _mpi_move_to_line > 0)
			{
				mp_move_xy(_mp_active, 0, _mpi_move_to_line - 1);
				_mpi_move_to_line=0;
			}
		}
	}

	return(0);
}


/**
 * mpi_startup - Startup interface function.
 *
 * Starts up the interface.
 */
void mpi_startup(void)
{
	int n;
	char tmp[1024];

	/* sets the language */
	mpl_set_language(_mpi_lang);

	mp_move_bof(_menu_info);

	while(mp_get_str(_menu_info,tmp,sizeof(tmp),'\n'))
	{
		char * funcname;

		if(tmp[0]=='/')
			mpv_add_menu(&tmp[1]);
		else
		{
			funcname=(char *)malloc(strlen(tmp)+1);
			strcpy(funcname,tmp);
			mpv_add_menu_item(funcname);
		}
	}

	mp_delete_sys_txt(_menu_info);

	/* creates readline history's texts */
	for(n=0;n < MPR_LAST + 1;n++)
		_mpi_history[n]=mp_create_sys_txt("<history>");
}


/**
 * mpi_shutdown - Shutdown interface function.
 *
 * Shuts down the interface.
 */
void mpi_shutdown(void)
{
	/* closes all open texts */
	while(_mp_active)
	{
		if(_mp_active->mod)
		{
			mpi_draw_all(_mp_active);
			mpv_title(NULL);
			mpv_filetabs();
		}

		mpi_close(_mp_active);
	}
}
