/*

    mp_func.h

    Functions (bindable to keys)

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

int mpf_call_func_by_funcname(char *funcname,char **args);
char * mpf_get_funcname_by_keyname(char * keyname);

char * mpf_get_desc_by_funcname(char * func_name);
char * mpf_get_keyname_by_funcname(char * funcname);
int mpf_bind_key(char * keyname, char * funcname);
int * mpf_toggle_function_value(char * funcname);
void mpf_get_funcnames(mp_txt * txt);

char **mpf_makeargs(char *str);

void mpf_desc_user_fn(char *func,char *desc);
void mpf_new_user_fn(char *func,char *desc,char **args);
