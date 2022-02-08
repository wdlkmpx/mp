/*

    mpv_curses.c

    Curses Interface (Linux/Unix)

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
#include "mp_core.h"
#include "mp_video.h"


#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <curses.h>
#include <signal.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "mp_synhi.h"
#include "mp_func.h"
#include "mp_iface.h"
#include "mp_lang.h"
#include "mp_conf.h"


/*******************
	Data
********************/

#define START_LINE 1
#define END_LINE 1

/* standard attributes */
static int _attrs[MP_COLOR_NUM];

/** menu **/

/* menu item */

struct mpv_menu_item
{
	char label[40]; 		/* label */
	char * funcname;		/* funcname */
	int hk; 			/* hotkey (not used) */
	int toggle;			/* toggle mark */
	struct mpv_menu_item * next;
	struct mpv_menu_item * prev;
};

/* menu bar */

struct mpv_menu_bar
{
	char label[15]; 		/* label */
	struct mpv_menu_item * first;	/* first item */
	struct mpv_menu_item * last;	/* last item */
	int items;			/* item number */
	int xpos;			/* horizontal position */
	struct mpv_menu_bar * next;
	struct mpv_menu_bar * prev;
};

/* pointers to first and last menu bars */
struct mpv_menu_bar * _mpv_menu=NULL;
struct mpv_menu_bar * _mpv_menu_last=NULL;

/* menu bar horizontal position */
static int _mpv_menu_xpos=2;

#ifndef DEFAULT_INK_COLOR
#define DEFAULT_INK_COLOR COLOR_WHITE
#endif

#ifndef DEFAULT_PAPER_COLOR
#define DEFAULT_PAPER_COLOR COLOR_BLACK
#endif

/* hint text to the left of first line */
char * _mpv_hint_text=NULL;

#ifdef POOR_MAN_BOXES
int _mpv_poor_man_boxes=1;
#else
int _mpv_poor_man_boxes=0;
#endif

#define MPBOX_HLINE ( _mpv_poor_man_boxes ? '-' : ACS_HLINE )
#define MPBOX_VLINE ( _mpv_poor_man_boxes ? '|' : ACS_VLINE )
#define MPBOX_TL_CORNER ( _mpv_poor_man_boxes ? '+' : ACS_ULCORNER )
#define MPBOX_TR_CORNER ( _mpv_poor_man_boxes ? '+' : ACS_URCORNER )
#define MPBOX_BL_CORNER ( _mpv_poor_man_boxes ? '+' : ACS_LLCORNER )
#define MPBOX_BR_CORNER ( _mpv_poor_man_boxes ? '+' : ACS_LRCORNER )
#define MPBOX_THUMB ( _mpv_poor_man_boxes ? ' ' : ACS_CKBOARD )

/* use ncurses mouse */
int _mpv_mouse=0;

/* use transparent terminal mode */
int _mpv_transp_mode=1;

/* reposition hardware cursor */
int _mpv_reposition_cursor=0;

/* 'shift' mode (escape pressed) */
static int _mpv_shift=0;


/*****************
       Code
******************/

/* from mpv_unix_common.c */
extern int _unix_strcasecmp(char *, char *);
extern FILE * _unix_fopen(char * file, char * mode);
extern mp_txt * _unix_glob(char * spec);
extern int _unix_help(char * term, int synhi);
extern void _mpv_strip_cwd(char * buf, int size);
extern int _unix_popen(mp_txt *txt,char *cmd,char *mode);

static int _mpv_isdir(const char *f) {
  struct stat s;
  if (!stat(f,&s) && S_ISDIR(s.st_mode)) return 1;
  return 0;
}



static void _mpv_attrset(int attr)
{
#ifndef ABSOLUTELY_NO_COLORS
	attrset(attr);
#endif
}


static void _mpv_wattrset(WINDOW * w, int attr)
{
#ifndef ABSOLUTELY_NO_COLORS
	wattrset(w,attr);
#endif
}


/**
 * _mpv_sigwinch - SIGWINCH capture function.
 * @s: signal
 *
 * The capture function for the SIGWINCH signal, raised
 * whenever a window resize has been done (from i.e. xterm).
 */
static void _mpv_sigwinch(int s)
{
#ifdef NCURSES_VERSION
	/* Make sure that window size changes... */
	struct winsize ws;
	int fd  = open("/dev/tty",O_RDWR);
	if (fd == -1) return; /* This should never have to happen! */
	if (ioctl(fd,TIOCGWINSZ,&ws) == 0) {
		resizeterm(ws.ws_row,ws.ws_col);
	}
	close(fd);
#else
	mpv_shutdown();
	mpv_startup_2();
#endif

	/* invalidate main window */
	clearok(stdscr,1);
	refresh();

	/* recalc dimensions */
	_mpv_x_size=COLS - 1;
	_mpv_y_size=LINES - START_LINE - END_LINE;

	/* redraw everything */
	mpv_title(NULL);
	mpi_draw_all(_mp_active);

	/* log new size */
	mp_log("New window size: %dx%d\n", COLS, LINES);

	/* reattach */
	signal(SIGWINCH,_mpv_sigwinch);
}


static void _goto(int x, int y)
{
	move(START_LINE+y,x);
}


static void _char(int c, int color)
{
	_mpv_attrset(_attrs[color]);
	addch((unsigned char) c);
}


static void _str(char * str, int color)
{
	_mpv_attrset(_attrs[color]);
	addstr(str);
}


static void _cursor(int x, int y)
{
	static int ox=0;
	static int oy=0;

	if(_mpv_reposition_cursor)
	{
		if(x==-1) x=ox;
		if(y==-1) y=oy;

		move(START_LINE+y,x);

		ox=x;
		oy=y;
	}
}


static void _refresh(void)
{
	refresh();
}


/**
 * _mpv_title_status - Draws the title or status line.
 * @y: the y position
 * @str1: first string
 * @str2: second (optional) string
 *
 * Draws the title or status line with str1 and, if defined,
 * concatenates str2. The y variable indicates the vertical
 * position where the line will be drawn. It is filled with
 * spaces to the right margin (internal).
 */
static void _mpv_title_status(int y, unsigned char * str1,
	unsigned char * str2)
{
	int n=0;
	char * ptr;
	static char _default_hint[256];

	move(y,0);
	_mpv_attrset(_attrs[MP_COLOR_TEXT_TITLE]);

	if(str1!=NULL)
	{
		for(n=0;*str1 && n < _mpv_x_size;n++,str1++)
			addch(*str1);
	}

	if(str2!=NULL)
	{
		for(ptr=" - ";*ptr && n < _mpv_x_size;n++,ptr++)
			addch(*ptr);
		for(;*str2 && n < _mpv_x_size;n++,str2++)
			addch(*str2);
	}

	if(_mpv_hint_text==NULL)
	{
		char * k;
		char * t;

		k=mpf_get_keyname_by_funcname("menu");
		t=L("menu");

		snprintf(_default_hint,sizeof(_default_hint),
			"%s %s",k, t);

		_mpv_hint_text=_default_hint;
	}

	if(y!=0)
	{
		for(;n < _mpv_x_size - strlen(_mpv_hint_text) + 1;n++)
			addch(' ');
		addstr(_mpv_hint_text);
	}
	else
	{
		for(;n<_mpv_x_size+1;n++)
			addch(' ');
	}

	mpv_cursor(-1,-1);
}


static void _title(char * str)
{
	mp_txt * t;
	char files[1024];
	int n,m;
	char * ptr;

	if(str==NULL && _mp_active!=NULL)
	{
		str=files;

		snprintf(files, sizeof(files) - 1, "%s%s",
			_mp_active->mod ? "*" : "",
			_mp_active->name);

		if(!_mp_active->sys)
		{
			t=_mp_active->next;
			n=strlen(files);

			m=sizeof(files)-1;
			if(_mpv_x_size < m) m=_mpv_x_size;

			while(n < sizeof(files)-1)
			{
				if(t==NULL) t=_mp_txts;
				if(t==_mp_active) break;

				for(ptr=" | ";*ptr && n < m;n++,ptr++)
					files[n]=*ptr;

				if(n < m && t->mod)
					files[n++]='*';

				for(ptr=t->name;*ptr && n < m;n++,ptr++)
					files[n]=*ptr;

				t=t->next;
			}

			files[n]='\0';
		}
	}

	_mpv_title_status(0, (unsigned char *)" mp " VERSION,
		(unsigned char *)str);
}


static void _status_line(char * str)
{
	_mpv_title_status(LINES - 1, (unsigned char *) str, NULL);
}


static int _zoom(int inc)
{
	return(0);
}


#define ctrl(k) ((k)&31)

static void __main_loop(void)
{
	char * ptr=NULL;
	int k;

	mpi_move_selecting=0;

	k=getch();

	if(_mpv_shift)
	{
		switch(k)
		{
		case '0':	ptr="f10"; break;
		case '1':	ptr="f1"; break;
		case '2':	ptr="f2"; break;
		case '3':	ptr="f3"; break;
		case '4':	ptr="f4"; break;
		case '5':	ptr="f5"; break;
		case '6':	ptr="f6"; break;
		case '7':	ptr="f7"; break;
		case '8':	ptr="f8"; break;
		case '9':	ptr="f9"; break;

		case KEY_LEFT:	ptr="ctrl-cursor-left"; break;
		case KEY_RIGHT: ptr="ctrl-cursor-right"; break;
		case KEY_DOWN:	ptr="ctrl-cursor-down"; break;
		case KEY_UP:	ptr="ctrl-cursor-up"; break;
		case KEY_END:	ptr="ctrl-end"; break;
		case KEY_HOME:	ptr="ctrl-home"; break;
		case '\r':	ptr="ctrl-enter"; break;
		case '\e':	ptr="escape"; break;
		case KEY_ENTER:	ptr="ctrl-enter"; break;

		case ' ':	ptr="ctrl-space"; break;
		case 'a':	ptr="ctrl-a"; break;
		case 'b':	ptr="ctrl-b"; break;
		case 'c':	ptr="ctrl-c"; break;
		case 'd':	ptr="ctrl-d"; break;
		case 'e':	ptr="ctrl-e"; break;
		case 'f':	ptr="ctrl-f"; break;
		case 'g':	ptr="ctrl-g"; break;
		case 'h':	ptr="ctrl-h"; break;
		case 'i':	ptr="ctrl-i"; break;
		case 'j':	ptr="ctrl-j"; break;
		case 'k':	ptr="ctrl-k"; break;
		case 'l':	ptr="ctrl-l"; break;
		case 'm':	ptr="ctrl-m"; break;
		case 'n':	ptr="ctrl-n"; break;
		case 'o':	ptr="ctrl-o"; break;
		case 'p':	ptr="ctrl-p"; break;
		case 'q':	ptr="ctrl-q"; break;
		case 'r':	ptr="ctrl-r"; break;
		case 's':	ptr="ctrl-s"; break;
		case 't':	ptr="ctrl-t"; break;
		case 'u':	ptr="ctrl-u"; break;
		case 'v':	ptr="ctrl-v"; break;
		case 'w':	ptr="ctrl-w"; break;
		case 'x':	ptr="ctrl-x"; break;
		case 'y':	ptr="ctrl-y"; break;
		case 'z':	ptr="ctrl-z"; break;
		default:	k=0; break;
		}

		_mpv_shift=0;
	}
	else
	{
		/* Del is rubout */
		if(k==0x7f) k='\b';

		if(k < 32 || k > 255)
		{
			if(k=='\e') _mpv_shift=1;

			switch(k)
			{
			case KEY_BACKSPACE:
			case '\b':	ptr="backspace"; break;
			case KEY_UP:	ptr="cursor-up"; break;
			case KEY_DOWN:	ptr="cursor-down"; break;
			case KEY_LEFT:	ptr="cursor-left"; break;
			case KEY_RIGHT: ptr="cursor-right"; break;
			case KEY_NPAGE: ptr="page-down"; break;
			case KEY_PPAGE: ptr="page-up"; break;
			case KEY_HOME:	ptr="home"; break;
			case KEY_END:	ptr="end"; break;
			case KEY_IC:	ptr="insert"; break;
			case KEY_DC:	ptr="delete"; break;
			case '\r':	ptr="enter"; break;
			case KEY_ENTER:	ptr="enter"; break;
			case '\t':	ptr="tab"; break;
			case KEY_F(1):	ptr="f1"; break;
			case KEY_F(2):	ptr="f2"; break;
			case KEY_F(3):	ptr="f3"; break;
			case KEY_F(4):	ptr="f4"; break;
			case KEY_F(5):	ptr="f5"; break;
			case KEY_F(6):	ptr="f6"; break;
			case KEY_F(7):	ptr="f7"; break;
			case KEY_F(8):	ptr="f8"; break;
			case KEY_F(9):	ptr="f9"; break;
			case KEY_F(10): ptr="f10"; break;
			case ctrl(' '): ptr="ctrl-space"; break;
			case ctrl('a'): ptr="ctrl-a"; break;
			case ctrl('b'): ptr="ctrl-b"; break;
			case ctrl('c'): ptr="ctrl-c"; break;
			case ctrl('d'): ptr="ctrl-d"; break;
			case ctrl('e'): ptr="ctrl-e"; break;
			case ctrl('f'): ptr="ctrl-f"; break;
			case ctrl('g'): ptr="ctrl-g"; break;
			case ctrl('j'): ptr="ctrl-j"; break;
			case ctrl('l'): ptr="ctrl-l"; break;
			case ctrl('n'): ptr="ctrl-n"; break;
			case ctrl('o'): ptr="ctrl-o"; break;
			case ctrl('p'): ptr="ctrl-p"; break;
			case ctrl('q'): ptr="ctrl-q"; break;
			case ctrl('r'): ptr="ctrl-r"; break;
			case ctrl('s'): ptr="ctrl-s"; break;
			case ctrl('t'): ptr="ctrl-t"; break;
			case ctrl('u'): ptr="ctrl-u"; break;
			case ctrl('v'): ptr="ctrl-v"; break;
			case ctrl('w'): ptr="ctrl-w"; break;
			case ctrl('x'): ptr="ctrl-x"; break;
			case ctrl('y'): ptr="ctrl-y"; break;
			case ctrl('z'): ptr="ctrl-z"; break;

			case KEY_SUSPEND: ptr="suspend" ; break;
			case KEY_SLEFT: mpi_move_selecting=1; ptr="cursor-left"; break;
			case KEY_SRIGHT: mpi_move_selecting=1; ptr="cursor-right"; break;
			case KEY_SHOME: mpi_move_selecting=1; ptr="home"; break;
			case KEY_SEND: mpi_move_selecting=1; ptr="end"; break;

			/* probably non-standard */
			case 0476: ptr="kp-divide"; break;
			case 0477: ptr="kp-multiply"; break;
			case 0500: ptr="kp-minus"; break;
			case 0501: ptr="kp-plus"; break;

#ifdef NCURSES_MOUSE_VERSION

			case KEY_MOUSE:
			{
				MEVENT m;

				getmouse(&m);
				mp_move_xy(_mp_active, m.x,
					_mp_active->vy + m.y-1);

				if(m.bstate & BUTTON1_PRESSED)
					ptr="mouse-left-button";
				else
				if(m.bstate & BUTTON2_PRESSED)
					ptr="mouse-middle-button";
				else
				if(m.bstate & BUTTON3_PRESSED)
					ptr="mouse-right-button";
				else
				if(m.bstate & BUTTON4_PRESSED)
					ptr="mouse-wheel-down";

				break;
			}
#endif
			default: k=0; break;
			}
		}
	}

	if(k!='\0' || ptr!=NULL)
		mpi_process(k, ptr, NULL);
}


static void _main_loop(void)
{
	while(!_mpi_exit_requested)
		__main_loop();
}


static void _add_menu(char * label)
{
	struct mpv_menu_bar * m;
	int n;

	label=L(label);
	m=(struct mpv_menu_bar *) malloc(sizeof(struct mpv_menu_bar));

	if(m==NULL) return;

	for(n=0;*label;label++)
	{
		if(*label!='&')
			m->label[n++]=*label;
	}
	m->label[n]='\0';

	m->first=m->last=NULL;
	m->items = 0;
	m->xpos=_mpv_menu_xpos;

	m->next=NULL;

	if(_mpv_menu==NULL)
	{
		_mpv_menu=m;
		m->prev=NULL;
	}
	else
	{
		_mpv_menu_last->next=m;
		m->prev=_mpv_menu_last;
	}

	/* this is the last, by now */
	_mpv_menu_last=m;

	_mpv_menu_xpos+=(strlen(m->label) + 3);
}


static void _add_menu_item(char * funcname)
{
	struct mpv_menu_item * i;
	int n, l;
	char * ptr;
	int * t;

	if((ptr=mpf_get_desc_by_funcname(funcname)) == NULL)
		return;

	ptr=L(ptr);

	i=(struct mpv_menu_item *) malloc(sizeof(struct mpv_menu_item));

	if(i==NULL) return;

	i->funcname=funcname;
	i->next=NULL;

	if((t=mpf_toggle_function_value(funcname)) != NULL)
		i->toggle=*t;
	else
		i->toggle=0;

	l=sizeof(i->label) - 3;

	/* copy the label */
	for(n=0;n < l && *ptr;n++,ptr++)
		i->label[n]=*ptr;

	/* fill with spaces to the end */
	for(;n < l;n++)
		i->label[n]=' ';

	/* null terminate */
	i->label[n]='\0';

	/* if there is a keyname, overwrite the label */
	if((ptr=mpf_get_keyname_by_funcname(funcname)) != NULL)
	{
		/* write a space */
		n=l - strlen(ptr) - 1;
		i->label[n++]=' ';

		/* write the keyname */
		for(;*ptr;n++,ptr++)
			i->label[n]=*ptr;
	}

	if(_mpv_menu_last->first==NULL)
	{
		_mpv_menu_last->first=i;
		i->prev=NULL;
	}
	else
	{
		_mpv_menu_last->last->next=i;
		i->prev=_mpv_menu_last->last;
	}

	_mpv_menu_last->last=i;

	_mpv_menu_last->items++;
}


static void _check_menu(char * funcname, int toggle)
{
	struct mpv_menu_bar * m;
	struct mpv_menu_item * i;

	for(m=_mpv_menu;m!=NULL;m=m->next)
	{
		for(i=m->first;i!=NULL;i=i->next)
		{
			if(i->funcname != NULL &&
			   strcmp(i->funcname, funcname)==0)
			{
				i->toggle=toggle;
				return;
			}
		}
	}
}


static void _mpv_box(WINDOW * w, int tx, int ty)
{
	int x,y;

	wmove(w,0,0);
	waddch(w,MPBOX_TL_CORNER);
	for(x=1;x < tx-1;x++)
		waddch(w,MPBOX_HLINE);
	waddch(w,MPBOX_TR_CORNER);

	for(y=1;y < ty-1;y++)
	{
		_mpv_wattrset(w,_attrs[MP_COLOR_TEXT_FRAME1]);
		wmove(w,y,0);
		waddch(w,MPBOX_VLINE);
		_mpv_wattrset(w,_attrs[MP_COLOR_TEXT_FRAME2]);
		wmove(w,y,tx-1);
		waddch(w,MPBOX_VLINE);
	}

	waddch(w,MPBOX_BL_CORNER);
	for(x=1;x < tx-1;x++)
		waddch(w,MPBOX_HLINE);
	waddch(w,MPBOX_BR_CORNER);
}


static int _menu(void)
{
	int k=0,ok;
	char tmp[1024];
	struct mpv_menu_bar * m;
	struct mpv_menu_item * i;
	struct mpv_menu_item * i2;
	WINDOW * w;
	int x,y,c,cc,sy;
	char * funcname=NULL;

	tmp[0]='\0';
	for(m=_mpv_menu;m!=NULL;m=m->next)
	{
		strcat(tmp,"   ");
		strcat(tmp,m->label);
	}

	_mpv_hint_text=L("ESC Cancel");
	_mpv_title_status(0, (unsigned char *) tmp, NULL);
	m=_mpv_menu;

	ok=0;
	while(!ok)
	{
		i=m->first;

		w=newwin(m->items+2, sizeof(i->label), 1, 
			 m->xpos + sizeof(i->label) < _mpv_x_size ?
			 m->xpos : _mpv_x_size - sizeof(i->label));

		if(w==NULL) return(0);

		_mpv_wattrset(w,_attrs[MP_COLOR_TEXT_FRAME1]);
		_mpv_box(w,sizeof(i->label),m->items+2);
		wrefresh(w);

		while(!ok)
		{
			/* draw items */
			for(sy=y=1,i2=m->first;i2!=NULL;y++,i2=i2->next)
			{
				if(i2==i)
				{
					c=_attrs[MP_COLOR_TEXT_MENU_SEL];
					sy=y;
				}
				else
					c=_attrs[MP_COLOR_TEXT_MENU_ELEM];

				/* if '-', then item is a separator */
				if(i2->label[0]=='-')
				{
					wmove(w,y,1);
					_mpv_wattrset(w,_attrs[MP_COLOR_TEXT_FRAME1]);

					/* waddch(w,ACS_LTEE); */
					for(x=0;x<sizeof(i2->label)-2;x++)
						waddch(w,MPBOX_HLINE);
					/* waddch(w,ACS_RTEE); */
				}
				else
				{
					wmove(w,y,1);
					_mpv_wattrset(w,c);

					for(x=0;x<sizeof(i->label) && (cc=i2->label[x]);x++)
					{
						if(cc=='&')
						{
							/* _mpv_wattrset(w,_menu_act_attr); */
							cc=i2->label[++x];
						}
						else
							_mpv_wattrset(w,c);

						waddch(w,(unsigned char)cc);
					}

					/* draws toggle mark */
					if(i2->toggle)
						waddch(w,'*');
					else
						waddch(w,' ');
				}
			}

			wmove(w,sy,1);
			wrefresh(w);
			k=getch();

			/* possible keys */
			if(k==0x1b || k==ctrl('A') || k==ctrl('N') || k==ctrl(' '))
			{
				k=0;
				ok=1;
			}
			else
			if(k==KEY_RIGHT)
			{
				if((m=m->next)==NULL)
					m=_mpv_menu;
				break;
			}
			else
			if(k==KEY_LEFT)
			{
				if((m=m->prev)==NULL)
					m=_mpv_menu_last;
				break;
			}
			else
			if(k==KEY_DOWN)
			{
				if((i=i->next)==NULL)
					i=m->first;

				/* this assumes a separator can't be
				   the last item of a menu, so don't do it */
				if(i->label[0]=='-')
					i=i->next;
			}
			else
			if(k==KEY_UP)
			{
				if((i=i->prev)==NULL)
					i=m->last;

				/* this assumes a separator can't be
				   the last item of a menu, so don't do it */
				if(i->label[0]=='-')
					i=i->prev;
			}
			else
			if(k=='\r' || k==KEY_ENTER)
			{
				funcname=i->funcname;
				ok=1;
			}
			else
				ok=1;
		}

		delwin(w);
		touchwin(stdscr);
		refresh();
	}

	_mpv_hint_text=NULL;

	if(funcname!=NULL)
		mpi_process('\0', NULL, funcname);

	return(2);
}


static int _mpv_prompt(char * prompt, char * prompt2)
{
	int c;
	char * yes;
	char * no;

	move(LINES - 1,0);
	_mpv_attrset(_attrs[MP_COLOR_TEXT_TITLE]);

	addstr(prompt);
	if(prompt2)
		addstr(prompt2);

	yes=L("Y");
	no=L("N");

	for(;;)
	{
		c=getch();

		if(toupper(c)==*yes || toupper(c)==*no ||
			c=='\r' || c==KEY_ENTER)
		{
			mpv_status_line(NULL);
			break;
		}
	}

	if(toupper(c)==*no) return(0);

	return(1);
}


static int _confirm(char * prompt)
{
	return(_mpv_prompt(prompt,L(" [Y/N]")));
}


static void _alert(char * msg, char * msg2)
{
	char tmp[4096];

	if(msg2==NULL)
		strncpy(tmp,msg,sizeof(tmp));
	else
		sprintf(tmp,msg,msg2);

	_mpv_prompt(tmp,L(" [ENTER]"));
}


static int _list(char * title, mp_txt * txt, int pos)
{
	int c,ok,ret;
	char * funcname;
	mp_txt * tmp;
	int ts;

	_mpv_hint_text=L("ESC Cancel");
	mpv_title(title);

	mp_move_bof(txt);

	if(txt->lasty==0)
	{
		/* no lines or just one line: exit */
		_mpv_hint_text=NULL;
		return(0);
	}

	txt->type=MP_TYPE_LIST;
	txt->mod=0;

	tmp=_mp_active;
	ts=_mp_tab_size;

	_mp_active=txt;
	_mp_tab_size=20;

	/* move to desired line */
	mp_move_xy(_mp_active, 0, pos);

	mpi_draw_all(_mp_active);

	ret=-1;

	ok=0;
	while(!ok)
	{
		c=getch();

		funcname=NULL;

		switch(c)
		{
		case KEY_LEFT:
		case KEY_UP:	funcname="move-up"; break;
		case KEY_RIGHT:
		case KEY_DOWN:	funcname="move-down"; break;
		case KEY_NPAGE: funcname="move-page-down"; break;
		case KEY_PPAGE: funcname="move-page-up"; break;
		case KEY_HOME:	funcname="move-bof"; break;
		case KEY_END:	funcname="move-eof"; break;

		case '\r':
		case KEY_ENTER:
			ret=_mp_active->y;
			/* falls thru */

		case KEY_F(4):
		case '\e':
			ok=1;
			break;
		}

		mpi_process('\0', NULL, funcname);
	}

	_mpv_hint_text=NULL;

	_mp_active=tmp;
	_mp_tab_size=ts;

	return(ret);
}


/**
 * _mpv_open_file_list - Shows a list of files to open
 * @rcpt: buffer to receive the selected file
 *
 * Asks for a file scheme and shows a list of files
 * matching it.
 * Returns the pushed key to exit ('\r' or '\e').
 * If no or just one file is matched, '\r' is also returned.
 * The selected file will be written into the rcpt
 * buffer.
 * This is an ugly hack: it will be rewritten
 * sooner or later.
 */
static int _mpv_open_file_list(char * rcpt)
{
  do {
    mp_txt * txt=NULL;
    char tmp[1024];
    int l;

    if(strchr(rcpt,'*')!=NULL || strchr(rcpt,'?')!=NULL) {
      char *p;
      /* We have globbers... */
      if((txt=mpv_glob(rcpt))==NULL) return '\r';
      mp_move_xy(txt,0,0);
      p = strrchr(rcpt,'/');
      if (p == rcpt) {
	mp_put_str(txt,"/\n",1);
      } else if (p) {
        *p = 0;
        mp_put_str(txt,p,1);
        mp_put_char(txt,'\n',1);
      } else {
        mp_put_str(txt,".\n",1);
      }
    } else {
      struct dirent *ent;
      DIR *dp = opendir(rcpt);

      if (!dp) return '\e';

      l = strlen(rcpt);
      while (rcpt[l-1] == '/' && l > 0) rcpt[--l] = 0;

      txt=mp_create_sys_txt("<glob>");
      MP_SAVE_STATE();
      while ((ent=readdir(dp)) != NULL) {
	if (ent->d_name[0] == '.' && ent->d_name[1] == '\0') continue;
	if (ent->d_name[0] == '.' && ent->d_name[1] == '.' && 
	  ent->d_name[2] == '\0') {
	  if (rcpt[0] != '/' || rcpt[1] != '\0') mp_put_str(txt,"..\n",1);
	  continue;
	}
	if (strcmp(rcpt,".")) {
	  mp_put_str(txt,rcpt,1);
	  if (rcpt[0] != '/' || rcpt[1] != '\0') mp_put_char(txt,'/',1);
	}
	mp_put_str(txt,ent->d_name,1);
	
	snprintf(tmp,sizeof(tmp),"%s/%s",rcpt,ent->d_name);
	if (_mpv_isdir(tmp)) mp_put_char(txt,'/',1);
	mp_put_char(txt,'\n',1);

      }
      closedir(dp);

      mp_move_left(txt); mp_delete_char(txt); mp_move_bof(txt);
      mp_sort(txt);
      mp_move_left(txt); mp_delete_char(txt); mp_move_bof(txt);
      
      MP_RESTORE_STATE();
    }
    if ((l=mpv_list("file list", txt, 0)) == -1) { 
      mp_delete_sys_txt(txt);
      return '\e';
    }

    mp_move_xy(txt,0,l);
    mp_get_str(txt,tmp,sizeof(tmp),'\n');
    mp_delete_sys_txt(txt);

    if (!strcmp(tmp,"..")) {
      /* Special case for going up a directory level... */
      char *p = rcpt;
      if (p[0] == '.' && p[1] == '/') p+=2;
      p = strchr(p,'/');
      if (!p) {
      	/* No /'s found, translate to realpath... */
      	char line[PATH_MAX];
      	char cwd[1024];


      	getcwd(cwd,sizeof(cwd));
      	snprintf(tmp,sizeof(tmp),"%s/%s",cwd,rcpt);
      	if (realpath(tmp,line))
      	  strncpy(rcpt,line,1024);
      	else
      	  strcpy(rcpt,"/");
      }
      p = strrchr(rcpt+1,'/');
      if (p)
        *p = 0;
      else
        strcpy(rcpt,"/");
    } else
      strncpy(rcpt,tmp,1024);
  } while (_mpv_isdir(rcpt));
  return '\r';
}


static char * _readline(int type, char * prompt, char * def)
{
	static char tmp[1024];
	int c,cursor;
	int n,history;
	int echo;

	move(LINES - 1,0);
	_mpv_attrset(_attrs[MP_COLOR_TEXT_TITLE]);

	/* clean line */
	for(n=0;n < COLS;n++) addch(' ');
	move(LINES - 1,0);

	if(def==NULL)
		tmp[0]='\0';
	else
		strncpy(tmp,def,sizeof(tmp));

	cursor=strlen(tmp);

	addstr(prompt);
	addstr(tmp);

	history=mpi_history_size(type) + 1;

	echo=(type != MPR_PASSWORD);

	for(;;)
	{
		c=getch();

		if(c=='\r' || c==KEY_ENTER || c=='\e')
			break;

		if((c==KEY_BACKSPACE || c=='\b' || c == 0x7f) && cursor > 0)
		{
			tmp[--cursor]='\0';

			if(echo)
			{
				addstr("\b \b");
				refresh();
			}
		}
		else
		if(c==ctrl('U'))
		{
			tmp[cursor=0]='\0';

			if(echo)
			{
				while(cursor--) { addstr("\b \b"); }
				refresh();
			}
		}
		else
		if(c==KEY_UP && echo)
		{
			if(history > 0) history--;

			mpi_history_get(type, history, tmp, sizeof(tmp));

			while(cursor--) { addstr("\b \b"); }
			addstr(tmp);
			cursor=strlen(tmp);
			refresh();
		}
		else
		if(c==KEY_DOWN && echo)
		{
			if(history < mpi_history_size(type))
				history++;

			mpi_history_get(type, history, tmp, sizeof(tmp));

			while(cursor--) { addstr("\b \b"); }
			addstr(tmp);
			cursor=strlen(tmp);
			refresh();
		}
		else
		if(c=='\t' && echo)
		{
			tmp[cursor++]='*';
			tmp[cursor]='\0';
			c='\r';
			break;
		}
		else
		if(c>=32 && c<=255 && c != 0x7f && cursor < 56)
		{
			tmp[cursor++]=c;
			tmp[cursor]='\0';

			if(echo)
			{
				addch(c);
				refresh();
			}
		}
	}

	if(c=='\e')
		return(NULL);

	if(type==MPR_OPEN)
	{
		if(tmp[0]=='~') {
		  char line[1024];
		  strncpy(line,tmp+1,sizeof(line));
		  snprintf(tmp,sizeof(tmp),"%s%s",_mpc_home,line);
		}

		if(tmp[0]=='\0')
			strncpy(tmp,"*",sizeof(tmp));

		if(strchr(tmp,'*')!=NULL || strchr(tmp,'?')!=NULL ||
			 _mpv_isdir(tmp))
		  c=_mpv_open_file_list(tmp);

	}

	mpv_status_line(NULL);

	if(c=='\e')
		return(NULL);

	/* store line in history */
	mpi_history_add(type, tmp);

	return(tmp);
}


static int _sys_to_clip(void)
{
	return(1);
}


static void _clip_to_sys(void)
{
}


static void _about(void)
{
	mp_txt * txt;

	txt=mp_create_txt(L("<about Minimum Profit>"));

	mp_put_str(txt,MP_LICENSE,1);

	mp_move_bof(txt);
	txt->type=MP_TYPE_READ_ONLY;
	txt->mod=0;
}


/**
 * mpv_notify - The notify function.
 * @str: the str
 *
 * The notify function for mp_set_notify().
 */
static void mpv_notify(char * str)
{
	mpv_status_line(str);
	mpv_refresh();
}


#define R3(a,b,c) (((a)*(b))/(c))

static void _scrollbar(int pos, int size, int max)
{
	int n;
	int l,u,a;

	if(max < size)
	{
		l=0;
		u=_mpv_y_size;
		a=1;
	}
	else
	{
		l=R3(pos,size,max);
		u=R3(pos+size,size,max);
		a=0;
	}

	for(n=0;n<_mpv_y_size;n++)
	{
		move(START_LINE+n,_mpv_x_size);

		if(n>=l && n<=u && !a)
		{
			_mpv_attrset(_attrs[MP_COLOR_TEXT_SCR_THUMB]);
			addch(MPBOX_THUMB);
		}
		else
		{
			_mpv_attrset(_attrs[MP_COLOR_TEXT_SCROLLBAR]);
			addch(MPBOX_VLINE);
		}
	}
}


static void _filetabs(int rebuild)
{
	/* dummy! */
}


/**
 * _mpv_create_colors - Creates the colors defined in the color database
 *
 * Creates the colors defined in the color database, probably
 * set from the configuration file.
 */
static void _mpv_create_colors(void)
{
	int n;
	int di,dp;

	use_default_colors();

	di=DEFAULT_INK_COLOR;
	dp=DEFAULT_PAPER_COLOR;

	if(_mpv_transp_mode)
		di=dp=-1;
	else
	{
		/* if transparent mode is disabled and
		   normal ink or paper is -1,
		   set as default */
		if(mpc_color_desc[0].ink_text==-1)
			mpc_color_desc[0].ink_text=di;
		if(mpc_color_desc[0].paper_text==-1)
			mpc_color_desc[0].paper_text=dp;
	}

	for(n=0;n < MP_COLOR_NUM;n++)
	{
		int i,p;

		i=mpc_color_desc[n].ink_text;
		p=mpc_color_desc[n].paper_text;

		if(n!=0 && (i==-1 || p==-1))
		{
			if(i==-1) i=mpc_color_desc[0].ink_text;
			if(p==-1) p=mpc_color_desc[0].paper_text;
		}

		if(mpi_monochrome)
			init_pair(n+1,di,dp);
		else
			init_pair(n+1,i,p);

		_attrs[n]=(COLOR_PAIR(n+1)
			| (mpc_color_desc[n].reverse ? A_REVERSE : 0)
			| (mpc_color_desc[n].bright ? A_BOLD : 0)
			| (mpc_color_desc[n].underline ? A_UNDERLINE : 0));
	}
}


static int _startup_1(void)
{
	return(1);
}


static void _startup_2(void)
{
	mp_log("Using curses/ncurses driver\n");

	initscr();
	start_color();

#ifdef NCURSES_MOUSE_VERSION
	if(_mpv_mouse)
		mousemask(BUTTON1_PRESSED|BUTTON2_PRESSED|
			  BUTTON3_PRESSED|BUTTON4_PRESSED,
			NULL);

	mp_log("Ncurses mouse version: %d\n",NCURSES_MOUSE_VERSION);
#endif

	keypad(stdscr, TRUE);
	nonl();
	raw();
	noecho();

	_mpv_create_colors();

	_mpv_x_size=COLS - 1;
	_mpv_y_size=LINES - START_LINE - END_LINE;

	signal(SIGWINCH,_mpv_sigwinch);

	mp_set_notify(mpv_notify);
	mpv_status_line(NULL);

	fclose(stderr);

	/* metainfo */

#ifdef NCURSES_VERSION
	mp_log("Ncurses version: %s\n",NCURSES_VERSION);
#else
	mp_log("Using plain curses library\n");
#endif
#ifdef POOR_MAN_BOXES
	mp_log("Poor man boxes active by default\n");
#endif

	mp_log("Default colors - ink: %d paper: %d\n",
		DEFAULT_INK_COLOR, DEFAULT_PAPER_COLOR);
}


static void _shutdown(void)
{
	mpv_title(NULL);
	mpv_refresh();

	endwin();
	putchar('\n');
}

static void _suspend(void)
{
	mpv_shutdown();
	mp_log("Suspending MP...\n");
	kill(0, SIGSTOP);
	mp_log("Resumming MP...\n");
	mpv_startup_2();
	_mpv_sigwinch(0);
	_mpv_sigwinch(0); /* For some reasons, CURSES needs this twice! */
}

static int _syscmd(mp_txt *txt,char *cmd,char *mode) {
  if (txt && mode) {
    return _unix_popen(txt,cmd,mode);
  } else {
    int ret;
    mpv_shutdown();
    mp_log("Executing %s\n",cmd);
    ret = system(cmd);
    mpv_startup_2();
    _mpv_sigwinch(0);
    _mpv_sigwinch(0);
    mp_log("Execution returned %d\n",ret);
    return ret;
  }
}



static void _usage(void)
{
	_shutdown();

	printf("Minimum Profit " VERSION " - Programmer Text Editor\n");
	printf("Copyright (C) 1991-2005 Angel Ortega <angel@triptico.com>\n");
	printf("%s\n", __DATE__ " " __TIME__);
	printf("This software is covered by the GPL license. NO WARRANTY.\n\n");

	printf("%s\n", L("\
Usage: mp [options] [file [file ...]]\n\
\n\
Options:\n\
\n\
 -t|--tag [tag] 	Edits the file where tag is defined\n\
 -w|--word-wrap [col]	Sets wordwrapping in column col\n\
 -ts|--tab-size [size]	Sets tab size\n\
 -ai|--autoindent	Sets automatic indentation mode\n\
 -l|--lang [lang]	Language selection\n\
 -m|--mode [mode]	Syntax-hilight mode\n\
 --col80		Marks column # 80\n\
 -bw|--monochrome	Monochrome\n\
 -tx|--text		Use text mode (instead of GUI)\n\
 -sp|--spellcheck	Active spellchecking\n\
 -h|--help		This help screen\n\
\n\
 -hw|--hardware-cursor	Activates the use of hardware cursor\n\
 --mouse		Activate mouse usage for cursor positioning\n\
 -nt|--no-transparent	Disable transparent mode (eterm, aterm, etc.)\n\
			"));

	printf("--mode: %s\n",mps_enumerate_modes());
	printf("\nConfig file: %s\n",_mpc_config_file);
}


static void _set_var(char * var, char * value)
{
	if(strcmp(var,"mouse")==0)
		_mpv_mouse=atoi(value);
	else
	if(strcmp(var,"hardware_cursor")==0)
		_mpv_reposition_cursor=atoi(value);
	else
	if(strcmp(var,"transparent")==0)
		_mpv_transp_mode=atoi(value);
	else
	if(strcmp(var,"poor_man_boxes")==0)
		_mpv_poor_man_boxes=atoi(value);
}


static int _args(int argc, char * argv[])
{
	int n;

	for(n=1;n < argc;n++)
	{
		if(argv[n] == NULL)
			continue;

		if(strcmp(argv[n], "--mouse") == 0)
			_mpv_mouse=1;
		else
		if(strcmp(argv[n], "--hardware-cursor") == 0 ||
		   strcmp(argv[n], "-hw") == 0)
			_mpv_reposition_cursor=1;
		else
		if(strcmp(argv[n], "--no-transparent") == 0 ||
		   strcmp(argv[n], "-nt") == 0)
			_mpv_transp_mode ^= 1;
		else
			continue;

		argv[n]=NULL;
	}

	return(0);
}


struct _mpv_driver _mpv_driver_curses=
{
	"curses", 1,
	_unix_strcasecmp, _unix_fopen, _unix_glob, _goto, _char, _str,
	_cursor, _refresh, _title, _status_line, _add_menu,
	_add_menu_item, _check_menu, _menu, _alert, _confirm,
	_list, _readline, _sys_to_clip, _clip_to_sys, _about,
	_unix_help, _zoom, _scrollbar, _filetabs, _set_var,
	_args, _usage, _main_loop,
	 _startup_1, _startup_2, _shutdown, _suspend, _syscmd
};

