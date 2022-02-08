/*

    mp_video.h

    Video driver definitions.

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

extern int _mpv_x_size;
extern int _mpv_y_size;

extern int _mpv_argc;
extern char ** _mpv_argv;

extern int _mpv_text;
extern char _mpv_interface[64];

/* mpv_readline types */

#define MPR_OPEN		0
#define MPR_SAVE		1
#define MPR_SEEK		2
#define MPR_GOTO		3
#define MPR_REPLACETHIS 	4
#define MPR_REPLACEWITH 	5
#define MPR_TAG 		6
#define MPR_EXEC		7
#define MPR_WORDWRAP		8
#define MPR_TABSIZE		9
#define MPR_EXECFUNCTION	10
#define MPR_GREPFILES		11
#define MPR_PASSWORD		12
#define MPR_LAST		MPR_PASSWORD

struct _mpv_driver
{
	char * name;
	int is_text;
	int (* _strcasecmp)(char *, char *);
	FILE * (* _fopen)(char *, char *);
	mp_txt * (* _glob)(char *);
	void (* _goto)(int, int);
	void (* _char)(int, int);
	void (* _str)(char *, int);
	void (* _cursor)(int, int);
	void (* _refresh)(void);
	void (* _title)(char *);
	void (* _status_line)(char *);
	void (* _add_menu)(char *);
	void (* _add_menu_item)(char *);
	void (* _check_menu)(char *, int);
	int (* _menu)(void);
	void (* _alert)(char *, char *);
	int (* _confirm)(char *);
	int (* _list)(char *, mp_txt *, int);
	char * (* _readline)(int, char *, char *);
	int (* _sys_to_clip)(void);
	void (* _clip_to_sys)(void);
	void (* _about)(void);
	int (* _help)(char *, int);
	int (* _zoom)(int);
	void (* _scrollbar)(int, int, int);
	void (* _filetabs)(int);
	void (* _set_var)(char *, char *);
	int (* _args)(int, char **);
	void (* _usage)(void);
	void (* _main_loop)(void);
	int (* _startup_1)(void);
	void (* _startup_2)(void);
	void (* _shutdown)(void);
	void (* _suspend)(void);
	int (*_syscmd)(mp_txt *txt,char *cmd,char *mode);
};


int mpv_strcasecmp(char * s1, char * s2);
FILE * mpv_fopen(char * file, char * mode);
mp_txt * mpv_glob(char * spec);

void mpv_goto(int x, int y);
void mpv_char(int c, int color);
void mpv_str(char * str, int color);
void mpv_cursor(int x, int y);
void mpv_refresh(void);

void mpv_title(char * str);
void mpv_status_line(char * str);

void mpv_add_menu(char * label);
void mpv_add_menu_item(char * funcname);
void mpv_check_menu(char * funcname, int toggle);
int mpv_menu(void);

void mpv_alert(char * msg, char * msg2);
int mpv_confirm(char * prompt);
int mpv_list(char * title, mp_txt * txt, int pos);
char * mpv_readline(int type, char * prompt, char * def);

int mpv_system_to_clipboard(void);
void mpv_clipboard_to_system(void);

void mpv_about(void);
int mpv_help(char * term, int synhi);
int mpv_zoom(int inc);

void mpv_scrollbar(int pos, int size, int max);
void mpv_filetabs(void);

void mpv_set_variable(char * var, char * value);
int mpv_args(int argc, char * argv[]);

void mpv_usage(void);
void mpv_main_loop(void);

int mpv_startup_1(void);
void mpv_startup_2(void);
void mpv_shutdown(void);
void mpv_suspend(void);

int mpv_syscmd(mp_txt *txt,char *cmd,char *mode);
