/*

    mp_func_i.c

    Functions definitions

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
#include <ctype.h>

#include "mp_core.h"
#include "mp_video.h"
#include "mp_func.h"
#include "mp_iface.h"
#include "mp_synhi.h"
#include "mp_lang.h"
#include "mp_conf.h"
#include "mp_wordp.h"

struct _mpf_functions
{
	struct _mpf_functions *left,*right;
	char * funcname;
	int (* func)();
	char * desc;
	int builtin;
	void *data;
};

int mpf_mark();
int mpf_key_help();

/* mpf_function definitions */

int mpf_move_up() /* DESC: Line up */
{ return(mp_move_up(_mp_active)); }
int mpf_move_down() /* DESC: Line down */
{ return(mp_move_down(_mp_active)); }
int mpf_move_left() /* DESC: Character left */
{ return(mp_move_left(_mp_active)); }
int mpf_move_right() /* DESC: Character right */
{ return(mp_move_right(_mp_active)); }
int mpf_move_word_left() /* DESC: Word left */
{ return(mp_move_word_left(_mp_active)); }
int mpf_move_word_right() /* DESC: Word right */
{ return(mp_move_word_right(_mp_active)); }
int mpf_move_eol() /* DESC: End of line */
{ return(mp_move_eol(_mp_active)); }
int mpf_move_bol() /* DESC: Beginning of line */
{ return(mp_move_bol(_mp_active)); }
int mpf_move_eof() /* DESC: End of document */
{ return(mp_move_eof(_mp_active)); }
int mpf_move_bof() /* DESC: Beginning of document */
{ return(mp_move_bof(_mp_active)); }
int mpf_move_wheel_up() /* DESC: Page up */
{ return(mpi_move_wheel_up(_mp_active)); }
int mpf_move_wheel_down() /* DESC: Page down */
{ return(mpi_move_wheel_down(_mp_active)); }
int mpf_move_page_up() /* DESC: Page up */
{ return(mpi_move_page_up(_mp_active)); }
int mpf_move_page_down() /* DESC: Page down */
{ return(mpi_move_page_down(_mp_active)); }
int mpf_goto() /* DESC: Go to line... */
{ return(mpi_goto(_mp_active)); }

int mpf_insert_line() /* DESC: Insert line */
{ return(mp_insert_line(_mp_active)); }
int mpf_insert_tab() /* DESC: Insert tab */
{ return(mp_insert_tab(_mp_active)); }

int mpf_delete() /* DESC: Delete char over cursor */
{
	if(mp_marked(_mp_active))
		return(mp_delete_mark(_mp_active));
	return(mp_delete_char(_mp_active));
}
int mpf_delete_left() /* DESC: Delete char to the left of cursor */
{
	if(mp_move_left(_mp_active))
		return(mp_delete_char(_mp_active));
	return(0);
}
int mpf_delete_line() /* DESC: Delete line */
{ return(mp_delete_line(_mp_active)); }
int mpf_delete_word_begin() /* DESC: Delete to the beginning of word */
{
	mpf_mark();
	mpf_move_word_left();
	mpf_move_word_right();
	mpf_mark();
	return(mpf_delete());
}
int mpf_delete_word_end() /* DESC: Delete to the end of word */
{
	mpf_mark();
	mpf_move_word_right();
	mpf_mark();
	return(mpf_delete());
}
int mpf_delete_word() /* DESC: Delete whole word */
{
	mpf_move_word_left();
	mpf_move_word_right();
	mpf_mark();
	mpf_move_word_right();
	mpf_mark();
	return(mpf_delete());
}

int mpf_insert_line_below() /* DESC: Insert new line below cursor */
{
	mpf_move_eol();
	return(mpf_insert_line());
}
int mpf_insert_line_above() /* DESC: Insert new line above cursor */
{
	mpf_move_up();
	mpf_move_eol();
	
	return(mpf_insert_line());
}

int mpf_join_line_above() /* DESC: Join current line to one above */
{
	char ch;
	
	mpf_move_bol();
	mpf_delete_left();
	mp_insert_char(_mp_active, ' ');

	ch = mp_get_char(_mp_active);
	while (ch == ' ' || ch == '\t') {
		mpf_delete_left();
		ch = mp_get_char(_mp_active);
	}

	return(1);
}
int mpf_join_line_below() /* DESC: Join current line to one below */
{
	char ch;

	mpf_move_eol();
	mpf_delete();
	mp_insert_char(_mp_active, ' ');

	ch = mp_get_char(_mp_active);
	while (ch == ' ' || ch == '\t') {
		mpf_delete();
		ch = mp_get_char(_mp_active);
	}

	return(1);
}

/* TODO: check whitespace before cursor as well */
int mpf_delete_whitespace() /* DESC: Delete all whitespace after cursor */
{
	char ch;

	mp_insert_char(_mp_active, ' ');

	ch = mp_get_char(_mp_active);
	while (ch == ' ' || ch == '\n') {
		mpf_delete_left();
		ch = mp_get_char(_mp_active);
	}

	return(1);
}

int flip_letter(int ch) {
	if(islower(ch))
		ch=toupper(ch);
	else
	if(isupper(ch))
		ch=tolower(ch);

	return ch;
}

int mpf_flip_letter_case() /* DESC: Flip letter case if A-Z or a-z */
{
	int ch = (int) mp_peek_char(_mp_active);
	int nch;

	nch = flip_letter(ch);
	if (nch != ch) {
		mpf_delete();
		mp_insert_char(_mp_active, (char) nch);
		mpf_move_left();

		return(1);
	}

	return(0);
}
int mpf_flip_word_case(void) /* DESC: Flip word case */
{
	int ch, nch, didChange = 0;
	
	mpf_move_word_left();
	mpf_move_word_right();

	while (1) {
		ch = (int)mp_get_char(_mp_active);
		if (ch == '\'' || ch == '"') {
			continue;
		}
		
		nch = flip_letter(ch);
		
		if (nch == ch) {
			break;
		} else {
			didChange = 1;
			
			mpf_delete_left();
			mp_insert_char(_mp_active, (char)nch);
		}
	}

	if (didChange == 1) {
		mpf_move_word_left();
		mpf_move_word_left();
		mpf_move_word_right();

		return(1);
	} else {
		mpf_move_left();
	}
	
	return(0);
}


int mpf_mark() /* DESC: Mark beginning/end of block */
{ mp_mark(_mp_active); return(1); }
int mpf_unmark() /* DESC: Unmark block */
{ mp_unmark(_mp_active); return(1); }
int mpf_paste() /* DESC: Paste block */
{
	int ret;
	if(! mpv_system_to_clipboard())
		return(0);

	MP_SAVE_STATE();
	ret = mp_paste_mark(_mp_active);
	MP_RESTORE_STATE();
	return ret;
}

int mpf_copy() /* DESC: Copy block */
{
	int ret=mp_copy_mark(_mp_active);
	mp_unmark(_mp_active);

	mpv_clipboard_to_system();
	return(ret);
}
int mpf_cut() /* DESC: Cut block */
{
	int ret=0;

	if(mp_copy_mark(_mp_active))
	{
		mpv_clipboard_to_system();
		ret=mp_delete_mark(_mp_active);
		mp_unmark(_mp_active);
	}

	return(ret);
}


int mpf_seek() /* DESC: Search text... */
{ return(mpi_seek(_mp_active)); }
int mpf_seek_next() /* DESC: Search next */
{ return(mpi_seek_next(_mp_active)); }
int mpf_mark_match() /* DESC: Select last succesful search */
{ mp_mark_match(_mp_active); return(2); }
int mpf_replace() /* DESC: Replace... */
{ return(mpi_replace(_mp_active)); }
int mpf_replace_all() /* DESC: Replace in all... */
{ return(mpi_replace_all()); }
int mpf_grep() /* DESC: Grep (find inside) files... */
{ return(mpi_grep()); }

int mpf_next() /* DESC: Next */
{ mp_next_txt(); return(2); }
int mpf_prev() /* DESC: Previous */
{ mp_prev_txt(); return(2); }

int mpf_new() /* DESC: New */
{ return(mpi_new()); }
int mpf_open(char **args) /* DESC: Open... */
{
  mpi_open(args ? args[0] : NULL, 0);
  return(2);
}
int mpf_reopen(char **args) /* DESC: Reopen... */
{
  mpi_open(args ? args[0] : NULL, 1);
  return(2);
}
int mpf_save_as() /* DESC: Save as... */
{ return(mpi_save_as(_mp_active)); }
int mpf_save() /* DESC: Save... */
{ return(mpi_save(_mp_active)); }
int mpf_sync() /* DESC: Save modified texts */
{ mpi_sync(); return(2); }
int mpf_close() /* DESC: Close */
{ return(mpi_close(_mp_active)); }

int mpf_open_under_cursor() /* DESC: Open file under cursor */
{
	char tmp[1024];

	mp_get_word(_mp_active,tmp,sizeof(tmp));
	mpi_open(tmp,0);
	return(2);
}


int mpf_zoom_in() /* DESC: Increment font size */
{ mpv_zoom(1); return(2); }
int mpf_zoom_out() /* DESC: Decrement font size */
{ mpv_zoom(-1); return(2); }


int mpf_toggle_insert() /* DESC: Toggle insert/overwrite */
{ _mpi_insert=!_mpi_insert; return(1); }
int mpf_toggle_case() /* DESC: Case sensitive */
{ _mp_case_cmp ^= 1; return(0); }
int mpf_toggle_save_tabs() /* DESC: Save tabs */
{ _mp_save_tabs ^= 1; return(0); }
int mpf_toggle_cr_lf() /* DESC: Save LF as CR/LF */
{ _mp_cr_lf ^= 1; return(0); }
int mpf_toggle_auto_indent() /* DESC: Automatic indentation */
{ _mp_auto_indent ^= 1; return(0); }
int mpf_toggle_column_80() /* DESC: Mark column #80 */
{ _mpi_mark_column_80 ^= 1; return(1); }
int mpf_toggle_regex() /* DESC: Use regular expressions */
{ _mp_regex ^= 1; return(0); }
int mpf_toggle_break_hardlinks() /* DESC: Break hardlinks on write */
{ _mpi_break_hardlinks ^= 1; return(0); }
int mpf_toggle_spellcheck() /* DESC: Mark spelling errors */
{ mpw_spellcheck ^= 1; return(1); }

int mpf_help() /* DESC: Help for word under cursor */
{ return(mpi_help(_mp_active)); }
int mpf_exec_command(char **args) /* DESC: Run system command... */
{
  return mpi_exec(_mp_active,args ? args[0] : NULL);
}
int mpf_document_list() /* DESC: Document list */
{ mpi_current_list(); return(2); }
int mpf_find_tag() /* DESC: Search tag... */
{ return(mpi_find_tag(_mp_active)); }
int mpf_insert_template() /* DESC: Insert template... */
{ return(mpi_insert_template()); }
int mpf_completion() /* DESC: Complete tag... */
{ return(mpi_completion(_mp_active)); }


int mpf_edit_templates_file() /* DESC: Edit templates file */
{
	FILE * f;

	mp_create_txt(_mpi_template_file);

	if((f=mpv_fopen(_mpi_template_file,"r")) != NULL)
	{
		mp_load_file(_mp_active, f);
		fclose(f);

		mps_auto_synhi(_mp_active);
	}
	else
		mp_put_str(_mp_active,_("\
%%Empty template file\n\
\n\
This template file is empty. To create templates, write a name for\n\
each one (marked by two % characteres together in the beginning of\n\
the line) and a text body, delimited by the next template name\n\
or the end of file. By selecting a template from the list (popped up\n\
by Ctrl-U), it will be inserted into the current text.\n"),1);

	_mp_active->mod=0;

	return(2);
}

int mpf_edit_config_file() /* DESC: Edit configuration file */
{
	FILE * f;

	mp_create_txt(_mpc_config_file);

	if((f=mpv_fopen(_mpc_config_file, "r")) != NULL)
	{
		mp_load_file(_mp_active, f);
		fclose(f);

		mps_auto_synhi(_mp_active);
	}
	else
		mp_put_str(_mp_active,_("\
#\n\
# Minimum Profit Config File\n\
#\n\
\n"),1);

	_mp_active->mod=0;

	return(2);
}


int mpf_set_word_wrap() /* DESC: Word wrap... */
{ return(mpi_set_word_wrap()); }
int mpf_set_tab_size() /* DESC: Tab size... */
{ return(mpi_set_tab_size()); }

int mpf_play_macro() /* DESC: Play macro */
{ mpi_play_macro(); return(2); }
int mpf_record_macro() /* DESC: Record macro */
{ mpi_record_macro(); return(2); }

int mpf_menu() /* DESC: Menu */
{ return(mpv_menu()); }

int mpf_about() /* DESC: About... */
{ mpv_about(); return(2); }
int mpf_exit() /* DESC: Exit */
{ _mpi_exit_requested=1; return(2); }


int mpf_mouse_position() /* DESC: Position cursor with mouse */
{ return(2); }

int mpf_exec_function() /* DESC: Execute editor function... */
{ return(mpi_exec_function()); }

int mpf_show_clipboard() /* DESC: Show clipboard */
{ mp_show_sys_txt(_mp_clipboard); return(2); }
int mpf_show_log() /* DESC: Show log */
{ mp_show_sys_txt(_mp_log); return(2); }

int mpf_set_password() /* DESC: Password protect... */
{
	char * pwd1;
	char * pwd2;

	/* ask twice for a password */
	if((pwd1=mpv_readline(MPR_PASSWORD, _("Password:"), NULL)) != NULL &&
		pwd1[0] != '\0')
	{
		pwd1=strdup(pwd1);

		/* ask again */
		if((pwd2=mpv_readline(MPR_PASSWORD,
			_("Password (again):"), NULL)) != NULL)
		{
			/* compare and set if both are the same */
			if(strcmp(pwd1, pwd2) == 0)
			{
				mp_set_password(_mp_active, pwd2);

				mp_log("Password set.\n");
			}
			else
				mpv_alert(_("Error: Passwords mismatch."), NULL);
		}

		free(pwd1);
	}

	return(0);
}

int mpf_suspend() /* DESC: Suspend application */
{
	mpv_suspend();
	return 0;
}

int mpf_reformat_paragraph() /* DESC: Reformat paragraph with word wrapping */
{
	mp_reformat_paragraph(_mp_active);
	return(1);
}

int mpf_sort() /* DESC: Sort lines */
{
  mp_sort(_mp_active);
  return 1;
}

int mpf_jump_matching_bracket() /* DESC: Jump to matching bracket */
{
	mp_match_bracket(_mp_active, -1);

	if(_mp_active->brx != -1 && _mp_active->bry != -1)
		mp_move_xy(_mp_active, _mp_active->brx, _mp_active->bry);
	return(1);
}


static int _mpf_move_next_prev(int s)
{
	int c;
	mp_txt * t;

	t=mp_get_tmp_txt(_mp_active);
	c=mp_peek_char(t);

	for(;;)
	{
		if(s < 0)
		{
			if(!mp_move_left(t))
				break;
		}
		else
		{
			if(!mp_move_right(t))
				break;
		}

		if(mp_peek_char(t) == c)
		{
			mp_move_xy(_mp_active, t->x, t->y);
			break;
		}
	}

	mp_end_tmp_txt();
	return(1);
}


int mpf_move_next() /* DESC: Move to next instance of current char */
{
	return(_mpf_move_next_prev(1));
}


int mpf_move_prev() /* DESC: Move to previous instance of current char */
{
	return(_mpf_move_next_prev(-1));
}


/* the functions */

#include "mp_func_i.h"

/* the keys */

struct _mpf_keys
{
	char * keyname;
	struct _mpf_functions *mpf_function;
};

struct _mpf_keys mpf_keys[]=
{
	{ "cursor-up",		_node_mpf_move_up },
	{ "cursor-down",	_node_mpf_move_down },
	{ "cursor-left",	_node_mpf_move_left },
	{ "cursor-right",	_node_mpf_move_right },
	{ "page-up",		_node_mpf_move_page_up },
	{ "page-down",		_node_mpf_move_page_down },
	{ "home",		_node_mpf_move_bol },
	{ "end",		_node_mpf_move_eol },
	{ "ctrl-cursor-up",	NULL },
	{ "ctrl-cursor-down",	NULL },
	{ "ctrl-cursor-left",	_node_mpf_move_word_left },
	{ "ctrl-cursor-right",	_node_mpf_move_word_right },
	{ "ctrl-page-up",	_node_mpf_move_prev },
	{ "ctrl-page-down",	_node_mpf_move_next },
	{ "ctrl-home",		_node_mpf_move_bof },
	{ "ctrl-end",		_node_mpf_move_eof },
	{ "insert",		_node_mpf_toggle_insert },
	{ "delete",		_node_mpf_delete },
	{ "backspace",		_node_mpf_delete_left },
	{ "escape",		NULL },
	{ "enter",		_node_mpf_insert_line },
	{ "tab",		_node_mpf_insert_tab },
	{ "kp-minus",		NULL },
	{ "kp-plus",		NULL },
	{ "kp-multiply",	NULL },
	{ "kp-divide",		NULL },
	{ "ctrl-kp-minus",	_node_mpf_zoom_out },
	{ "ctrl-kp-plus",	_node_mpf_zoom_in },
	{ "ctrl-kp-multiply",	NULL },
	{ "ctrl-kp-divide",	NULL },
	{ "f1", 		_node_mpf_help },
	{ "f2", 		_node_mpf_save },
	{ "f4", 		_node_mpf_close },
	{ "f5", 		_node_mpf_new },
	{ "f6", 		_node_mpf_next },
	{ "f7", 		_node_mpf_play_macro },
	{ "f8", 		_node_mpf_unmark },
	{ "f9", 		_node_mpf_mark },
	{ "f10",		_node_mpf_record_macro },
	{ "f11",		_node_mpf_zoom_out },
	{ "f12",		_node_mpf_zoom_in },
	{ "ctrl-f1",		NULL },
	{ "ctrl-f2",		NULL },
	{ "ctrl-f3",		NULL },
	{ "ctrl-f4",		NULL },
	{ "ctrl-f5",		NULL },
	{ "ctrl-f6",		NULL },
	{ "ctrl-f7",		NULL },
	{ "ctrl-f8",		NULL },
	{ "ctrl-f9",		NULL },
	{ "ctrl-f10",		_node_mpf_record_macro },
	{ "ctrl-f11",		NULL },
	{ "ctrl-f12",		NULL },
	{ "ctrl-enter", 	_node_mpf_open_under_cursor },
	{ "ctrl-a",		_node_mpf_menu },
	{ "ctrl-c",		_node_mpf_copy },
	{ "ctrl-d",		_node_mpf_copy },
	{ "ctrl-e",		_node_mpf_move_bof },
	{ "ctrl-g",		_node_mpf_goto },
	{ "ctrl-h",		_node_mpf_delete_left },
	{ "ctrl-i",		_node_mpf_insert_tab },
	{ "ctrl-j",		_node_mpf_move_word_left },
	{ "ctrl-k",		_node_mpf_move_word_right },
	{ "ctrl-l",		_node_mpf_seek_next },
	{ "ctrl-m",		_node_mpf_insert_line },
	{ "ctrl-p",		_node_mpf_paste },
	{ "ctrl-r",		_node_mpf_replace },
	{ "ctrl-t",		_node_mpf_cut },
	{ "ctrl-u",		_node_mpf_insert_template },
	{ "ctrl-v",		_node_mpf_paste },
	{ "ctrl-y",		_node_mpf_delete_line },
	{ "ctrl-z",		_node_mpf_suspend },
	{ "ctrl-space", 	_node_mpf_menu },
	{ "mouse-left-button",	_node_mpf_mouse_position },
	{ "mouse-right-button", _node_mpf_mark },
	{ "mouse-middle-button",_node_mpf_paste },
	{ "mouse-wheel-up",	_node_mpf_move_wheel_up },
	{ "mouse-wheel-down",	_node_mpf_move_wheel_down },
	{ "mouse-wheel-left",	NULL },
	{ "mouse-wheel-right",	NULL },
	{ "suspend",		_node_mpf_suspend },

#ifdef CONFOPT_MP5_KEYS

	{ "ctrl-b",		NULL },
	{ "ctrl-f",		_node_mpf_seek },
	{ "f3", 		_node_mpf_seek_next },
	{ "ctrl-x",		_node_mpf_cut },
	{ "ctrl-q",		_node_mpf_exit },
	{ "ctrl-s",		_node_mpf_save },
	{ "ctrl-w",		_node_mpf_close },
	{ "ctrl-n",		_node_mpf_next },
	{ "ctrl-o",		_node_mpf_open },

#else /* CONFOPT_MP5_KEYS */

	{ "ctrl-b",		_node_mpf_seek },
	{ "ctrl-f",		_node_mpf_find_tag },
	{ "f3", 		_node_mpf_open },
	{ "ctrl-x",		_node_mpf_exit },
	{ "ctrl-q",		_node_mpf_cut },
	{ "ctrl-s",		_node_mpf_completion },
	{ "ctrl-w",		_node_mpf_move_eof },
	{ "ctrl-n",		_node_mpf_grep },
	{ "ctrl-o",		_node_mpf_document_list },

#endif /* CONFOPT_MP5_KEYS */

	{ NULL, NULL }
};


/* toggle functions and its value */
struct
{
	char * funcname;
	int * toggle;
} _toggle_functions[]=
{
	{ "toggle-case", &_mp_case_cmp },
	{ "toggle-save-tabs", &_mp_save_tabs },
	{ "toggle-cr-lf", &_mp_cr_lf },
	{ "toggle-auto-indent", &_mp_auto_indent },
	{ "toggle-column-80", &_mpi_mark_column_80 },
	{ "toggle-regex", &_mp_regex },
	{ "toggle-break-hardlinks", &_mpi_break_hardlinks },
	{ "toggle-spellcheck", &mpw_spellcheck },

	{ NULL, NULL }
};

	
/**
 * mpf_get_func_by_keyname - Gets a function by the key bound to it
 * @key_name: name of the key
 *
 * Returns a pointer to the function bound to the @key_name, or
 * NULL if no function is bound to that key name.
 */
struct _mpf_functions * mpf_get_func_by_keyname(char * key_name)
{
	int n;

	if(key_name==NULL) return(NULL);

	for(n=0;mpf_keys[n].keyname!=NULL;n++)
	{
		if(strcmp(key_name, mpf_keys[n].keyname)==0)
			return(mpf_keys[n].mpf_function);
	}

	return(NULL);
}


struct _mpf_functions *_mpf_get_func_by_funcname(char *func_name,
						struct _mpf_functions *r) {
  int cmp;
  if (!r) return NULL;

  cmp = strcmp(r->funcname,func_name);
  if (cmp < 0) 
    return _mpf_get_func_by_funcname(func_name,r->left);
  else if (cmp > 0)
    return _mpf_get_func_by_funcname(func_name,r->right);
  return r;
}


/**
 * mpf_get_desc_by_funcname - Gets a function description by its name
 * @func_name: name of the function
 *
 * Returns a pointer to the @func_name description text,
 * or NULL if it's not found.
 */
char * mpf_get_desc_by_funcname(char * func_name)
{
	struct _mpf_functions *f;

	if(func_name==NULL) return(NULL);

	/* if it's the menu separator or a special string */
	if(strcmp(func_name, "-") == 0 || *func_name == '<')
		return(func_name);

	f = _mpf_get_func_by_funcname(func_name,mpf_functions);
	return f ? f->desc : NULL;
}


/**
 * mpf_get_keyname_by_funcname - Returns the key bound to a function
 * @func_name: name of the function
 *
 * Returns the name of the key bound to the function called
 * @func_name, or NULL of none is.
 */
char * mpf_get_keyname_by_funcname(char * func_name)
{
	struct _mpf_functions *f;
	int n;
	if(func_name==NULL) return(NULL);
	f = _mpf_get_func_by_funcname(func_name,mpf_functions);

	if (!f) return NULL;

	/* search now a key with that function */
	for(n=0;mpf_keys[n].keyname!=NULL;n++)
	{
		if(mpf_keys[n].mpf_function == f)
			return(mpf_keys[n].keyname);
	}

	return(NULL);
}


/**
 * mpf_get_funcname_by_keyname - Returns the function name bound to a key
 * @key_name: name of the key
 *
 * Returns the name of the function bound to the @key_name,
 * or NULL otherwise.
 */
char * mpf_get_funcname_by_keyname(char * key_name)
{
	struct _mpf_functions *func=NULL;
	if((func=mpf_get_func_by_keyname(key_name))==NULL) return NULL;
	return func->funcname;
}



/**
 * mpf_bind_key - Binds a key to a function
 * @key_name: name of the key to be bound
 * @func_name: name of the function
 *
 * Binds a key to a function. @func_name is the name of the
 * function to be assigned, or <none> if the key is to be
 * unbounded. @key_name is the name of the key, or <all>
 * if the function is to be bound to ALL keys (mainly thought
 * to be used in combination with <none> to clean all key
 * bindings). Returns 0 if @key_name
 * or @func_name is not a defined one, or 1 if the key was bound.
 */
int mpf_bind_key(char * key_name, char * func_name)
{
	int n,c;
	struct _mpf_functions *f;

	if(strcmp(func_name,"<none>")==0)
		f=NULL;
	else
	if((f=_mpf_get_func_by_funcname(func_name,mpf_functions))==NULL)
		return(0);

	/* search now a key with that keyname */
	for(n=c=0;mpf_keys[n].keyname!=NULL;n++)
	{
		if(strcmp(key_name, "<all>")==0 ||
			strcmp(mpf_keys[n].keyname, key_name)==0)
		{
			mpf_keys[n].mpf_function=f;
			c++;
		}
	}

	return(c);
}

void _mpf_add_unlinked_functions(mp_txt *txt,struct _mpf_functions *r) {
  int m;

  if (!r) return ;
  _mpf_add_unlinked_functions(txt,r->right);

  if (!mpf_get_keyname_by_funcname(r->funcname)) {
	mp_put_str(txt,"  ",1);
	mp_put_str(txt,r->funcname,1);

	for(m=0;m < 23 - strlen(r->funcname);m++)  mp_put_char(txt,' ',1);

	mp_put_str(txt,_(r->desc),1);

	mp_put_char(txt,'\n',1);
  
  }
  
  _mpf_add_unlinked_functions(txt,r->left);
}


/**
 * mpf_key_help - Shows the key binding help to the user
 *
 * Shows the key binding help to the user.
 */
int mpf_key_help() /* DESC: Help on keys */
{
	int n,m;
	mp_txt * txt;
	char * ptr;

	MP_SAVE_STATE();

	txt=mp_create_txt(_("<help on keys>"));

	for(n=0;(ptr=mpf_keys[n].keyname)!=NULL;n++)
	{
		mp_put_str(txt,"  ",1);
		mp_put_str(txt,ptr,1);

		for(m=0;m < 23 - strlen(ptr);m++)
			mp_put_char(txt,' ',1);

		if((ptr=mpf_get_funcname_by_keyname(ptr))==NULL)
			ptr=_("<none>");

		mp_put_str(txt,ptr,1);

		for(m=0;m < 23 - strlen(ptr);m++)
			mp_put_char(txt,' ',1);

		ptr=_(mpf_get_desc_by_funcname(ptr));
		mp_put_str(txt,ptr,1);

		mp_put_char(txt,'\n',1);
	}

	mp_put_char(txt,'\n',1);
	mp_put_str(txt,_("Unlinked functions"),1);
	mp_put_char(txt,'\n',1);
	mp_put_char(txt,'\n',1);

	_mpf_add_unlinked_functions(txt,mpf_functions);

	mp_move_bof(txt);
	txt->type=MP_TYPE_READ_ONLY;
	txt->mod=0;

	MP_RESTORE_STATE();

	return(2);
}


/**
 * mpf_toggle_function_value - Returns the value of a toggle function
 * @func_name: name of the function
 *
 * Returns a pointer to an integer containing the boolean value
 * of a toggle function, or NULL if @func_name does not exist.
 */
int * mpf_toggle_function_value(char * func_name)
{
	int n;

	for(n=0;_toggle_functions[n].funcname!=NULL;n++)
	{
		if(strcmp(_toggle_functions[n].funcname,func_name)==0)
			return(_toggle_functions[n].toggle);
	}

	return(NULL);
}

void _mpf_add_funcnames(mp_txt *txt,struct _mpf_functions *r) {
  if (!r) return ;

  _mpf_add_funcnames(txt,r->right);

  mp_put_str(txt,r->funcname,1);
  mp_put_char(txt,'\n',1);

  _mpf_add_funcnames(txt,r->left);
}


void mpf_get_funcnames(mp_txt * txt)
{
	/* clears the text */
	mp_empty_txt(txt);
	_mpf_add_funcnames(txt,mpf_functions);
}


/**
 * mpf_call_func_by_funcname - Calls a function by its name
 *
 * @func_name: name of the function
 * @args: additional optional arguments to pass
 *
 * Returns -1 for function not found, or the return value of
 * the function being called with should correspond to:
 * 0 for no change, 1 for redraw, 2 for update tabs and redraw.
 */
int mpf_call_func_by_funcname(char *func_name,char **args)
{
  struct _mpf_functions *f;
  int ret;

  if(func_name==NULL) return -1;
  f = _mpf_get_func_by_funcname(func_name,mpf_functions);
  if (!f) return -1;

  ret = f->func(args,f);
  if (ret == -1) {
    mp_log("Function returned -1: %s\n",func_name);
    ret = 1;
  }
  return ret;
}


#define ARGBLKS		256
#define SM_FINDSPC	1
#define SM_QUOTE	2
#define SM_SKIPSPC	3

/**
 * mpf_makeargs - Splits a string into an argument list
 *
 * @str: string to split
 *
 * Splits a string into an argument list.  It is mindful of respecting
 * double quotes and backslashes.  Returns an array of pointers.
 * The caller is responsible of freeing the returned pointer.
 * Returns NULL on failure.
 */
char **mpf_makeargs(char *str) {
  int argc = 0;
  int i,j,state, bsz = ARGBLKS;
  char **args = malloc(sizeof(char *)*ARGBLKS);
  
  if (!args) return 0;
  args[argc++] = str;

  for (i=j=0,state=SM_FINDSPC ; str[i] ; i++) {
    switch (state) {
    case SM_QUOTE:
    case SM_FINDSPC:
      switch (str[i]) {
      case '"':
	state = state == SM_QUOTE ? SM_FINDSPC : SM_QUOTE;
	break;
      case '\\':
	if (str[++i])
	  str[j++] = str[i];
	else 
	  --i;
	break;
      default:
	if (state == SM_FINDSPC && isspace(str[i])) {
	  str[j++] = 0;
	  state = SM_SKIPSPC;
	} else {
	  str[j++] = str[i];
	}
      }
      break;
    case SM_SKIPSPC:
      if (!isspace(str[i])) {
	if (argc+2 >= bsz) {
	  char **nb = realloc(args,(bsz += ARGBLKS) * sizeof(char *));
	  if (!nb) {
	    free(args);
	    return nb;
	  }
	  args = nb;
	}
	args[argc++] = str+j;
	state = SM_FINDSPC;
	--i;
      }
      break;
    default:
	abort();	/* This should never happen! */	
    }
  }
  str[j++] = 0;
  args[argc++] = 0;

  if (argc < bsz) {
    char **nb = realloc(args,argc * sizeof(char *));
    if (nb) args = nb;
  }

  return args;
}

struct _mpf_functions *_mpf_add_func(struct _mpf_functions *r,
					struct _mpf_functions *n) {
  int cmp;
  if (!r) return n;

  cmp = strcmp(r->funcname,n->funcname);
  if (cmp < 0)
    r->left = _mpf_add_func(r->left,n);
  else if (cmp > 0)
    r->right = _mpf_add_func(r->right,n);
  else
    abort(); /* We failed to add the item! */
  return r;
}

#define MAXBLKSZ	8192	/* Everything should fit in 8K */

static void *_mpf_make_func_list(char **args) {
  void *memblk = malloc(MAXBLKSZ);
  char *mempos;
  char ***ftab;
  char **ptrs;
  int i,j,k,b;

  if (!memblk) return NULL;
  
  /* How many arguments are there... */
  for (i=0, ftab = memblk; args[i] ; i++);
  if ( sizeof(char **)*i >= MAXBLKSZ-sizeof(char **) ) {
    free(memblk);
    return NULL;
  }

  /* Calculate memory requirements... */
  for (b=i=j=0; args[i] ; i++) {
    if (args[i][0] == '>') {
      ftab[i] = NULL;
      j += 3;
      b += strlen(args[i]);
    } else {
      ftab[i] = mpf_makeargs(args[i]);
      if (!ftab[i]) break;
      for (k=0; ftab[i][k] ; k++) {
        ++j;
        b += strlen(ftab[i][k])+1;
      }
      ++j;
    }
  }
  k = sizeof(char **)*(i+2)+sizeof(char *)*(j+2)+b+1;
  if (!args[i] && k < MAXBLKSZ) {
    memblk = realloc(memblk,k);
    if (memblk) {
      /* We know we have enough space... */

      /* Allocate areas in memblk... */
      ptrs = (char **)(ftab+(i+1));
      mempos = (char *)(ptrs+(j+1));

      /* Copy stuff over... */
      for (i=0; args[i] ; i++) {
        if (args[i][0] == '>') {
          ftab[i] = ptrs;
          *(ptrs++) = ">";
          *(ptrs++) = mempos;
          for (j=1; args[i][j] ; *(mempos++) = args[i][j++]);
          *(mempos++) = 0;
        } else {
          char **p = ftab[i];
          ftab[i] = ptrs;
	  for (k=0; p[k] ; k++) {
            *(ptrs++) = mempos;
            for (j=0;p[k][j] ; *(mempos++) = p[k][j++]);
            *(mempos++) = 0;
          }
          free(p);
        }
        *(ptrs++) = NULL;
      }    
      ftab[i] = NULL;
      return memblk;
    } else
      memblk = ftab;
  }
  /* Not enough memory, give up! */
  for (j=0;j<i; j++) if (ftab[i]) free(ftab[i]);
  free(memblk);
  return NULL;
}

static int _mpf_call_user_fn(char **args,struct _mpf_functions *f) {
  char ***seq = (char ***)f->data;
  int ret = 0, nret;

  mp_log("USERFN: %s\n",f->funcname);

  while (*seq) {
    args = *seq;

    if (args[0][0] == '>') {
      mp_log("*    INSERT: %s\n",args[1]);
      mp_put_str(_mp_active,args[1],1);
      nret = 1;
    } else {
      mp_log("*    FUNCTION: %s\n",args[0]);
      nret = mpf_call_func_by_funcname(args[0],args[1] ? args+1 : NULL);
      if (nret == -1) {
        mpv_alert(_("Error executing user defined function"),args[0]);
	break;
      }
    }
    if (nret > ret) ret = nret;
    seq++;
  }
  return ret;    
}

/**
 * mpf_new_user_fn - Create an user defined function
 *
 * @func: function name
 * @desc: function description
 * @args: functions to execute
 *
 * This function creates an user defined functions.
 * User defined functions are sequence of functions that can be defined
 * as function names and later bound to different keys or menu items.
 */
void mpf_new_user_fn(char *func,char *desc,char **args) {
  struct _mpf_functions *f;
  void *data;
  f = _mpf_get_func_by_funcname(func,mpf_functions);
  if (f) {
    /* Function already exists! */
    if (f->builtin) {
      mp_log("Attempting to redefine built-in function %s\n",func);
      return;
    }
    desc = strdup(desc);
    data = _mpf_make_func_list(args);
    if (!desc || !data) {
      if (desc) free(desc);
      if (data) free(data);
      mp_log("Memory errors re-defining cmd: %s\n",func);
      return;
    }
    free(f->desc); f->desc = desc;
    free(f->data); f->data = data;
  } else {
    /* Brand new function... */
    char *fnm = strdup(func);
    f = malloc(sizeof(struct _mpf_functions));
    desc = strdup(desc);
    data = _mpf_make_func_list(args);
    if (!f || !fnm || !desc || !data) {
      if (f) free(f);
      if (fnm) free(fnm);
      if (desc) free(desc);
      if (data) free(data);
      mp_log("Memory errors defining cmd: %s\n",func);
      return;
    }

    f->left = f->right = NULL;
    f->funcname = fnm;
    f->func = _mpf_call_user_fn;
    f->desc = desc;
    f->builtin = 0;
    f->data = data;

    mpf_functions = _mpf_add_func(mpf_functions,f);
  }
}

/**
 * mpf_desc_user_fn - Redefines a user function description
 *
 * @func: function name
 * @desc: command description
 *
 * This function lets you change the description of a user
 * defined function.  Potentially for the purpose of localisation.
 */
void mpf_desc_user_fn(char *func,char *desc)
{
  struct _mpf_functions *f;
  f = _mpf_get_func_by_funcname(func,mpf_functions);
  if (!f) {
    /* Function does not exists! */
    mp_log("Unable to describe unknown function %s\n",func);
    return;
  }
  if (f->builtin) {
    mp_log("Attempting to describe built-in function %s\n",func);
    return;
  }
  desc = strdup(desc);
  if (!desc) {
    mp_log("Memory errors describing function %s\n",func);
    return;
  }
  free(f->desc); f->desc = desc;
}

