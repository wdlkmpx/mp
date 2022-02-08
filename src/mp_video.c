/*

    mp_video.c

    Video driver entry point.

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

#include "mp_core.h"
#include "mp_video.h"
#include "mp_iface.h"
#include "mp_func.h"
#include "mp_lang.h"
#include "mp_synhi.h"
#include "mp_conf.h"
#include "mp_tags.h"

/*******************
	Data
********************/

/* x and y size, in characters */
int _mpv_x_size;
int _mpv_y_size;

int _mpv_argc;
char ** _mpv_argv;

extern struct _mpv_driver _mpv_driver_curses;

struct _mpv_driver * _mpv_drivers[] = {
	&_mpv_driver_curses,
	NULL
};

static struct _mpv_driver * drv=NULL;

/* driver is in text mode (also used to force) */
int _mpv_text=0;

/* selected interface driver (also used to force) */
char _mpv_interface[64]="";


/*****************
       Code
******************/

/**
 * mpv_strcasecmp - Case ignoring string compare
 * @s1: first string
 * @s2: second string
 *
 * Case ignoring string compare. System dependent
 * (strcasecmp in Unix, stricmp in Win32)
 */
int mpv_strcasecmp(char * s1, char * s2)
{
	return(drv->_strcasecmp(s1, s2));
}


FILE * mpv_fopen(char * file, char * mode)
{
	return(drv->_fopen(file, mode));
}


/**
 * mpv_glob - Returns a result of a file globbing
 * @spec: the file pattern
 *
 * Executes a file globbing using @spec as a pattern
 * and returns a mp_txt containing the files matching
 * the glob.
 */
mp_txt * mpv_glob(char * spec)
{
	return(drv->_glob(spec));
}


/**
 * mpv_goto - Positions the cursor to start drawing.
 * @x: the x position
 * @y: the y position
 *
 * Positions the cursor to start drawing the document window.
 */
void mpv_goto(int x, int y)
{
	drv->_goto(x, y);
}


/**
 * mpv_char - Draws a char with color in the document window.
 * @c: the char
 * @color: the color (one of MP_COLOR_ constants)
 *
 * Draws a char with color in current cursor position of the
 * document window.
 */
void mpv_char(int c, int color)
{
	drv->_char(c, color);
}


/**
 * mpv_str - Draws a string with color in the document window.
 * @str: the string
 * @color: the color (one of MP_COLOR_ constants)
 *
 * Draws a string, calling mpv_char() for each of their chars.
 */
void mpv_str(char * str, int color)
{
	drv->_str(str, color);
}


/**
 * mpv_cursor - Positions the hardware cursor.
 * @x: the real x position
 * @y: the real y position
 *
 * Sets the hardware cursor to x, y position.
 */
void mpv_cursor(int x, int y)
{
	drv->_cursor(x, y);
}


/**
 * mpv_refresh - Refresh the screen.
 *
 * Orders the underlying system to redraw the screen.
 */
void mpv_refresh(void)
{
	drv->_refresh();
}


/**
 * mpv_title - Sets the string to be drawn as title
 * @str: the string
 *
 * Sets the string to be drawn as title of the document window.
 */
void mpv_title(char * str)
{
	drv->_title(str);
}


/**
 * mpv_status_line - Sets the string to be drawn as status line
 * @str: the string
 *
 * Sets the string to be drawn as the status line.
 */
void mpv_status_line(char * str)
{
	drv->_status_line(str);
}


/**
 * mpv_add_menu - Creates a new menu bar.
 * @label: the label
 *
 * Creates a new menu bar.
 */
void mpv_add_menu(char * label)
{
	drv->_add_menu(label);
}


/**
 * mpv_add_menu_item - Adds a menu item to current menu bar.
 * @f: the name of the function to invoke
 *
 * Adds a menu item to the current menu bar.
 */
void mpv_add_menu_item(char * f)
{
	drv->_add_menu_item(f);
}


/**
 * mpv_check_menu - Checks or unchecks a menu item
 * @f: function name which menu item should be (un)checked
 * @t: boolean value
 *
 * Checks or unchecks the menu item that executes @funcname.
 */
void mpv_check_menu(char * f, int t)
{
	drv->_check_menu(f, t);
}


/**
 * mpv_menu - Manages the menu.
 *
 * Manages the menu (drawing it, if applicable).
 * Returns the key associated to the activated menu item,
 * or 0 if none was. 
 */
int mpv_menu(void)
{
	return drv->_menu();
}


/**
 * mpv_alert - Alerts the user.
 * @m1: the alert message
 * @m2: a possible second message.
 *
 * Alerts the user by showing the message and
 * asking for validation.
 */
void mpv_alert(char * m1, char * m2)
{
	drv->_alert(m1, m2);
}


/**
 * mpv_confirm - Asks for confirmation.
 * @prompt: the question to ask
 *
 * Asks for confirmation.
 * Returns 1 if choosen 'yes'.
 */
int mpv_confirm(char * prompt)
{
	return drv->_confirm(prompt);
}


/**
 * mpv_list - Manages a selection list
 * @title: the title or caption of the list
 * @txt: the text containing the list to show
 * @pos: element to be set by default
 *
 * Shows a unique element selection list to the user.
 * The list must be previously built into txt.
 * Returns the selected element (the number of the
 * line where the element is) or -1 on cancellation.
 */
int mpv_list(char * title, mp_txt * txt, int pos)
{
	return drv->_list(title, txt, pos);
}


/**
 * mpv_readline - Ask for user input.
 * @t: the type of input asked (one of MPR_ constants)
 * @p: the prompt
 * @d: the default value
 *
 * Asks for user input. The expected data type is
 * described in type.
 * Returns a pointer to a static buffer with the
 * data entered by the user, or NULL if user cancelled.
 */
char * mpv_readline(int t, char * p, char * d)
{
	return drv->_readline(t, p, d);
}


/**
 * mpv_system_to_clipboard - Copies from the system clipboard
 *
 * Copies the clipboard's content from the underlying system
 * to Minimum Profit's internal one. A returning value of 0
 * means the actual pasting is already done (or done from a
 * system callback), so it won't be necessary to do it.
 */
int mpv_system_to_clipboard(void)
{
	return drv->_sys_to_clip();
}


/**
 * mpv_clipboard_to_system - Copies to the system clipboard
 *
 * Copies the clipboard's content from Minimum Profit to
 * the underlying system's one.
 */
void mpv_clipboard_to_system(void)
{
	drv->_clip_to_sys();
}


/**
 * mpv_about - Shows the 'About Minimum Profit...' information.
 *
 * Shows a text or dialog box showing the information
 * about the program, version and such.
 */
void mpv_about(void)
{
	drv->_about();
}


/**
 * mpv_help - Shows the available help for a term
 * @term: the term
 * @synhi: the syntax highlighter
 *
 * Shows the available help for the term. The argument
 * filetype is a string describing the file type,
 * taken from the syntax hilighter (this allows to
 * retrieve the help from different sources for C
 * or Perl code, for example).
 * Returns 0 on error or if no help is available
 * for this term.
 */
int mpv_help(char * term, int synhi)
{
	return drv->_help(term, synhi);
}


/**
 * mpv_zoom - Zooms the document window.
 * @inc: the increment (+1 or -1)
 *
 * Increases / decreases font size of the document window,
 * if applicable.
 */
int mpv_zoom(int inc)
{
	return drv->_zoom(inc);
}


/**
 * mpv_scrollbar - Draws / updates the scrollbar.
 * @pos: current position
 * @size: vertical size
 * @max: maximum value
 *
 * Draws / updates the scrollbar. @pos is current position
 * (the line where the cursor is), @size is the number of
 * lines of the document window and @max is the maximum
 * value (the total number of lines in current text).
 */
void mpv_scrollbar(int pos, int size, int max)
{
	drv->_scrollbar(pos, size, max);
}


/**
 * mpv_filetabs - Draws the tab set containing the file names
 *
 * Draws the names of the opened files in the form of a tab set.
 */
void mpv_filetabs(void)
{
	static int _last_version=-1;

	drv->_filetabs(_mp_txts_version != _last_version);

	_last_version=_mp_txts_version;
}


/**
 * mpv_set_variable - Sets a driver-dependent variable
 * @var: the name of the variable to be set
 * @value: the new value
 *
 * Sets a variable from the configuration file that is
 * driver-specific.
 */
void mpv_set_variable(char * var, char * value)
{
	drv->_set_var(var, value);
}


/**
 * mpv_args - Driver command line argument processing
 * @argc: argument count
 * @argv: the arguments
 *
 * Driver dependent argument processing, after video driver is
 * initialised. Called from within mpi_args_2().
 */
int mpv_args(int argc, char * argv[])
{
	return drv->_args(argc, argv);
}


/**
 * mpv_mainloop - Main message processing loop.
 *
 * Main message processing loop.
 */
void mpv_main_loop(void)
{
	drv->_main_loop();
}


/**
 * mpv_usage - Prints the usage help.
 *
 * Prints the usage help.
 */
void mpv_usage(void)
{
	drv->_usage();
}


/**
 * mpv_startup_1 - Finds a usable video driver
 *
 * Finds a usable video driver, pre-initializing it. If no usable
 * driver is found, 0 is returned, or nonzero otherwise.
 */
int mpv_startup_1(void)
{
	int n;

	/* finds a usable driver */
	for(n=0;_mpv_drivers[n] != NULL;n++)
	{
		/* if text mode is forced, ignore non-text modes */
		if(_mpv_text && ! _mpv_drivers[n]->is_text)
			continue;

		/* if interface driver is forced, test */
		if(_mpv_interface[0] != 0 &&
		   strcmp(_mpv_interface, _mpv_drivers[n]->name) != 0)
			continue;

		if(_mpv_drivers[n]->_startup_1 != NULL)
		{
			if(_mpv_drivers[n]->_startup_1())
			{
				/* select this one */
				drv=_mpv_drivers[n];

				/* store information */
				_mpv_text=drv->is_text;
				strncpy(_mpv_interface, drv->name,
					sizeof(_mpv_interface));

				break;
			}
		}
	}

	return(drv == NULL ? 0 : 1);
}

void mpv_startup_2(void) { drv->_startup_2(); }


/**
 * mpv_shutdown - Shuts down the system dependent driver.
 *
 * Shuts down the system dependent driver.
 */
void mpv_shutdown(void)
{
	drv->_shutdown();
}

/**
 * mpv_suspend - Suspend the system dependant driver
 *
 * Suspend the execution of the application.  For windowed interfaces,
 * this will put the application in minimized mode.
 */
void mpv_suspend(void)
{
	drv->_suspend();
}

/**
 * mpv_syscmd - System dependant command execution
 *
 * Executes external commands.  This is system dependant.
 * It will take care of I/O redirection as needed as well as setting
 * up the necessary TTY modes.
 *
 * Returns 0 on success, -1 on error.
 */
int mpv_syscmd(mp_txt *txt,char *cmd,char *mode) {
	return drv->_syscmd(txt,cmd,mode);
}


/******* MAIN *******/

int main(int argc, char * argv[])
{
	int r;

	_mpv_argc=argc;
	_mpv_argv=argv;

	/* text engine startup */
	mp_startup();

	/* first stage arg processing */
	mpi_args_1(_mpv_argc, _mpv_argv);

	/* finds a usable input / output driver */
	if(!mpv_startup_1())
	{
		printf("%s\n",_("No usable video driver found."));
		return(1);
	}

	/* start synhi */
	mps_startup();

	/* processes configuration files */
	mpc_startup();

	/* starts interface */
	mpi_startup();

	/* input / output driver specific args */
	mpv_args(_mpv_argc, _mpv_argv);

	/* really starts the i/o driver */
	mpv_startup_2();

	/* second stage arg processing */
	if((r=mpi_args_2(_mpv_argc, _mpv_argv)) == -1)
	{
		mpv_usage();
		return(0);
	}

	/* start tags interface */
	mpt_startup();

	/* create empty text if no file is open */
	if(_mp_active==NULL)
	{
		mp_create_txt(_("<unnamed>"));
		mps_auto_synhi(_mp_active);
	}

	mpv_title(NULL);
	mpi_draw_all(_mp_active);
	mpv_filetabs();

	if(r == -2)
		mpv_alert(_("Bad mode."),"");

	/* main loop */
	mpv_main_loop();

	/* close everything */
	mpi_shutdown();
	mpv_shutdown();
	mp_shutdown();

	return(0);
}

