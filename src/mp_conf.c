/*

    mp_conf.c

    Configuration file.

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
#include "mp_conf.h"
#include "mp_lang.h"
#include "mp_func.h"
#include "mp_iface.h"
#include "mp_tags.h"
#include "mp_wordp.h"

/*******************
	Data
********************/

#ifndef CONFIG_FILE_NAME
#define CONFIG_FILE_NAME "mprc"
#endif

/* home sweet home */
char _mpc_home[1024]="";

/* the configuration file */
char _mpc_config_file[1024];

/* colors */

#define TEXT_COLOR_DEF		-1
#define TEXT_COLOR_BLACK	0
#define TEXT_COLOR_RED		1
#define TEXT_COLOR_GREEN	2
#define TEXT_COLOR_YELLOW	3
#define TEXT_COLOR_BLUE 	4
#define TEXT_COLOR_MAGENTA	5
#define TEXT_COLOR_CYAN 	6
#define TEXT_COLOR_WHITE	7

struct _mpc_color_desc mpc_color_desc[MP_COLOR_NUM]= {
	{ 0x00000000, 0x00ffffff,
	  TEXT_COLOR_DEF, TEXT_COLOR_DEF,
	  0, 0, 0, 0, "normal" }, /* MP_COLOR_NORMAL */
	{ 0x00ff0000, 0x00ffffff,
	  TEXT_COLOR_RED, TEXT_COLOR_WHITE,
	  0, 0, 1, 0, "selected" }, /* MP_COLOR_SELECTED */
	{ 0x0000aaaa, 0xffffffff,
	  TEXT_COLOR_GREEN, TEXT_COLOR_DEF,
	  1, 0, 0, 0, "comment" }, /* MP_COLOR_COMMENT */
	{ 0x000000ff, 0xffffffff,
	  TEXT_COLOR_BLUE, TEXT_COLOR_DEF,
	  0, 0, 0, 1, "string" }, /* MP_COLOR_STRING */
	{ 0x0000aa00, 0xffffffff,
	  TEXT_COLOR_GREEN, TEXT_COLOR_DEF,
	  0, 0, 0, 1, "token" }, /* MP_COLOR_TOKEN */
	{ 0x00ff6666, 0xffffffff,
	  TEXT_COLOR_RED, TEXT_COLOR_DEF,
	  0, 0, 0, 0, "var" }, /* MP_COLOR_VAR */
	{ 0xffffffff, 0xffffffff,
	  TEXT_COLOR_DEF, TEXT_COLOR_DEF,
	  0, 0, 1, 0, "cursor" }, /* MP_COLOR_CURSOR */
	{ 0x00dddd00, 0xffffffff,
	  TEXT_COLOR_YELLOW, TEXT_COLOR_DEF,
	  0, 0, 0, 1, "caps" }, /* MP_COLOR_CAPS */
	{ 0x008888ff, 0xffffffff,
	  TEXT_COLOR_CYAN, TEXT_COLOR_DEF,
	  0, 1, 0, 0, "local" }, /* MP_COLOR_LOCAL */
	{ 0x00000000, 0x00ffff00,
	  TEXT_COLOR_BLACK, TEXT_COLOR_CYAN,
	  0, 0, 0, 0, "bracket" }, /* MP_COLOR_BRACKET */
	{ 0x00ff8888, 0xffffffff,
	  TEXT_COLOR_RED, TEXT_COLOR_DEF,
	  0, 1, 0, 1, "misspelled" }, /* MP_COLOR_MISSPELLED */

	{ 0, 0,
	  TEXT_COLOR_BLUE, TEXT_COLOR_WHITE,
	  0, 0, 1, 1, "title" }, /* MP_COLOR_TEXT_TITLE */
	{ 0, 0,
	  TEXT_COLOR_BLUE, TEXT_COLOR_WHITE,
	  0, 0, 1, 1, "menu_element" }, /* MP_COLOR_TEXT_MENU_ELEM */
	{ 0, 0,
	  TEXT_COLOR_WHITE, TEXT_COLOR_BLACK,
	  0, 0, 0, 1, "menu_selection" }, /* MP_COLOR_TEXT_MENU_SEL */
	{ 0, 0,
	  TEXT_COLOR_BLUE, TEXT_COLOR_BLUE,
	  0, 0, 1, 1, "frame1" }, /* MP_COLOR_TEXT_FRAME1 */
	{ 0, 0,
	  TEXT_COLOR_BLUE, TEXT_COLOR_BLACK,
	  0, 0, 1, 1, "frame2" }, /* MP_COLOR_TEXT_FRAME2 */
	{ 0, 0,
	  TEXT_COLOR_DEF, TEXT_COLOR_DEF,
	  0, 0, 0, 0, "scrollbar" }, /* MP_COLOR_TEXT_SCROLLBAR */
	{ 0, 0,
	  TEXT_COLOR_BLUE, TEXT_COLOR_WHITE,
	  0, 0, 1, 1, "scrollbar_thumb" } /* MP_COLOR_TEXT_SCR_THUMB */
};

mp_txt * _menu_info=NULL;


/* default menu */
char * _default_menu=
	"/" N_("&File") "\n"\
	"new\nopen\nreopen\nsave\nsave-as\nclose\n-\nset-password\n-\nsync\n-\nexit\n"\
	"/" N_("&Edit") "\n"\
	"cut\ncopy\npaste\ndelete-line\n-\n"\
	"mark\nunmark\n-\n"\
	"edit-templates-file\n"\
	"edit-config-file\n-\nexec-command\nexec-function\n"\
	"/" N_("&Search") "\n"\
	"seek\nseek-next\nreplace\nreplace-all\ntoggle-case\ntoggle-regex\n-\nfind-tag\ncompletion\n-\ngrep\n"\
	"/" N_("&Go to") "\n"\
	"next\nmove-bof\nmove-eof\nmove-bol\nmove-eol\ngoto\n"\
	"move-word-right\nmove-word-left\n-\ndocument-list\n"\
	"/" N_("&Options") "\n"\
	"toggle-save-tabs\ntoggle-cr-lf\ntoggle-auto-indent\n"\
	"toggle-column-80\ntoggle-break-hardlinks\ntoggle-spellcheck\n-\n"\
	"record-macro\nplay-macro\n-\n"\
	"set-tab-size\nset-word-wrap\n-\n"\
	"key-help\n-\nabout\n"\
	"";

/*******************
	Code
********************/


static void _mpc_set_variable(char * var, char * value)
{
	if(strcmp(var,"tab_size")==0)
		_mp_tab_size=atoi(value);
	else
	if(strcmp(var,"word_wrap")==0)
		_mp_word_wrap=atoi(value);
	else
	if(strcmp(var,"wheel_scroll_rows")==0)
		_mp_wheel_scroll_rows=atoi(value);
	else
	if(strcmp(var,"case_sensitive_search")==0)
		_mp_case_cmp=atoi(value);
	else
	if(strcmp(var,"auto_indent")==0)
		_mp_auto_indent=atoi(value);
	else
	if(strcmp(var,"save_tabs")==0)
	       _mp_save_tabs=atoi(value);
	else
	if(strcmp(var,"col_80")==0)
		_mpi_mark_column_80=atoi(value);
	else
	if(strcmp(var,"monochrome")==0)
		mpi_monochrome=atoi(value);
	else
	if(strcmp(var,"cr_lf")==0)
		_mp_cr_lf=atoi(value);
	else
	if(strcmp(var,"preread_lines")==0)
		_mpi_preread_lines=atoi(value);
	else
	if(strcmp(var,"use_regex")==0)
		_mp_regex=atoi(value);
	else
	if(strcmp(var,"template_file")==0)
		strncpy(_mpi_template_file,value,sizeof(_mpi_template_file));
	else
	if(strcmp(var,"lang")==0)
	{
		/* store only if user has not set
		   another one via command line arguments */
		if(_mpi_lang[0] != '\0')
			strncpy(_mpi_lang, value, sizeof(_mpi_lang));
	}
	else
	if(strcmp(var, "ctags_cmd")==0)
		strncpy(_mpt_ctags_cmd,value,sizeof(_mpt_ctags_cmd));
	else
	if(strcmp(var, "status_format")==0)
		strncpy(_mpi_status_line_f,value,sizeof(_mpi_status_line_f));
	else
	if(strcmp(var, "strftime_format")==0)
		strncpy(_mpi_strftime_f,value,sizeof(_mpi_strftime_f));
	else
	if(strcmp(var, "break_hardlinks") == 0)
		_mpi_break_hardlinks=atoi(value);
	else
	if(strcmp(var, "spellcheck") == 0)
		mpw_spellcheck=atoi(value);
	else
	if(strcmp(var, "ispell_cmd") == 0)
		strncpy(mpw_ispell_command, value, sizeof(mpw_ispell_command));
	else
	if(strcmp(var, "move_seek_to_line") == 0)
		_mpi_seek_to_line = atoi(value);
	else
		mpv_set_variable(var, value);
}

/*
 * Define colors...
 */
static void _mpc_def_color(char *colorname,char *ink,char *paper,
				char **opts,int text) {
	/* colordef string is:
	   {colorname} {ink_color} {paper_color} [{options}]
	   where options can be
	   italic, underline, reverse, bright

	   - Bright is not used in gui_color, nor italic
	     in text_color
	   - {ink_color} and {paper_color} are RGB
	     definitions in gui_color, and color names
	     in text_color
	*/
	int c,n;
	int in,pn;

	
	char * text_color_names[]={ "default", "black", "red", "green",
		"yellow", "blue", "magenta", "cyan","white", NULL };
	char * text_color_names_equiv[]= { "ffffff", "000000", "ff0000",
		"00ff00", "ffff00", "0000ff", "ff00ff", "00ffff", "ffffff",
		NULL };

	if(_mpv_text != text) return;
	/* Find color name */
	for (c=0;c < MP_COLOR_NUM;c++) {
	  if (strcmp(colorname,mpc_color_desc[c].colorname)==0) break;
	}
	/* bad color name */
	if(c==MP_COLOR_NUM) return;


	if(text) {
	  in=pn=-1;

	  /* find the ink and paper offset */
	  for(n=0;text_color_names[n]!=NULL;n++) {
	    if(strcmp(ink,text_color_names[n])==0) in=n;
	    if(strcmp(paper,text_color_names[n])==0) pn=n;
	  }

	  /* bad ink or paper name */
	  if(in==-1 || pn==-1) return;

	  /* store */
	  mpc_color_desc[c].ink_text=in-1;
	  mpc_color_desc[c].paper_text=pn-1;
	} else {
	  /* try to find if user set a color name, skipping 'default' */
	  for(n=1;text_color_names[n]!=NULL;n++) {
	    if(strcmp(ink,text_color_names[n])==0)
	      ink=text_color_names_equiv[n];
	    if(strcmp(paper,text_color_names[n])==0)
	      paper=text_color_names_equiv[n];
	  }
	  /* ink and paper are RGB values */
	  if(strcmp(ink,"default")==0)
	    mpc_color_desc[c].ink_rgb=0xffffffff;
	  else
	    sscanf(ink, "%x", &mpc_color_desc[c].ink_rgb);

	  if(strcmp(paper,"default")==0)
	    mpc_color_desc[c].paper_rgb=0xffffffff;
	  else
	    sscanf(paper, "%x", &mpc_color_desc[c].paper_rgb);
	}

	mpc_color_desc[c].italic=mpc_color_desc[c].underline=
	mpc_color_desc[c].reverse=mpc_color_desc[c].bright=0;

	/* process the options */
	while (*opts) {
	  if (strcmp(*opts,"italic")==0) {
	    mpc_color_desc[c].italic=1;
	  } else if (strcmp(*opts,"underline")==0) {
	    mpc_color_desc[c].underline = 1;
	  } else if (strcmp(*opts,"reverse")==0) {
	    mpc_color_desc[c].reverse = 1;
	  } else if (strcmp(*opts,"bright")==0) {
	    mpc_color_desc[c].bright = 1;
	  }
	  opts++;
	}
}


static void _mpc_add_menu(char * data, int menu_item)
{
	if(!menu_item)
		mp_put_char(_menu_info,'/',1);
	else
	{
		/* add only existing functions */
		if(mpf_get_desc_by_funcname(data) == NULL)
		{
			mp_log("ERROR: bad function '%s'\n", data);
			return;
		}
	}

	mp_put_str(_menu_info,data,1);
	mp_put_char(_menu_info,'\n',1);
}

static void _mpc_clear_menu(void) {
  if(_menu_info!=NULL) {
    mp_empty_txt(_menu_info);
  }
}

static void _mpc_bind_menu(char *fn,char **keys) {
  _mpc_add_menu(fn,1);
  while (*keys) mpf_bind_key(*(keys++),fn);  
}

/*
 *
 * Read lines and tokenise...
 * 
 */
#define SM_SKIPSPC ' '
#define SM_FINDSPC 'a'
#define SM_QUOTE '"'

int ch_esc(int ch) {
  switch (ch) {
    case 'a': ch = '\a'; break;
    case 'b': ch = '\b'; break;
    case 'f': ch = '\f'; break;
    case 'r': ch = '\r'; break;
    case 'n': ch = '\n'; break;
    case 'v': ch = '\v'; break;
  }
  return ch;
}

int fgetargs(char *buf,int bufsz,char **args,int argsz,FILE *f) {
  int ch;
  int argc = 0;
  int state = SM_SKIPSPC;
  int iptr = 0;


  if (feof(f)) return -1;

  while ((ch = getc(f)) != EOF && iptr < bufsz-1 && argc < argsz-2) {
    if (state != SM_QUOTE &&  ch == '#') {
      while ((ch = getc(f)) != '\n' && ch != EOF);
      break;
    }
    if (ch == '\n') break;
    if (ch == '\r') continue;

    if (state == SM_SKIPSPC) {
      if (isspace(ch)) continue;
      if (ch == '\\') {
        while ((ch = getc(f)) == '\r');
        if (ch == '\n'  || ch == EOF) continue;
	ungetc(ch,f);
	ch = '\\';
      }
      args[argc++] = buf+iptr;
      state = SM_FINDSPC;
    }
    if (ch == '\\') {
      while ((ch = getc(f)) == '\r');
      if (ch == '\n'  || ch == EOF) continue;
      buf[iptr++] = ch_esc(ch);
      continue;
    }
    if (ch == '"') {
      state = state == SM_QUOTE ? SM_FINDSPC : SM_QUOTE;
    } else {
      if (state == SM_FINDSPC && isspace(ch)) {
	buf[iptr++] = 0;
	state = SM_SKIPSPC;
      } else {
	buf[iptr++] = ch;
      }
    }
  }
  buf[iptr] = 0;
  args[argc] = NULL;
  return argc;
}

/*
 * Check for the truth of an expression
 *
 * Currently only checks for video interface type...
 */
static int _mpc_bool(char *expr) {
  if (_mpi_lang[0] != 0 && !strncmp("lang(",expr,5)) {
    int l = strlen(_mpi_lang);
    if (!strncmp(expr+5,_mpi_lang,l) && expr[l+5] == ')') return 1;
  } 
  return !strcmp(expr,_mpv_interface);
}

/**
 * mpc_read_config - Reads and parses the configuration file
 *
 * Reads and parses the configuration file.
 */
static void _mpc_read_config(char * file, int level)
{
	char line[4096];
	FILE * f;
	char * ptr;
	char *argb[128], **args;
	int argc, mode = 1;

	if(level >= 16)
		return;

	if((f=fopen(file,"r"))==NULL)
		return;

	mp_log("Config file: '%s'\n", file);

	while ((argc = fgetargs(line,sizeof line,
				argb,sizeof(argb)/sizeof(argb[0]),f)) != -1) {
	  if (!argc) continue;
	  args = argb;

	  if ((!strcmp(args[0],"if") || !strcmp(args[0],"unless")) && argc>=2){
	    int bool = _mpc_bool(args[1]);
	    if (args[0][0] == 'u') bool = !bool;

	    if (argc > 2) {
	      if (mode && bool) {
	        args += 2;
	        argc -= 2;
	      } else
	        continue;
	    } else {
	      mode = bool;
	      continue;
	    }
	  } else if (!strcmp(args[0],"else")) {
	    mode = !mode;
	    continue;
	  } else if (!strcmp(args[0],"endif")) {
	    mode = 1;
	    continue;
	  }

	  if (!mode) continue;

	  if (!strcmp(args[0],"bind") && argc >= 3) {
	    mpf_bind_key(args[1],args[2]);
	  } else if (!strcmp(args[0],"text_color") && argc > 3) {
	    _mpc_def_color(args[1],args[2],args[3],args+4,1);
	  } else if (!strcmp(args[0],"gui_color") && argc > 3) {
	    _mpc_def_color(args[1],args[2],args[3],args+4,0);
	  } else if (!strcmp(args[0],"menu") && argc >= 2) {
	    _mpc_add_menu(args[1],0);
	  } else if (!strcmp(args[0],"menu_item") && argc >= 2) {
	    _mpc_add_menu(args[1],1);
	  } else if (!strcmp(args[0],"menu_bind") && argc >= 2) {
	    _mpc_bind_menu(args[1],args+2);
	  } else if (!strcmp(args[0],"menu_reset")) {
	    _mpc_clear_menu();
	  } else if (!strcmp(args[0],"source") && argc >= 2) {
	    _mpc_read_config(args[1],level+1);
	  } else if (!strcmp(args[0],"user-fn") && argc >= 4) {
	    mpf_new_user_fn(args[1],args[2],args+3);
	  } else if (!strcmp(args[0],"desc-user-fn") && argc >= 3) {
	    mpf_desc_user_fn(args[1],args[2]);
	  } else if (((ptr=strchr(args[0],':'))!=NULL || 
			(ptr=strchr(args[0],'='))!=NULL) && ptr[1] == '\0' &&
			argc >= 2) {
	    *ptr = 0;
	    if (argc > 2) {
		int i;
		for (i=2; args[i] ; i++) *(args[i]-1)=' ';
	    }
	    _mpc_set_variable(args[0],args[1]);
	  }
	}
	fclose(f);
}


void mpc_startup(void)
{
	char * home;
	int n;

	/* create and fill the default menu */
	_menu_info=mp_create_sys_txt("<menu_tmp_data>");

	MP_SAVE_STATE();
	mp_put_str(_menu_info, _default_menu, 1);
	MP_RESTORE_STATE();

	/* reads first a global config file */
	snprintf(_mpc_config_file, sizeof(_mpc_config_file),
		"/etc/%s", CONFIG_FILE_NAME);

	_mpc_read_config(_mpc_config_file, 0);

	/* take home from the environment variable,
	   unless externally defined */
	if(_mpc_home[0]=='\0' && (home=getenv("HOME"))!=NULL)
		strncpy(_mpc_home,home,sizeof(_mpc_home));

	n=strlen(_mpc_home)-1;
	if(_mpc_home[n]=='/' || _mpc_home[n]=='\\')
		_mpc_home[n]='\0';

	snprintf(_mpc_config_file, sizeof(_mpc_config_file),
		"%s/.%s", _mpc_home, CONFIG_FILE_NAME);

	_mpc_read_config(_mpc_config_file, 0);

	/* fix the template file name if it has a leading ~ */
	if(_mpi_template_file[0]=='~')
	{
		char line[1024];

		strncpy(line,&_mpi_template_file[1],sizeof(line));
		snprintf(_mpi_template_file,sizeof(_mpi_template_file),
			"%s%s",_mpc_home,line);
	}
}
