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

struct tag_index
{
	char * tag;	/* tag */
	char * file;	/* file where target lives */
	char * target;	/* target (string to be searched) */
};

extern char _mpt_ctags_cmd[128];

struct tag_index * mpt_seek_tag(char * tag);
struct tag_index * mpt_select_tag(char * partial_tag);
void mpt_open_tag(char * partial_tag);

void mpt_startup(void);
void mpt_shutdown(void);
