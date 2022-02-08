/*

    mp_core.h

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


/* text */
typedef struct txtctl mp_txt;

/* block */
typedef struct blkctl mp_blk;


/* block size */

#if !defined(BLK_SIZE)
#define BLK_SIZE 4096
#endif

/* a block */
struct blkctl
{
	mp_blk * next;		/* pointer to next */
	mp_blk * last;		/* pointer to previous */
	int size;		/* used size of block */
	char buf[BLK_SIZE];	/* the real text */
};

#define MP_TYPE_TEXT		0
#define MP_TYPE_READ_ONLY	1
#define MP_TYPE_LIST		2

/* a text */
struct txtctl
{
	char * name;		/* associated filename */
	mp_blk * first; 	/* first block */
	mp_blk * cursor;	/* block where the cursor is */
	int offset;		/* offset to cursor in block */
	int x;			/* x cursor position */
	int y;			/* y cursor position */
	int vx; 		/* x visible position */
	int vy; 		/* y visible position */
	int mbx;		/* x of beginning of selected block */
	int mby;		/* y of beginning of selected block */
	int mex;		/* x of end of selected block */
	int mey;		/* y of end of selected block */
	int hbx;		/* x of beginning of last search hit */
	int hby;		/* y of beginning of last search hit */
	int hex;		/* x of end of last search hit */
	int hey;		/* y of end of last search hit */
	int brx;		/* x of matching bracket (-1, none) */
	int bry;		/* y of matching bracket (-1, none) */ 
	int lasty;		/* number of lines */
	int synhi;		/* index to syntax hilight structure */
	int mod;		/* modified flag */
	int sys;		/* system flag */
	int type;		/* text type (MP_TYPE_*) */
	unsigned char * passwd;	/* encryption password (NULL: no encrypt) */
	mp_txt * next;		/* pointer to next */
};


/* the text list */
extern mp_txt * _mp_txts;

/* the active text */
extern mp_txt * _mp_active;

/* the clipboard */
extern mp_txt * _mp_clipboard;

/* the log */
extern mp_txt * _mp_log;

/* generation error macro (assert-like)  */
#define MP_ERROR(s) _mp_error(s,__LINE__)

#if !defined(DEFAULT_TAB_SIZE)
#define DEFAULT_TAB_SIZE 8
#endif

#if !defined(DEFAULT_WHEEL_SCROLL_ROWS)
#define DEFAULT_WHEEL_SCROLL_ROWS 4
#endif

#if !defined(AUTO_INDENT)
#define AUTO_INDENT 0
#endif

#define MP_REAL_TAB_SIZE(c) (_mp_tab_size - ((c) % _mp_tab_size))

/** externs **/

extern int _mp_tab_size;
extern int _mp_wheel_scroll_rows;
extern char _mp_separators[];
extern int _mp_word_wrap;
extern int _mp_save_tabs;
extern int _mp_auto_indent;
extern int _mp_indent;
extern int _mp_cr_lf;
extern int _mp_case_cmp;
extern int _mp_regex;
extern int _mp_txts_version;

#define MP_SAVE_STATE() mp_save_state(1)
#define MP_RESTORE_STATE() mp_save_state(-1)

/** prototypes **/

void mp_name_txt(mp_txt * txt, char * name);
mp_txt * mp_find_txt(char * name);
mp_txt * mp_create_sys_txt(char * name);
mp_txt * mp_create_txt(char * name);
mp_txt * mp_get_tmp_txt(mp_txt * otxt);
void mp_end_tmp_txt(void);
void mp_delete_sys_txt(mp_txt * txt);
void mp_delete_txt(mp_txt * txt);
void mp_show_sys_txt(mp_txt * txt);
void mp_empty_txt(mp_txt * txt);
void mp_next_txt(void);
void mp_prev_txt(void);

int mp_peek_char(mp_txt * txt);
void mp_modified(mp_txt * txt);
void mp_adjust(mp_txt * txt, int tx, int ty);
void mp_match_bracket(mp_txt * txt, int max);
void mp_set_notify(void (* func)(char *));
void mp_debug_hook(void);

int mp_is_sep(char c);
int mp_move_right(mp_txt * txt);
int mp_move_left(mp_txt * txt);
void mp_move_to_visual_column(mp_txt * txt, int r);
int mp_move_down(mp_txt * txt);
int mp_move_bol(mp_txt * txt);
int mp_move_eol(mp_txt * txt);
int mp_move_up(mp_txt * txt);
int mp_move_bof(mp_txt * txt);
int mp_move_eof(mp_txt * txt);
int mp_move_word_right(mp_txt * txt);
int mp_move_word_left(mp_txt * txt);
int mp_move_xy(mp_txt * txt, int x, int y);

int mp_get_char(mp_txt * txt);
int mp_get_str(mp_txt * txt, char * str, int maxsize, int delim);
void mp_get_word(mp_txt * txt, char * buf, int size);

void mp_save_state(int state);
int mp_insert_char(mp_txt * txt, int c);
int mp_insert_tab(mp_txt * txt);
int mp_insert_line(mp_txt * txt);
int mp_over_char(mp_txt * txt, char c);
int mp_put_char(mp_txt * txt, int c, int insert);
int mp_put_str(mp_txt * txt, char * str, int insert);
int mp_put_strf(mp_txt * txt, char * fmt, ...);
int mp_delete_char(mp_txt * txt);
int mp_delete_line(mp_txt * txt);
void mp_log(char * fmt, ...);

void mp_unmark(mp_txt * txt);
int mp_marked(mp_txt * txt);
void mp_mark(mp_txt * txt);
void mp_lock_clipboard(int lock);
int mp_copy_mark(mp_txt * txt);
int mp_paste_mark(mp_txt * txt);
int mp_delete_mark(mp_txt * txt);
void mp_reformat_paragraph(mp_txt * txt);

int mp_seek_plain(mp_txt * txt, char * str);
int mp_seek_regex(mp_txt * txt, char * str);
int mp_seek(mp_txt * txt, char * str);
int mp_mark_match(mp_txt * txt);
int mp_replace(mp_txt * txt, char * src, char * des);
void _mp_error(char * s, int line);

void mp_set_password(mp_txt * txt, char * passwd);
int mp_get_encryption_format(FILE * f);
int mp_load_file(mp_txt * txt, FILE * f);
int mp_save_file(mp_txt * txt, FILE * f);

int mp_startup(void);
void mp_shutdown(void);
void mp_sort(mp_txt *txt);

