REM Minimum Profit - lccwin32 build

echo /* automatically created by buildlcc.bat - do not modify */ > config.h
type VERSION >> config.h
echo /* */ >> config.h

echo #define MPOPT_WIN32 1 >> config.h
echo #define HAVE_STRING_H 1 >> config.h
echo #define REGEX 1 >> config.h
echo #define MPOPT_INCLUDED_REGEX 1 >> config.h

lcc -O mp_core.c
lcc -O mp_synhi.c
lcc -O mp_iface.c
lcc -O gnu_regex.c
lcc -O mp_lang_m.c
lcc -O mp_lang.c
lcc -O mp_conf.c
lcc -O mp_func.c
lcc -O mp_video.c
lcc -O mp_tags.c
lcc -O mpv_unix_common.c
lcc -O mpv_curses.c
lcc -O mpv_gtk.c
lcc -O mpv_win32.c
lrc mp_res.rc
lcclnk -s *.obj mp_res.res -subsystem windows -o wmp.exe
