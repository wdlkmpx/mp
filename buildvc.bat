@echo off
REM Visual C++ build
REM ----------------
REM Run VCVARS32.BAT first (comes with your installation)
REM
REM Tested with:
REM - Visual C++ 6.0
REM - Visual C++ 7.1

echo Minimum Profit - Visual Studio build

echo /* DO NOT MODIFY */ > config.h
type VERSION >> config.h
echo #define CONFOPT_WIN32 1 >> config.h
echo #define HAVE_STRING_H 1 >> config.h
echo #define REGEX 1 >> config.h
echo #define CONFOPT_INCLUDED_REGEX 1 >> config.h
echo. >> config.h
echo #define snprintf _snprintf >> config.h
echo #define strcasecmp _stricmp >> config.h
echo #define STDCALL __stdcall >> config.h

set CFLAGS=/c /O2 /GA /D__WIN32__
set LFLAGS=user32.lib gdi32.lib advapi32.lib comdlg32.lib comctl32.lib shell32.lib /NOLOGO /SUBSYSTEM:WINDOWS /OPT:NOWIN98 /OUT:wmp.exe

cl %CFLAGS% mp_core.c 2> NUL
cl %CFLAGS% mp_synhi.c 2> NUL
cl %CFLAGS% mp_iface.c 2> NUL
cl %CFLAGS% gnu_regex.c 2> NUL
cl %CFLAGS% mp_lang_m.c 2> NUL
cl %CFLAGS% mp_lang.c 2> NUL
cl %CFLAGS% mp_conf.c 2> NUL
cl %CFLAGS% mp_func.c 2> NUL
cl %CFLAGS% mp_video.c 2> NUL
cl %CFLAGS% mp_tags.c 2> NUL
cl %CFLAGS% mp_wordp.c 2> NUL
cl %CFLAGS% mpv_unix_common.c 2> NUL
cl %CFLAGS% mpv_curses.c 2> NUL
cl %CFLAGS% mpv_gtk.c 2> NUL
cl %CFLAGS% mpv_win32.c 2> NUL
rc mp_res.rc
link mp_core.obj mp_synhi.obj mp_iface.obj gnu_regex.obj mp_lang.obj mp_lang_m.obj mp_conf.obj mp_func.obj mp_video.obj mp_tags.obj mp_wordp.obj mpv_unix_common.obj mpv_curses.obj mpv_gtk.obj mpv_win32.obj mp_res.res %LFLAGS%
