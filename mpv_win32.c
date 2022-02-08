/*

    mpv_win32.c

    Win32 Interface.

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

#include "config.h"

#include <stdio.h>

#include "mp_core.h"
#include "mp_video.h"

#ifdef CONFOPT_WIN32

#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <commctrl.h>

#include "mp_func.h"
#include "mp_lang.h"
#include "mp_iface.h"
#include "mp_synhi.h"
#include "mp_res.h"
#include "mp_conf.h"

/*******************
	 Data
 *******************/

/* Win32 necessary globals */

/* the instance */
HINSTANCE hinst;

/* the windows */
HWND hwnd;
HWND hwtabs;

/* font handles */
HFONT _font_normal=NULL;
HFONT _font_italic=NULL;
HFONT _font_underline=NULL;

/* frame buffer with chars in document window */
static int * _mpv_fb=NULL;

/* cursor position */
static int _mpv_x_cursor;
static int _mpv_y_cursor;

/* temporal buffer */
static char _mpv_buffer[1024];

/* font size */
int _mpv_font_size=12;

/* font face */
char _mpv_font_face[80]="Lucida Console";

/* calculated font sizes in local units */
int _mpv_font_height=0;
int _mpv_font_width=0;

/* attributes */
static COLORREF _inks[MP_COLOR_NUM];
static COLORREF _papers[MP_COLOR_NUM];

/* Used to prevent the WM_CHAR and WM_KEYDOWN handlers from generating two
 * events for the same key */
static int is_wm_keydown = 0;

/* win32.hlp path */
char _mpv_win32help[1024]="http://search.microsoft.com/search/results.aspx?qu=%s";

/* dialog box vars */
static char * _mpv_dlg_prompt=NULL;
static char * _mpv_dlg_default=NULL;
static char * _mpv_list_title="";
static mp_txt * _mpv_list_text=NULL;
static int _mpv_list_pos=0;

/* readline buffer */
static char _mpv_readline_buf[1024];

/* title and status line buffers */
static char _mpv_title_buffer[1024];
static char _mpv_status_line_buffer[1024];

/* menu and submenu handlers */
static HMENU _mmenu=NULL;
static HMENU _smenu=NULL;
static char _menu_label[40]="";

/* height of the tab of files */
int _tab_height=28;

/* microsoft moronity overriding */
char * _argv_[100];

/* sequential menu id */
int _win32_menu_id=1000;

/* stored readline type */
static int _mpv_readline_type;


/*******************
	 Code
 *******************/

static int _strcasecmp(char * s1, char * s2)
{
	return(stricmp(s1,s2));
}


static FILE * _fopen(char * file, char * mode)
{
	if(strcmp(mode, "r") == 0)
		mode="rb";
	else
		mode="wb";

	return(fopen(file, mode));
}


static mp_txt * _glob(char * spec)
{
	mp_txt * txt=NULL;
	WIN32_FIND_DATA f;
	HANDLE h;

	if(spec==NULL || *spec=='\0' || strcmp(spec,"*")==0)
		spec="*.*";

	txt=mp_create_sys_txt("<glob>");

	if((h=FindFirstFile(spec,&f))==INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL,"FindFirstFile unexpected error","ERROR",MB_OK);
		return(NULL);
	}

	do
	{
		if(strcmp(f.cFileName,".")==0 ||
		   strcmp(f.cFileName,"..")==0)
			continue;

		if(f.dwFileAttributes &
			FILE_ATTRIBUTE_DIRECTORY)
			continue;

		mp_put_str(txt,f.cFileName,1);
		mp_put_char(txt,'\n',1);
	}
	while(FindNextFile(h,&f));

	FindClose(h);

	return(txt);
}


static void _goto(int x, int y)
{
	/* just store the coords */
	_mpv_x_cursor=x;
	_mpv_y_cursor=y;
}


static void _char(int c, int color)
{
	if(_mpv_y_cursor >= _mpv_y_size ||
	   _mpv_x_cursor >= _mpv_x_size)
		return;

	/* fill the frame buffer */
	_mpv_fb[(_mpv_y_cursor * _mpv_x_size) + _mpv_x_cursor]=
		(color << 8)|((unsigned char)c);

	_mpv_x_cursor++;
}


static void _str(char * str, int color)
{
	int * fb;

	fb=&_mpv_fb[(_mpv_y_cursor * _mpv_x_size) + _mpv_x_cursor];

	while(*str)
	{
		*fb=(color << 8)|(*str);

		fb++; _mpv_x_cursor++; str++;
	}
}


static void _cursor(int x, int y)
{
	/* nothing; the Windows system caret could be used,
	   but I just hate it */
}


static void _refresh(void)
{
	InvalidateRect(hwnd,NULL,TRUE);
}


static void _mpv_title_status(void)
{
	char tmp[2048];

	strncpy(tmp,"mp " VERSION,sizeof(tmp));

	if(_mpv_title_buffer[0]!='\0')
	{
		strcat(tmp," - ");
		strcat(tmp,_mpv_title_buffer);
	}

	if(_mpv_status_line_buffer[0]!='\0')
		strcat(tmp,_mpv_status_line_buffer);

	SetWindowText(hwnd,tmp);
}


static void _title(char * str)
{
	if(str!=NULL)
		strncpy(_mpv_title_buffer,str,sizeof(_mpv_title_buffer));
	else
	if(_mp_active != NULL)
		strncpy(_mpv_title_buffer,_mp_active->name,sizeof(_mpv_title_buffer));
	else
		_mpv_title_buffer[0]='\0';

	_mpv_title_status();
}


static void _status_line(char * str)
{
	if(str)
		strncpy(_mpv_status_line_buffer,str,sizeof(_mpv_status_line_buffer));
	else
		_mpv_status_line_buffer[0]='\0';

	_mpv_title_status();
}


static void _add_menu(char * label)
{
	label=L(label);

	if(_mmenu)
		AppendMenu(_mmenu,MF_STRING|MF_POPUP,
			(UINT)_smenu,_menu_label);
	else
		_mmenu=CreateMenu();

	strncpy(_menu_label,label,sizeof(_menu_label));
	_smenu=CreatePopupMenu();
}


static void _add_menu_item(char * funcname)
{
	char tmp[1024];
	char * label;
	MENUITEMINFO mi;
	int * i;
	char * keyname;

	if((label=mpf_get_desc_by_funcname(funcname)) == NULL)
		return;

	label=L(label);

	if((keyname=mpf_get_keyname_by_funcname(funcname))!=NULL)
		snprintf(tmp,sizeof(tmp),"%s\t%s",label,keyname);
	else
		strncpy(tmp,label,sizeof(tmp));

	if(tmp[0]=='-')
		AppendMenu(_smenu,MF_SEPARATOR,0,NULL);
	else
	{
		AppendMenu(_smenu,MF_STRING,_win32_menu_id,tmp);

		memset(&mi,'\0',sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.fMask=MIIM_DATA;
		mi.dwItemData=(unsigned long)funcname;

		SetMenuItemInfo(_smenu,_win32_menu_id,FALSE,&mi);

		if((i=mpf_toggle_function_value(funcname))!=NULL)
		{
			CheckMenuItem(_smenu,_win32_menu_id,
				((*i) ? MF_CHECKED : MF_UNCHECKED));
		}

		_win32_menu_id++;
	}
}


static void _check_menu(char * funcname, int toggle)
{
	int n;
	MENUITEMINFO mi;

	/* close the menu, if not closed */
	if(_menu_label[0]!='\0')
		mpv_add_menu("");

	for(n=1000;n < _win32_menu_id;n++)
	{
		memset(&mi,'\0',sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.fMask=MIIM_DATA;

		if(GetMenuItemInfo(_mmenu,n,FALSE,&mi))
		{
			if(mi.dwItemData != 0 &&
			   strcmp((char *)mi.dwItemData,funcname)==0)
			{
				CheckMenuItem(_mmenu, n,
					toggle ? MF_CHECKED :
						 MF_UNCHECKED);

				break;
			}
		}
	}
}


static int _menu(void)
{
	/* dummy */
	return(0);
}


static void _alert(char * msg, char * msg2)
{
	char tmp[4096];

	if(msg2==NULL)
		strncpy(tmp,msg,sizeof(tmp));
	else
		sprintf(tmp,msg,msg2);

	MessageBox(hwnd,tmp,"mp " VERSION,
		MB_ICONWARNING|MB_OK);
}


static int _confirm(char * prompt)
{
	if(MessageBox(hwnd,prompt,"mp " VERSION,
		MB_ICONQUESTION|MB_YESNO)==IDYES)
		return(1);
	else
		return(0);
}


/**
 * TextDlgProc - Procedure for text input dialog
 * @hwnd: the window handler
 * @msg: the message sent
 * @wparam: the word param
 * @lparam: the long word param
 *
 * Procedure for text input dialog. Called from inside MS Windows.
 */
BOOL CALLBACK TextDlgProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	int ret, n;

	switch(msg)
	{
	case WM_INITDIALOG:

		SetWindowText(hwnd,"mp " VERSION);

		SetDlgItemText(hwnd,WMP_1STR_LABEL,
			_mpv_dlg_prompt);


		/* store the history into combo_items */
		SendDlgItemMessage(hwnd,WMP_1STR_EDIT,CB_RESETCONTENT,0,0);
		
		if (_mpv_readline_type == MPR_PASSWORD) {
			SendDlgItemMessage(hwnd, WMP_1STR_EDIT,
				EM_SETPASSWORDCHAR, (WPARAM)'*', (LPARAM)0);
		} else {
			for(n=0;n < 100;n++)
			{
				char tmp[512];

				if(! mpi_history_get(_mpv_readline_type,
					mpi_history_size(_mpv_readline_type) - n,
					tmp, sizeof(tmp)))
					break;

				if(tmp[0]!='\0')
				 	SendDlgItemMessage(hwnd,WMP_1STR_EDIT,
				 			CB_ADDSTRING,
				 			0,(LPARAM)tmp);
			}
			if(_mpv_dlg_default!=NULL)
			{
				SetDlgItemText(hwnd,WMP_1STR_EDIT,
					_mpv_dlg_default);
				SendDlgItemMessage(hwnd,WMP_1STR_EDIT,
					EM_SETSEL, 0, 1000);
			}
		}

		return(TRUE);

	case WM_COMMAND:

		switch(LOWORD(wparam))
		{
		case WMP_OK:
		case WMP_CANCEL:

			if(LOWORD(wparam)==WMP_OK)
			{
				ret=1;
				GetDlgItemText(hwnd,WMP_1STR_EDIT,
					_mpv_readline_buf,
					sizeof(_mpv_readline_buf));
				mpi_history_add(_mpv_readline_type,
					_mpv_readline_buf);
			}
			else
				ret=0;

			EndDialog(hwnd,ret);

			return(TRUE);
		}
	}

	return(FALSE);
}


/**
 * ListDlgProc - Procedure for the list input dialog
 * @hwnd: the window handler
 * @msg: the message sent
 * @wparam: the word param
 * @lparam: the long word param
 *
 * Procedure for list input dialog. Called from inside MS Windows.
 */
BOOL CALLBACK ListDlgProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	int ret;
	HWND lst;
	char line[1024];

	switch(msg)
	{
	case WM_INITDIALOG:

		SetWindowText(hwnd, _mpv_list_title);

		lst=GetDlgItem(hwnd, WMP_LIST);
		SendMessage(lst, WM_SETFONT, 
			(WPARAM) GetStockObject(ANSI_FIXED_FONT), 0);

		{
			int n=20 * 4;

			/* sets tab stops on each 20 quarters of char */
			SendMessage(lst, LB_SETTABSTOPS,
				(WPARAM) 1, (LPARAM) &n);
		}

		/* traverses the list, filling the listbox */
		mp_move_bof(_mpv_list_text);

		while(mp_peek_char(_mpv_list_text)!='\0')
		{
			mp_get_str(_mpv_list_text,line,
				sizeof(line),'\n');

			SendMessage(lst, LB_ADDSTRING, 0,
				(LPARAM) line);
		}

		/* sets the desired element as default */
		SendDlgItemMessage(hwnd, WMP_LIST, LB_SETCURSEL,
			_mpv_list_pos, 0);

		return(TRUE);

	case WM_COMMAND:

		switch(LOWORD(wparam))
		{
		case WMP_OK:
		case WMP_CANCEL:

			if(LOWORD(wparam)==WMP_OK)
				ret=SendDlgItemMessage(hwnd, WMP_LIST,
					LB_GETCURSEL, 0, 0);
			else
				ret=-1;

			EndDialog(hwnd,ret);

			return(TRUE);
		}
	}

	return(FALSE);
}


/**
 * AboutDlgProc - Procedure for the about box
 * @hwnd: the window handler
 * @msg: the message sent
 * @wparam: the word param
 * @lparam: the long word param
 *
 * Procedure for the about box. Called from inside MS Windows.
 */
BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	char tmp[4096];
	char * ptr;
	int n;

	switch(msg)
	{
	case WM_INITDIALOG:

		SetWindowText(hwnd, L("<about Minimum Profit>"));

		/* move the license text converting \n to \r\n */
		for(n=0,ptr=tmp;MP_LICENSE[n];n++)
		{
			if(MP_LICENSE[n]=='\n')
				*(ptr++)='\r';
			*(ptr++)=MP_LICENSE[n];
		}
		*ptr='\0';

		SetDlgItemText(hwnd,WMP_LICENSE,tmp);

		SendDlgItemMessage(hwnd,WMP_LICENSE,
			EM_SETSEL, -1, 0);
		SendDlgItemMessage(hwnd,WMP_LICENSE,
			EM_SETREADONLY, 1, 0);

		return(TRUE);

	case WM_COMMAND:

		SendDlgItemMessage(hwnd,WMP_LICENSE,
			EM_SETSEL, -1, 0);

		switch(LOWORD(wparam))
		{
		case WMP_OK:

			EndDialog(hwnd,0);

			return(TRUE);
		}
	}

	return(FALSE);
}


static int _list(char * title, mp_txt * txt, int pos)
{
	_mpv_list_title=title;
	_mpv_list_text=txt;
	_mpv_list_pos=pos;

	if(txt->lasty==0)
	{
		/* no lines or just one line: exit */
		return(0);
	}

	return(DialogBox(hinst,"DLGLIST",hwnd,ListDlgProc));
}


static char * _readline(int type, char * prompt, char * def)
{
	OPENFILENAME ofn;

	/* store type */
	_mpv_readline_type=type;

	if(def==NULL)
		_mpv_readline_buf[0]='\0';
	else
		strncpy(_mpv_readline_buf,def,sizeof(_mpv_readline_buf));

	if(type==MPR_OPEN || type==MPR_SAVE)
	{
		_mpv_readline_buf[0]='\0';

		memset(&ofn,'\0',sizeof(OPENFILENAME));
		ofn.lStructSize=sizeof(OPENFILENAME);
		ofn.hwndOwner=hwnd;
		ofn.lpstrFilter="*.*\0*.*\0";
		ofn.nFilterIndex=1;
		ofn.lpstrFile=_mpv_readline_buf;
		ofn.nMaxFile=sizeof(_mpv_readline_buf);
		ofn.lpstrTitle=prompt;
		ofn.lpstrDefExt=(def==NULL ? "" : def);

		if(type==MPR_OPEN)
		{
			ofn.Flags=OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|
				OFN_NOCHANGEDIR|OFN_FILEMUSTEXIST;

			if(GetOpenFileName(&ofn))
				return(_mpv_readline_buf);
		}
		else
		{
			ofn.Flags=OFN_HIDEREADONLY;

			if(GetSaveFileName(&ofn))
				return(_mpv_readline_buf);
		}
	}
	else
	{
		_mpv_dlg_prompt=prompt;
		_mpv_dlg_default=def;

		if(DialogBox(hinst,
			type == MPR_PASSWORD ? "DLGPASSWORD" : "DLG1STRING",
			hwnd,TextDlgProc))
			return(_mpv_readline_buf);
	}

	return(NULL);
}


static void _about(void)
{
	DialogBox(hinst,"ABOUTBOX",hwnd,AboutDlgProc);
}


static int _help(char * term, int synhi)
{
	char * ptr;

	if((ptr=strrchr(_mpv_win32help, '.')) != NULL &&
		strcmp(ptr, ".hlp") == 0)
	{
		/* special case: _mpv_win32help contains the
		   name of a MS Windows help file */
		WinHelp(hwnd, _mpv_win32help, HELP_KEY, (DWORD) term);
	}
	else
	{
		char tmp[2048];

		/* rest of cases: _mpv_win32help can contain a %s,
		   where the term will be sprintf-ed */
		if(strstr(_mpv_win32help, "%s") != NULL)
			snprintf(tmp, sizeof(tmp) - 1, _mpv_win32help, term);
		else
			strncpy(tmp, _mpv_win32help, sizeof(tmp) - 1);

		tmp[sizeof(tmp) - 1]='\0';

		ShellExecute(NULL, "open", tmp,	NULL, NULL, SW_SHOWNORMAL);
	}

	return(0);
}


/**
 * _mpv_init_fonts - Starts up font stuff.
 * @hdc: the device context
 *
 * Starts up font stuff.
 */
static void _mpv_init_fonts(HDC hdc)
{
	TEXTMETRIC tm;
	RECT rect;
	int n;
	static int f=1;

	/* create fonts */
	n=-MulDiv(_mpv_font_size,GetDeviceCaps(hdc, LOGPIXELSY),72);

	_font_normal=CreateFont(n,0,0,0,0,0,0,
		0,0,0,0,0,0,_mpv_font_face);

	_font_italic=CreateFont(n,0,0,0,0,1,0,
		0,0,0,0,0,0,_mpv_font_face);

	_font_underline=CreateFont(n,0,0,0,0,0,1,
		0,0,0,0,0,0,_mpv_font_face);

	SelectObject(hdc, _font_normal);

	GetTextMetrics(hdc, &tm);

	/* store sizes */
	_mpv_font_height=tm.tmHeight;
	_mpv_font_width=tm.tmAveCharWidth;

	GetClientRect(hwnd, &rect);

	/* calculate the size in chars */
	_mpv_x_size=((rect.right-rect.left)/_mpv_font_width)+1;
	_mpv_y_size=((rect.bottom-rect.top-_tab_height)/_mpv_font_height)+1;

	/* rebuild framebuffer */
	if(_mpv_fb != NULL) free(_mpv_fb);
	_mpv_fb=(int *) malloc(_mpv_x_size * _mpv_y_size * sizeof(int));

	if(f)
	{
		f=0;
		GetWindowRect(hwnd,&rect);
		SetCursorPos(rect.left+200,rect.top+6);
	}
}


static int _zoom(int inc)
{
	HDC hdc;

	hdc=GetDC(hwnd);

	SelectObject(hdc,GetStockObject(SYSTEM_FONT));

	DeleteObject(_font_normal);
	DeleteObject(_font_italic);
	DeleteObject(_font_underline);

	if(inc>0)
	{
		if(_mpv_font_size < 76)
			_mpv_font_size+=2;
	}
	else
	{
		if(_mpv_font_size > 6)
			_mpv_font_size-=2;
	}

	_mpv_init_fonts(hdc);

	mpi_draw_all(_mp_active);

	ReleaseDC(hwnd,hdc);

	return(0);
}


/**
 * _mpv_paint - Dump the frame buffer to screen.
 * @hwnd: the window handler
 *
 * Dumps the document window frame buffer to the window.
 */
static void _mpv_paint(HWND hwnd)
{
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rect;
	RECT r2;
	int n,m,i;
	int c, color;
	int * fb;

	GetClientRect(hwnd, &rect);
	r2=rect;

	hdc=BeginPaint(hwnd, &ps);

	if(_font_normal==NULL)
		_mpv_init_fonts(hdc);

	SelectObject(hdc, _font_normal);

	SetTextColor(hdc, _inks[MP_COLOR_SELECTED]);
	SetBkColor(hdc, _papers[MP_COLOR_SELECTED]);

	r2.top+=_tab_height;
	r2.bottom=r2.top + _mpv_font_height;

	for(n=0;n < _mpv_y_size;n++)
	{
		r2.left=r2.right=rect.left;

		fb=&_mpv_fb[(n * _mpv_x_size)];

		for(m=0;m < _mpv_x_size;)
		{
			/* get first color */
			color=*fb & 0xff00;

			/* writes into _mpv_buffer while
			   color is the same */
			for(i=0;m<_mpv_x_size &&
				color==(*fb & 0xff00);
				i++,m++,fb++)
			{
				c=*fb & 0xff;
				_mpv_buffer[i]=c;
				r2.right+=_mpv_font_width;
			}

			_mpv_buffer[i]='\0';

			color>>=8;
			SetTextColor(hdc,_inks[color]);
			SetBkColor(hdc,_papers[color]);

			SelectObject(hdc, color==MP_COLOR_COMMENT ?
				_font_italic :
				color==MP_COLOR_LOCAL ? _font_underline :
				_font_normal);

			DrawText(hdc,_mpv_buffer,-1,&r2,DT_SINGLELINE|DT_NOPREFIX);

			r2.left=r2.right;
		}

		r2.top+=_mpv_font_height;
		r2.bottom+=_mpv_font_height;
	}

	EndPaint(hwnd, &ps);
}


static int _sys_to_clip(void)
{
	HGLOBAL hclp;
	char * ptr;

	OpenClipboard(NULL);
	hclp=GetClipboardData(CF_TEXT);
	CloseClipboard();

	if(hclp)
	{
		if((ptr=(char *)GlobalLock(hclp))!=NULL)
		{
			MP_SAVE_STATE();
			mp_lock_clipboard(1);

			/* transfer */
			while(*ptr)
			{
				if(*ptr!='\r')
					mp_put_char(_mp_clipboard, *ptr, 1);

				ptr++;
			}

			mp_lock_clipboard(0);
			MP_RESTORE_STATE();

			GlobalUnlock(hclp);
		}
	}

	return(1);
}


static void _clip_to_sys(void)
{
	int n,c;
	HGLOBAL hclp;
	char * ptr;

	if(_mp_clipboard==NULL) return;

	/* traverses clipboard counting chars */
	mp_move_bof(_mp_clipboard);
	for(n=0;(c=mp_get_char(_mp_clipboard))!='\0';n++)
	{
		if(c=='\n') n++;
	}

	mp_move_bof(_mp_clipboard);

	/* alloc and transfer */
	hclp=GlobalAlloc(GHND, n+1);
	ptr=(char *)GlobalLock(hclp);

	while(n > 0)
	{
		c=mp_get_char(_mp_clipboard);

		if(c=='\n')
		{
			*ptr++='\r';
			n--;
		}

		*ptr++=c;
		n--;
	}

	*ptr='\0';

	GlobalUnlock(hclp);

	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hclp);
	CloseClipboard();
}


/**
 * mpv_vkey - Converts Windows keys to key names.
 * @c: the key
 *
 * Converts Windows virtual keys to key names.
 */
static void _mpv_vkey(int c)
{
	char * ptr=NULL;
	static int _maximized=0;

	mpi_move_selecting=(GetKeyState(VK_SHIFT) & 0x8000);

	if(GetKeyState(VK_CONTROL) & 0x8000 ||
	   GetKeyState(VK_MENU) & 0x8000)
	{
		switch(c)
		{
		case VK_UP:	ptr="ctrl-cursor-up"; break;
		case VK_DOWN:	ptr="ctrl-cursor-down"; break;
		case VK_LEFT:	ptr="ctrl-cursor-left"; break;
		case VK_RIGHT:	ptr="ctrl-cursor-right"; break;
		case VK_PRIOR:	ptr="ctrl-page-up"; break;
		case VK_NEXT:	ptr="ctrl-page-down"; break;
		case VK_HOME:	ptr="ctrl-home"; break;
		case VK_END:	ptr="ctrl-end"; break;
		case VK_SPACE:	ptr="ctrl-space"; break;

		case VK_DIVIDE: ptr="ctrl-kp-divide"; break;
		case VK_MULTIPLY: ptr="ctrl-kp-multiply"; break;
		case VK_SUBTRACT: ptr="ctrl-kp-minus"; break;
		case VK_ADD:	ptr="ctrl-kp-plus"; break;
		case VK_RETURN: ptr="ctrl-enter"; break;

		case VK_F1:	ptr="ctrl-f1"; break;
		case VK_F2:	ptr="ctrl-f2"; break;
		case VK_F3:	ptr="ctrl-f3"; break;
		case VK_F4:	ptr="ctrl-f4"; break;
		case VK_F5:	ptr="ctrl-f5"; break;
		case VK_F6:	ptr="ctrl-f6"; break;
		case VK_F7:	ptr="ctrl-f7"; break;
		case VK_F8:	ptr="ctrl-f8"; break;
		case VK_F9:	ptr="ctrl-f9"; break;
		case VK_F10:	ptr="ctrl-f10"; break;
		case VK_F11:	ptr="ctrl-f11"; break;
		case VK_F12:
			SendMessage(hwnd, WM_SYSCOMMAND,
			_maximized ? SC_RESTORE : SC_MAXIMIZE, 0);

			_maximized^=1;

			break;
		}
	}
	else
	{
		switch(c)
		{
		case VK_UP:	ptr="cursor-up"; break;
		case VK_DOWN:	ptr="cursor-down"; break;
		case VK_LEFT:	ptr="cursor-left"; break;
		case VK_RIGHT:	ptr="cursor-right"; break;
		case VK_PRIOR:	ptr="page-up"; break;
		case VK_NEXT:	ptr="page-down"; break;
		case VK_HOME:	ptr="home"; break;
		case VK_END:	ptr="end"; break;

		case VK_TAB:	ptr="tab"; break;
		case VK_RETURN: ptr="enter"; break;
		case VK_BACK:	ptr="backspace"; break;
		case VK_DELETE: ptr="delete"; break;
		case VK_INSERT: ptr="insert"; break;

		case VK_DIVIDE: ptr="kp-divide"; break;
		case VK_MULTIPLY: ptr="kp-multiply"; break;
		case VK_SUBTRACT: ptr="kp-minus"; break;
		case VK_ADD:	ptr="kp-plus"; break;

		case VK_F1:	ptr="f1"; break;
		case VK_F2:	ptr="f2"; break;
		case VK_F3:	ptr="f3"; break;
		case VK_F4:	ptr="f4"; break;
		case VK_F5:	ptr="f5"; break;
		case VK_F6:	ptr="f6"; break;
		case VK_F7:	ptr="f7"; break;
		case VK_F8:	ptr="f8"; break;
		case VK_F9:	ptr="f9"; break;
		case VK_F10:	ptr="f10"; break;
		case VK_F11:	ptr="f11"; break;
		case VK_F12:	ptr="f12"; break;
		}
	}

	if(ptr != NULL) {
		mpi_process('\0', ptr, NULL);
		is_wm_keydown = 1;
	}
}


#define ctrl(ac) ((ac)&31)

/**
 * mpv_akey - Converts ascii keys to key names.
 * @k: the key
 *
 * Converts ascii keys to key names.
 */
static void _mpv_akey(int k)
{
	int c='\0';
	char * ptr=NULL;

	if (is_wm_keydown) return;
	switch(k)
	{
	case ctrl(' '): ptr="ctrl-space"; break;
	case ctrl('a'): ptr="ctrl-a"; break;
	case ctrl('b'): ptr="ctrl-b"; break;
	case ctrl('c'): ptr="ctrl-c"; break;
	case ctrl('d'): ptr="ctrl-d"; break;
	case ctrl('e'): ptr="ctrl-e"; break;
	case ctrl('f'): ptr="ctrl-f"; break;
	case ctrl('g'): ptr="ctrl-g"; break;
	case ctrl('h'): ptr="ctrl-h"; break;
	case ctrl('i'): ptr="ctrl-i"; break;
	case ctrl('j'): ptr="ctrl-j"; break;
	case ctrl('k'): ptr="ctrl-k"; break;
	case ctrl('l'): ptr="ctrl-l"; break;
	case ctrl('m'): ptr="ctrl-m"; break;
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
	default: if(k >=32 && k <=255) c=k; break;
	}

	if(c!='\0' || ptr!=NULL)
		mpi_process(c, ptr, NULL);
}


static void _mpv_amenu(int item)
{
	MENUITEMINFO mi;

	memset(&mi,'\0',sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.fMask=MIIM_DATA;

	if(GetMenuItemInfo(_mmenu,item,FALSE,&mi))
	{
		if(mi.dwItemData != 0)
			mpi_process('\0', NULL, (char *)mi.dwItemData);
	}
	else
	{
		char tmp[100];
		sprintf(tmp,"%d,%d",item,(int)GetLastError());
		MessageBox(NULL,tmp,tmp,MB_OK);
	}
}


#ifndef STDCALL
#define STDCALL __stdcall
#endif

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL			0x020A
#endif


static int load_dropped_files(HDROP hDrop, HWND hwnd)
{
   CHAR szFilename[1024];
   INT iNumFiles;

   (void) hwnd;

   iNumFiles = DragQueryFile (hDrop, 0xFFFFFFFF, NULL,sizeof(szFilename)-1);
   
   while(--iNumFiles>=0)
     {
	DragQueryFile(hDrop, iNumFiles, szFilename, sizeof(szFilename)-1);
	mpi_open(szFilename,0);
	mpv_title(NULL);
	/* redraw open file tabs, as status
	   has probably changed */
	mpv_filetabs();
	mpi_draw_all(_mp_active);
     }
   DragFinish(hDrop);
   
   return 0;
}

void sb_thumbscroll(int ev,int pos) {
  if (_mp_active) {
    if (_mp_active->y < pos) {
      pos += _mpv_y_size-1;
      if (pos >= _mp_active->lasty) {
	pos = _mp_active->lasty-1;
      }
    }
    mp_move_xy(_mp_active,0,pos);
    mpi_draw_all(_mp_active);
  }
}


/**
 * WndProc - Main window callback.
 * @hwnd: the window handler
 * @msg: the message
 * @wparam: the word param
 * @lparam: the long word param
 *
 * Main window callback.
 */
long STDCALL WndProc(HWND hwnd, UINT msg, UINT wparam, LONG lparam)
{
	int x,y;
	LPNMHDR p;
	mp_txt * t;
	char * ptr=NULL;

	switch(msg)
	{
	case WM_CREATE:
	  is_wm_keydown = 0;
	  DragAcceptFiles(hwnd,TRUE);
	  return 0;
	case WM_DROPFILES:
	  (void) load_dropped_files ((HANDLE) wparam, hwnd);
	  return 0;

	case WM_KEYUP:
	  is_wm_keydown = 0;
	  return 0;
	case WM_KEYDOWN:

		_mpv_vkey(wparam);

		return(0);

	case WM_CHAR:

		_mpv_akey(wparam);

		return(0);

	case WM_VSCROLL:

		switch(LOWORD(wparam))
		{
		case SB_PAGEUP: ptr="move-page-up"; break;
		case SB_PAGEDOWN: ptr="move-page-down"; break;
		case SB_LINEUP: ptr="move-up"; break;
		case SB_LINEDOWN: ptr="move-down"; break;
 		case SB_THUMBPOSITION:
 		case SB_THUMBTRACK:
 		  sb_thumbscroll(LOWORD(wparam),HIWORD(wparam));
 		  return(0);
		}

		if(ptr != NULL)
			mpi_process('\0', NULL, ptr);

		return(0);

	case WM_PAINT:
		_mpv_paint(hwnd);
		return(0);

	case WM_SIZE:

		if (IsIconic(hwnd)) return 0;
		if(_mpv_font_width && _mpv_font_height)
		{
			_mpv_x_size=(LOWORD(lparam)/_mpv_font_width)+1;
			_mpv_y_size=((HIWORD(lparam)-_tab_height)/_mpv_font_height)+1;

			/* rebuild framebuffer */
			if(_mpv_fb != NULL) free(_mpv_fb);
			_mpv_fb=(int *) malloc(_mpv_x_size * _mpv_y_size * sizeof(int));

			mpi_draw_all(_mp_active);
		}

		MoveWindow(hwtabs,0,0,LOWORD(lparam),_tab_height,TRUE);

		return(0);

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:

		x=LOWORD(lparam);
		y=HIWORD(lparam) - _tab_height;

		x/=_mpv_font_width;
		y/=_mpv_font_height;

		mp_move_xy(_mp_active,x,y+_mp_active->vy);
		mp_move_bol(_mp_active);
		mp_move_to_visual_column(_mp_active,x);

		mpi_draw_all(_mp_active);

		switch(msg)
		{
		case WM_LBUTTONDOWN: ptr="mouse-left-button"; break;
		case WM_RBUTTONDOWN: ptr="mouse-right-button"; break;
		case WM_MBUTTONDOWN: ptr="mouse-middle-button"; break;
		}

		if(ptr != NULL)
			mpi_process('\0', ptr, NULL);

		return(0);

	case WM_MOUSEWHEEL:

		if((int)wparam > 0)
			ptr="mouse-wheel-up";
		else
			ptr="mouse-wheel-down";

		if(ptr != NULL)
			mpi_process('\0', ptr, NULL);

		return(0);

	case WM_COMMAND:

		_mpv_amenu(LOWORD(wparam));

		return(0);

	case WM_CLOSE:
		mpi_shutdown();

		DestroyWindow(hwnd);
		return(0);

	case WM_DESTROY:
		PostQuitMessage(0);
		return(0);

	case WM_NOTIFY:
		p=(LPNMHDR)lparam;

		if(p->code==TCN_SELCHANGE)
		{
			y=TabCtrl_GetCurSel(hwtabs);

			for(t=_mp_txts,x=0;t!=NULL;t=t->next,x++)
			{
				if(x==y)
				{
					_mp_active=t;
					break;
				}
			}

			mpi_draw_all(_mp_active);
		}

		return(0);
	}

	if(_mpi_exit_requested)
	{
		PostMessage(hwnd,WM_CLOSE,0,0);
		_mpi_exit_requested=0;
	}

	return(DefWindowProc(hwnd,msg,wparam,lparam));
}


static void _scrollbar(int pos, int size, int max)
{
	SCROLLINFO si;

	si.cbSize=sizeof(si);
	si.fMask=SIF_ALL;
	si.nMin=1;
	si.nMax=max;
	si.nPage=size;
	si.nPos=pos;

	SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
}


static void _filetabs(int rebuild)
{
	mp_txt * t;
	int n;

	if(rebuild)
		TabCtrl_DeleteAllItems(hwtabs);

	for(t=_mp_txts,n=0;t!=NULL;t=t->next,n++)
	{
		if(rebuild)
		{
			TC_ITEM ti;
			char * ptr;

			ti.mask=TCIF_TEXT;

			if((ptr=strrchr(t->name,'\\')) == NULL)
				ptr=t->name;
			else
				ptr++;

			ti.pszText=ptr;

			TabCtrl_InsertItem(hwtabs,n,&ti);
		}

		if(_mp_active == t)
			TabCtrl_SetCurSel(hwtabs,n);
	}
}


static void _mpv_set_sys_lang(void)
{
	short s;
	char * ptr;

	/* do nothing if user forced a language */
	if(_mpi_lang[0] != '\0')
		return;

	s=GetSystemDefaultLangID() & 0x00ff;

	/* try only some 'famous' languages:
	   the complete world language database should
	   be implemented */
	switch(s)
	{
	case 0x01: ptr="ar"; break; /* arabic */
	case 0x02: ptr="bg"; break; /* bulgarian */
	case 0x03: ptr="ca"; break; /* catalan */
	case 0x04: ptr="zh"; break; /* chinese */
	case 0x05: ptr="cz"; break; /* czech */
	case 0x06: ptr="da"; break; /* danish */
	case 0x07: ptr="de"; break; /* german */
	case 0x08: ptr="el"; break; /* greek */
	case 0x09: ptr="en"; break; /* english */
	case 0x0a: ptr="es"; break; /* spanish */
	case 0x0b: ptr="fi"; break; /* finnish */
	case 0x0c: ptr="fr"; break; /* french */
	case 0x0d: ptr="he"; break; /* hebrew */
	case 0x0e: ptr="hu"; break; /* hungarian */
	case 0x0f: ptr="is"; break; /* icelandic */
	case 0x10: ptr="it"; break; /* italian */
	case 0x11: ptr="jp"; break; /* japanese */
	case 0x13: ptr="nl"; break; /* dutch */
	case 0x14: ptr="no"; break; /* norwegian */
	case 0x15: ptr="po"; break; /* polish */
	case 0x16: ptr="pt"; break; /* portuguese */
	case 0x1d: ptr="se"; break; /* swedish */
	default: ptr="en"; break;
	}

	strncpy(_mpi_lang, ptr, sizeof(_mpi_lang));
}


/**
 * mpv_create_colors - Creates the colors defined in the color database
 *
 * Creates the colors defined in the color database, probably
 * set from the configuration file.
 */
static void _mpv_create_colors(void)
{
	int n;
	int ink,paper;

	for(n=0;n < MP_COLOR_PRIVATE;n++)
	{
		if(mpi_monochrome)
		{
			ink=0x00000000;
			paper=0x00ffffff;

			if(n == MP_COLOR_SELECTED ||
			   n == MP_COLOR_CURSOR)
				mpc_color_desc[n].reverse=1;
		}
		else
		{
			ink=mpc_color_desc[n].ink_rgb;
			paper=mpc_color_desc[n].paper_rgb;

			if(ink==0xffffffff) ink=mpc_color_desc[0].ink_rgb;
			if(paper==0xffffffff) paper=mpc_color_desc[0].paper_rgb;
		}

		if(mpc_color_desc[n].reverse)
		{
			int i=paper;
			paper=ink; ink=i;
		}

		_inks[n]=((ink & 0x000000ff) << 16)|
			 ((ink & 0x0000ff00)) |
			 ((ink & 0x00ff0000) >> 16);
		_papers[n]=((paper & 0x000000ff) << 16)|
			 ((paper & 0x0000ff00)) |
			 ((paper & 0x00ff0000) >> 16);;
	}
}


static int _startup_1(void)
{
	_mpv_set_sys_lang();

	return(1);
}


static void _startup_2(void)
{
	WNDCLASS wc;
	RECT r;

	mp_log("Using Win32 driver.\n");

	InitCommonControls();

	/* register the window */
	wc.style=CS_HREDRAW|CS_VREDRAW;
	wc.lpfnWndProc=WndProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hInstance=hinst;
	wc.hIcon=LoadIcon(hinst,"MP_ICON");
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground=NULL;
	wc.lpszMenuName=NULL;
	wc.lpszClassName="minimumprofit3.x";

	if(!RegisterClass(&wc))
		return;

	/* create the window */
	hwnd=CreateWindow("minimumprofit3.x","mp " VERSION,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VSCROLL,
		CW_USEDEFAULT,CW_USEDEFAULT,
		CW_USEDEFAULT,CW_USEDEFAULT,
		NULL,_mmenu,hinst,NULL);

	if(hwnd==NULL)
		return;

	mpv_add_menu("");

	DrawMenuBar(hwnd);

	ShowWindow(hwnd,SW_SHOW);
	UpdateWindow(hwnd);

	GetClientRect(hwnd,&r);

	hwtabs=CreateWindow(WC_TABCONTROL, "tab",
		WS_CHILD | TCS_TABS | TCS_SINGLELINE | TCS_FOCUSNEVER,
		0, 0, r.right-r.left, _tab_height, hwnd, NULL, hinst, NULL);

	SendMessage(hwtabs, WM_SETFONT, 
		(WPARAM) GetStockObject(ANSI_VAR_FONT), 0);

	ShowWindow(hwtabs,SW_SHOW);
	UpdateWindow(hwtabs);

	_mpv_create_colors();
}


static void _main_loop(void)
{
	MSG msg;

	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


static void _shutdown(void)
{
	SendMessage(hwnd,WM_CLOSE,0,0);
}

static void _suspend(void)
{
	SendMessage(hwnd,WM_SYSCOMMAND,SC_MINIMIZE, 0);
	return;
}


HANDLE _makeNonInherit(HANDLE cHandle) {
  HANDLE nHandle;
  if (!DuplicateHandle(GetCurrentProcess(),cHandle,
  			GetCurrentProcess(),&nHandle,0,FALSE,
  			DUPLICATE_SAME_ACCESS)) {
    CloseHandle(cHandle);
    return NULL;
  }
  CloseHandle(cHandle);
  return nHandle;
}

int _spawnProc(char *cmdline,HANDLE inp,HANDLE out) {
  PROCESS_INFORMATION pi;
  STARTUPINFO si;
  BOOL ret;

  memset(&pi,0,sizeof(pi));
  memset(&si,0,sizeof(si));

  si.cb = sizeof(STARTUPINFO);
  si.hStdError = out;
  si.hStdOutput = out;
  si.hStdInput = inp;
  if (inp || out) si.dwFlags |= STARTF_USESTDHANDLES;

  ret = CreateProcess(NULL,cmdline,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi);

  if (out) CloseHandle(out);
  if (inp) CloseHandle(inp);

  if (ret) {  
    if (!(inp || out)) WaitForSingleObject(pi.hProcess,INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;    
  }
  return -1;
}

static int _syscmd(mp_txt *txt,char *cmd,char *mode) {
  if (txt && mode) {
    char tmp[1024];
    int c;
    HANDLE rdPipe, wrPipe;
    SECURITY_ATTRIBUTES saAttr;

    /* Create a pipe */
    memset(&saAttr,0,sizeof(saAttr));
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    if (!CreatePipe(&rdPipe,&wrPipe,&saAttr,0)) return -1;
    
    if (mode[0] == 'w') {
      mp_txt *ctxt;
      
      wrPipe = _makeNonInherit(wrPipe);
      if (!wrPipe) {
        CloseHandle(rdPipe);
        return -1;
      }
      if (_spawnProc(cmd,rdPipe,NULL)) {
        CloseHandle(wrPipe);
        return -1;
      }

      ctxt=mp_get_tmp_txt(txt);
      mp_move_bof(ctxt);

      mp_log("exec: write to '%s'\n",cmd);

      while((c=mp_get_str(ctxt,tmp,sizeof(tmp)-1,0)) > 0) 
        WriteFile(wrPipe,tmp,c,NULL,NULL);
        
      mp_end_tmp_txt();
      CloseHandle(wrPipe);
      return 0;
    } else {
      DWORD read;
      
      rdPipe = _makeNonInherit(rdPipe);
      if (!rdPipe) {
        CloseHandle(wrPipe);
        return -1;
      }
      if (_spawnProc(cmd,NULL,wrPipe)) {
        CloseHandle(rdPipe);
        return -1;
      }
      /* Read data */
      mp_log("exec: read from '%s'\n",cmd);
      while (ReadFile(rdPipe,tmp,sizeof(tmp)-1,&read,NULL) && read > 0) {
        tmp[read] = 0;
        mp_put_str(txt,tmp,1);
      }
      CloseHandle(rdPipe);
    }
    return 0;
  } else {
    int ret;
    mp_log("Executing %s\n",cmd);
    ret = _spawnProc(cmd,NULL,NULL);
    mp_log("Execution returned %d\n",ret);
    return ret;
  }
  return -1;
}



void cmd_2_argv(char * cmd)
{
	char d;
	unsigned long int n;
	HKEY hkey;

	_mpv_argc=1;
	d=' ';
	while(*cmd)
	{
		while(*cmd==d || *cmd==' ')
		{
			*cmd='\0';
			cmd++;
		}

		if(*cmd=='"' || *cmd=='\'')
		{
			d=*cmd;
			cmd++;
		}
		else
			d=' ';

		_argv_[_mpv_argc]=cmd;
		_mpv_argc++;

		while(*cmd && *cmd!=' ')
			cmd++;
	}

	_mpv_argv=_argv_;

	/* extract the home directory from the registry */
	if(RegOpenKeyEx(HKEY_CURRENT_USER,
		"Software\\Minimum Profit",0,
		KEY_QUERY_VALUE, &hkey)==ERROR_SUCCESS)
	{
		n=sizeof(_mpc_home);

		RegQueryValueEx(hkey,"Home",NULL,NULL,_mpc_home,&n);
	}
}


static void _set_var(char * var, char * value)
{
	if(strcmp(var,"win32_font_face")==0)
		strncpy(_mpv_font_face, value, sizeof(_mpv_font_face));
	else
	if(strcmp(var,"win32_font_size")==0)
		_mpv_font_size=atoi(value);
	else
	if(strcmp(var,"win32_help_file")==0)
		strncpy(_mpv_win32help, value, sizeof(_mpv_win32help));
}


static void _usage(void)
{
	char msg[2048];

	strncpy(msg,L("\
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
 -h|--help		This help screen\n\
\n\
 -hw|--hardware-cursor	Activates the use of hardware cursor\n\
 --mouse		Activate mouse usage for cursor positioning\n\
 -nt|--no-transparent	Disable transparent mode (eterm, aterm, etc.)\n\
			"),sizeof(msg));
	strcat(msg,"\n--mode: ");
	strcat(msg,mps_enumerate_modes());

	MessageBox(NULL,msg,"Minimum Profit" VERSION,MB_OK);
}


static int _args(int argc, char * argv[])
{
	return(0);
}


struct _mpv_driver _mpv_driver_win32=
{
	"win32", 0,
	_strcasecmp, _fopen, _glob, _goto, _char, _str,
	_cursor, _refresh, _title, _status_line, _add_menu,
	_add_menu_item, _check_menu, _menu, _alert, _confirm,
	_list, _readline, _sys_to_clip, _clip_to_sys, _about,
	_help, _zoom, _scrollbar, _filetabs, _set_var,
	_args, _usage, _main_loop,
	_startup_1, _startup_2, _shutdown, _suspend, _syscmd
};


#else

struct _mpv_driver _mpv_driver_win32=
{
	"win32", 0,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL
};

#endif /* CONFOPT_WIN32 */
