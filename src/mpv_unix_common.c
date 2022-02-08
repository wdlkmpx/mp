/*

    mpv_unix_common.c

    Code common to Unix versions (curses / X).

    mp - Programmer Text Editor

    Copyright (C) 1991/2005 Angel Ortega <angel@triptico.com>

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

#if !defined(_WIN32)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mp_core.h"
#include "mp_video.h"
#include "mp_iface.h"
#include "mp_lang.h"
#include "mp_synhi.h"
#include "mp_conf.h"
#include <glob.h>

/*******************
	Data
********************/

/*******************
	Code
********************/

int _unix_strcasecmp(char * s1, char * s2)
{
	return(strcasecmp(s1,s2));
}


/**
 * _mpv_strip_cwd - Strips current working directory
 * @buf: buffer containing a file name
 * @size: size of the buffer
 *
 * If the file name contained in @buf begins with
 * the current working directory, strips it (effectively
 * converting an absolute pathname to relative). If
 * the file name path is different from the
 * current working directory, nothing is done.
 */
void _mpv_strip_cwd(char * buf, int size)
{
	char tmp[1024];
	int l;
	char * ptr;

	if(getcwd(tmp,sizeof(tmp))==NULL)
		return;

	l=strlen(tmp);
	if(strncmp(tmp,buf,l)==0)
	{
		ptr=&buf[l];
		if(*ptr=='/') ptr++;

		strncpy(tmp,ptr,sizeof(tmp));
		strncpy(buf,tmp,size);
	}
}


FILE * _unix_fopen(char * file, char * mode)
{
	FILE * f=NULL;

	if(strcmp(file, "-") == 0)
	{
		int fd;

		/* if file is -, duplicate stdin or stdout
		   depending on the output mode */
		fd=dup(mode[0] == 'w' ? 1 : 0);
		f=fdopen(fd, mode);
	}
	else
	if(_mpi_break_hardlinks && !strcmp(mode, "w"))
	{
		struct stat st;

		if(stat(file, &st) != -1)
		{
			/* file exists and metainformation
			   is stored in st; unlink now */
			unlink(file);

			if((f=fopen(file, mode)) != NULL)
			{
				/* tries to restore original owner
				   and file permissions */
				fchmod(fileno(f), st.st_mode);
				fchown(fileno(f), st.st_uid, st.st_gid);
			}
		}
	}

	/* if not already open, do as usual */
	if(f == NULL)
		f=fopen(file, mode);

	return(f);
}


mp_txt * _unix_glob(char * spec)
{
	mp_txt * txt=NULL;
	struct stat s;

	int n;
	glob_t globbuf;
	char * ptr;

	if(spec[0]=='\0') spec="*";

	globbuf.gl_offs=1;
	if(glob(spec,GLOB_MARK,NULL,&globbuf))
		return(NULL);

	txt=mp_create_sys_txt("<glob>");

	MP_SAVE_STATE();

	for(n=0;globbuf.gl_pathv[n]!=NULL;n++)
	{
		ptr=globbuf.gl_pathv[n];

		if(stat(ptr,&s)==-1) continue;
		if(s.st_mode & S_IFDIR) continue;

		mp_put_str(txt,ptr,1);
		mp_put_char(txt,'\n',1);
	}

	globfree(&globbuf);

	mp_move_left(txt);
	mp_delete_char(txt);
	mp_move_bof(txt);

	MP_RESTORE_STATE();

	return(txt);
}

int _unix_popen(mp_txt *txt,char *cmd,char *mode)
{
	FILE *f;
	int c;
	mp_txt *ctxt;
	int ret=-1;
  
	if (mode[0] == 'w')
	{
		ctxt=mp_get_tmp_txt(txt);
		mp_move_bof(ctxt);

		mp_log("exec: write to '%s'\n",cmd);

		if((f=popen(cmd,"w")) != NULL)
		{
			while((c=mp_get_char(ctxt)) != '\0')
				fputc(c,f);

			if(pclose(f) != -1)
				ret=0;
		}

		mp_end_tmp_txt();
	}
	else
	{
		mp_log("exec: read from '%s'\n",cmd);

		if((f=popen(cmd,"r")) != NULL)
		{
			while((c=fgetc(f)) != EOF)
				mp_put_char(txt,c,1);

			if(pclose(f) != -1)
				ret=0;
		}
	}

	return(ret);
}


int _unix_help(char * term, int synhi)
{
	char tmp[1024];
	FILE * f;
	mp_txt * txt;
	char ** ptr;

	sprintf(tmp,_("<help about '%s'>"),term);

	if(synhi==0 || (txt=mp_create_txt(tmp))==NULL)
		return(0);

	if((ptr=_mps_synhi[synhi - 1].helpers)!=NULL)
	{
		for(;*ptr!=NULL;ptr++)
		{
			snprintf(tmp,sizeof(tmp),*ptr,term);

			if((f=popen(tmp,"r"))!=NULL)
			{
				mp_load_file(txt,f);

				if(!pclose(f))
					break;
			}
		}
	}

	if(ptr==NULL || *ptr==NULL)
	{
		mp_delete_txt(txt);
		mpv_alert(_("No help for '%s'"),term);
		return(0);
	}

	mps_auto_synhi(txt);
	txt->type=MP_TYPE_READ_ONLY;
	txt->mod=0;

	return(1);
}

#endif
