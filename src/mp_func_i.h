// this was created with fngen.pl using mp_func.c as the input file
// unfortunately fngen.pl was deleted
//
// so further changes have to be done manually ...

struct _mpf_functions _node_mpf_zoom_out [] = { {
  NULL,NULL,
  "zoom-out",mpf_zoom_out,N_("Decrement font size"),1
} };
struct _mpf_functions _node_mpf_toggle_spellcheck [] = { {
  NULL,NULL,
  "toggle-spellcheck",mpf_toggle_spellcheck,N_("Mark spelling errors"),1
} };
struct _mpf_functions _node_mpf_unmark [] = { {
  NULL,_node_mpf_toggle_spellcheck,
  "unmark",mpf_unmark,N_("Unmark block"),1
} };
struct _mpf_functions _node_mpf_zoom_in [] = { {
  _node_mpf_zoom_out,_node_mpf_unmark,
  "zoom-in",mpf_zoom_in,N_("Increment font size"),1
} };
struct _mpf_functions _node_mpf_toggle_insert [] = { {
  NULL,NULL,
  "toggle-insert",mpf_toggle_insert,N_("Toggle insert/overwrite"),1
} };
struct _mpf_functions _node_mpf_toggle_regex [] = { {
  NULL,_node_mpf_toggle_insert,
  "toggle-regex",mpf_toggle_regex,N_("Use regular expressions"),1
} };
struct _mpf_functions _node_mpf_toggle_case [] = { {
  NULL,NULL,
  "toggle-case",mpf_toggle_case,N_("Case sensitive"),1
} };
struct _mpf_functions _node_mpf_toggle_column_80 [] = { {
  NULL,_node_mpf_toggle_case,
  "toggle-column-80",mpf_toggle_column_80,N_("Mark column #80"),1
} };
struct _mpf_functions _node_mpf_toggle_cr_lf [] = { {
  _node_mpf_toggle_regex,_node_mpf_toggle_column_80,
  "toggle-cr-lf",mpf_toggle_cr_lf,N_("Save LF as CR/LF"),1
} };
struct _mpf_functions _node_mpf_toggle_save_tabs [] = { {
  _node_mpf_zoom_in,_node_mpf_toggle_cr_lf,
  "toggle-save-tabs",mpf_toggle_save_tabs,N_("Save tabs"),1
} };
struct _mpf_functions _node_mpf_toggle_auto_indent [] = { {
  NULL,NULL,
  "toggle-auto-indent",mpf_toggle_auto_indent,N_("Automatic indentation"),1
} };
struct _mpf_functions _node_mpf_sort [] = { {
  NULL,NULL,
  "sort",mpf_sort,N_("Sort lines"),1
} };
struct _mpf_functions _node_mpf_suspend [] = { {
  NULL,_node_mpf_sort,
  "suspend",mpf_suspend,N_("Suspend application"),1
} };
struct _mpf_functions _node_mpf_sync [] = { {
  _node_mpf_toggle_auto_indent,_node_mpf_suspend,
  "sync",mpf_sync,N_("Save modified texts"),1
} };
struct _mpf_functions _node_mpf_set_word_wrap [] = { {
  NULL,NULL,
  "set-word-wrap",mpf_set_word_wrap,N_("Word wrap..."),1
} };
struct _mpf_functions _node_mpf_show_clipboard [] = { {
  NULL,_node_mpf_set_word_wrap,
  "show-clipboard",mpf_show_clipboard,N_("Show clipboard"),1
} };
struct _mpf_functions _node_mpf_seek_next [] = { {
  NULL,NULL,
  "seek-next",mpf_seek_next,N_("Search next"),1
} };
struct _mpf_functions _node_mpf_set_password [] = { {
  NULL,_node_mpf_seek_next,
  "set-password",mpf_set_password,N_("Password protect..."),1
} };
struct _mpf_functions _node_mpf_set_tab_size [] = { {
  _node_mpf_show_clipboard,_node_mpf_set_password,
  "set-tab-size",mpf_set_tab_size,N_("Tab size..."),1
} };
struct _mpf_functions _node_mpf_show_log [] = { {
  _node_mpf_sync,_node_mpf_set_tab_size,
  "show-log",mpf_show_log,N_("Show log"),1
} };
struct _mpf_functions _node_mpf_toggle_break_hardlinks [] = { {
  _node_mpf_toggle_save_tabs,_node_mpf_show_log,
  "toggle-break-hardlinks",mpf_toggle_break_hardlinks,N_("Break hardlinks on write"),1
} };
struct _mpf_functions _node_mpf_save_as [] = { {
  NULL,NULL,
  "save-as",mpf_save_as,N_("Save as..."),1
} };
struct _mpf_functions _node_mpf_replace [] = { {
  NULL,NULL,
  "replace",mpf_replace,N_("Replace..."),1
} };
struct _mpf_functions _node_mpf_replace_all [] = { {
  NULL,_node_mpf_replace,
  "replace-all",mpf_replace_all,N_("Replace in all..."),1
} };
struct _mpf_functions _node_mpf_save [] = { {
  _node_mpf_save_as,_node_mpf_replace_all,
  "save",mpf_save,N_("Save..."),1
} };
struct _mpf_functions _node_mpf_record_macro [] = { {
  NULL,NULL,
  "record-macro",mpf_record_macro,N_("Record macro"),1
} };
struct _mpf_functions _node_mpf_reformat_paragraph [] = { {
  NULL,_node_mpf_record_macro,
  "reformat-paragraph",mpf_reformat_paragraph,N_("Reformat paragraph with word wrapping"),1
} };
struct _mpf_functions _node_mpf_paste [] = { {
  NULL,NULL,
  "paste",mpf_paste,N_("Paste block"),1
} };
struct _mpf_functions _node_mpf_play_macro [] = { {
  NULL,_node_mpf_paste,
  "play-macro",mpf_play_macro,N_("Play macro"),1
} };
struct _mpf_functions _node_mpf_prev [] = { {
  _node_mpf_reformat_paragraph,_node_mpf_play_macro,
  "prev",mpf_prev,N_("Previous"),1
} };
struct _mpf_functions _node_mpf_reopen [] = { {
  _node_mpf_save,_node_mpf_prev,
  "reopen",mpf_reopen,N_("Reopen..."),1
} };
struct _mpf_functions _node_mpf_next [] = { {
  NULL,NULL,
  "next",mpf_next,N_("Next"),1
} };
struct _mpf_functions _node_mpf_open [] = { {
  NULL,_node_mpf_next,
  "open",mpf_open,N_("Open..."),1
} };
struct _mpf_functions _node_mpf_move_word_left [] = { {
  NULL,NULL,
  "move-word-left",mpf_move_word_left,N_("Word left"),1
} };
struct _mpf_functions _node_mpf_move_word_right [] = { {
  NULL,_node_mpf_move_word_left,
  "move-word-right",mpf_move_word_right,N_("Word right"),1
} };
struct _mpf_functions _node_mpf_new [] = { {
  _node_mpf_open,_node_mpf_move_word_right,
  "new",mpf_new,N_("New"),1
} };
struct _mpf_functions _node_mpf_move_up [] = { {
  NULL,NULL,
  "move-up",mpf_move_up,N_("Line up"),1
} };
struct _mpf_functions _node_mpf_move_wheel_down [] = { {
  NULL,_node_mpf_move_up,
  "move-wheel-down",mpf_move_wheel_down,N_("Page down"),1
} };
struct _mpf_functions _node_mpf_move_page_up [] = { {
  NULL,NULL,
  "move-page-up",mpf_move_page_up,N_("Page up"),1
} };
struct _mpf_functions _node_mpf_move_prev [] = { {
  NULL,_node_mpf_move_page_up,
  "move-prev",mpf_move_prev,N_("Move to previous instance of current char"),1
} };
struct _mpf_functions _node_mpf_move_right [] = { {
  _node_mpf_move_wheel_down,_node_mpf_move_prev,
  "move-right",mpf_move_right,N_("Character right"),1
} };
struct _mpf_functions _node_mpf_move_wheel_up [] = { {
  _node_mpf_new,_node_mpf_move_right,
  "move-wheel-up",mpf_move_wheel_up,N_("Page up"),1
} };
struct _mpf_functions _node_mpf_open_under_cursor [] = { {
  _node_mpf_reopen,_node_mpf_move_wheel_up,
  "open-under-cursor",mpf_open_under_cursor,N_("Open file under cursor"),1
} };
struct _mpf_functions _node_mpf_seek [] = { {
  _node_mpf_toggle_break_hardlinks,_node_mpf_open_under_cursor,
  "seek",mpf_seek,N_("Search text..."),1
} };
struct _mpf_functions _node_mpf_move_next [] = { {
  NULL,NULL,
  "move-next",mpf_move_next,N_("Move to next instance of current char"),1
} };
struct _mpf_functions _node_mpf_move_eof [] = { {
  NULL,NULL,
  "move-eof",mpf_move_eof,N_("End of document"),1
} };
struct _mpf_functions _node_mpf_move_eol [] = { {
  NULL,_node_mpf_move_eof,
  "move-eol",mpf_move_eol,N_("End of line"),1
} };
struct _mpf_functions _node_mpf_move_left [] = { {
  _node_mpf_move_next,_node_mpf_move_eol,
  "move-left",mpf_move_left,N_("Character left"),1
} };
struct _mpf_functions _node_mpf_move_bof [] = { {
  NULL,NULL,
  "move-bof",mpf_move_bof,N_("Beginning of document"),1
} };
struct _mpf_functions _node_mpf_move_bol [] = { {
  NULL,_node_mpf_move_bof,
  "move-bol",mpf_move_bol,N_("Beginning of line"),1
} };
struct _mpf_functions _node_mpf_mark_match [] = { {
  NULL,NULL,
  "mark-match",mpf_mark_match,N_("Select last succesful search"),1
} };
struct _mpf_functions _node_mpf_menu [] = { {
  NULL,_node_mpf_mark_match,
  "menu",mpf_menu,N_("Menu"),1
} };
struct _mpf_functions _node_mpf_mouse_position [] = { {
  _node_mpf_move_bol,_node_mpf_menu,
  "mouse-position",mpf_mouse_position,N_("Position cursor with mouse"),1
} };
struct _mpf_functions _node_mpf_move_down [] = { {
  _node_mpf_move_left,_node_mpf_mouse_position,
  "move-down",mpf_move_down,N_("Line down"),1
} };
struct _mpf_functions _node_mpf_key_help [] = { {
  NULL,NULL,
  "key-help",mpf_key_help,N_("Help on keys"),1
} };
struct _mpf_functions _node_mpf_join_line_above [] = { {
  NULL,NULL,
  "join-line-above",mpf_join_line_above,N_("Join current line to one above"),1
} };
struct _mpf_functions _node_mpf_join_line_below [] = { {
  NULL,_node_mpf_join_line_above,
  "join-line-below",mpf_join_line_below,N_("Join current line to one below"),1
} };
struct _mpf_functions _node_mpf_jump_matching_bracket [] = { {
  _node_mpf_key_help,_node_mpf_join_line_below,
  "jump-matching-bracket",mpf_jump_matching_bracket,N_("Jump to matching bracket"),1
} };
struct _mpf_functions _node_mpf_insert_line_below [] = { {
  NULL,NULL,
  "insert-line-below",mpf_insert_line_below,N_("Insert new line below cursor"),1
} };
struct _mpf_functions _node_mpf_insert_tab [] = { {
  NULL,_node_mpf_insert_line_below,
  "insert-tab",mpf_insert_tab,N_("Insert tab"),1
} };
struct _mpf_functions _node_mpf_help [] = { {
  NULL,NULL,
  "help",mpf_help,N_("Help for word under cursor"),1
} };
struct _mpf_functions _node_mpf_insert_line [] = { {
  NULL,_node_mpf_help,
  "insert-line",mpf_insert_line,N_("Insert line"),1
} };
struct _mpf_functions _node_mpf_insert_line_above [] = { {
  _node_mpf_insert_tab,_node_mpf_insert_line,
  "insert-line-above",mpf_insert_line_above,N_("Insert new line above cursor"),1
} };
struct _mpf_functions _node_mpf_insert_template [] = { {
  _node_mpf_jump_matching_bracket,_node_mpf_insert_line_above,
  "insert-template",mpf_insert_template,N_("Insert template..."),1
} };
struct _mpf_functions _node_mpf_mark [] = { {
  _node_mpf_move_down,_node_mpf_insert_template,
  "mark",mpf_mark,N_("Mark beginning/end of block"),1
} };
struct _mpf_functions _node_mpf_goto [] = { {
  NULL,NULL,
  "goto",mpf_goto,N_("Go to line..."),1
} };
struct _mpf_functions _node_mpf_find_tag [] = { {
  NULL,NULL,
  "find-tag",mpf_find_tag,N_("Search tag..."),1
} };
struct _mpf_functions _node_mpf_flip_letter_case [] = { {
  NULL,_node_mpf_find_tag,
  "flip-letter-case",mpf_flip_letter_case,N_("Flip letter case if A-Z or a-z"),1
} };
struct _mpf_functions _node_mpf_flip_word_case [] = { {
  _node_mpf_goto,_node_mpf_flip_letter_case,
  "flip-word-case",mpf_flip_word_case,N_("Flip word case"),1
} };
struct _mpf_functions _node_mpf_exec_command [] = { {
  NULL,NULL,
  "exec-command",mpf_exec_command,N_("Run system command..."),1
} };
struct _mpf_functions _node_mpf_exec_function [] = { {
  NULL,_node_mpf_exec_command,
  "exec-function",mpf_exec_function,N_("Execute editor function..."),1
} };
struct _mpf_functions _node_mpf_document_list [] = { {
  NULL,NULL,
  "document-list",mpf_document_list,N_("Document list"),1
} };
struct _mpf_functions _node_mpf_edit_config_file [] = { {
  NULL,_node_mpf_document_list,
  "edit-config-file",mpf_edit_config_file,N_("Edit configuration file"),1
} };
struct _mpf_functions _node_mpf_edit_templates_file [] = { {
  _node_mpf_exec_function,_node_mpf_edit_config_file,
  "edit-templates-file",mpf_edit_templates_file,N_("Edit templates file"),1
} };
struct _mpf_functions _node_mpf_exit [] = { {
  _node_mpf_flip_word_case,_node_mpf_edit_templates_file,
  "exit",mpf_exit,N_("Exit"),1
} };
struct _mpf_functions _node_mpf_delete_word [] = { {
  NULL,NULL,
  "delete-word",mpf_delete_word,N_("Delete whole word"),1
} };
struct _mpf_functions _node_mpf_delete_word_begin [] = { {
  NULL,_node_mpf_delete_word,
  "delete-word-begin",mpf_delete_word_begin,N_("Delete to the beginning of word"),1
} };
struct _mpf_functions _node_mpf_delete_left [] = { {
  NULL,NULL,
  "delete-left",mpf_delete_left,N_("Delete char to the left of cursor"),1
} };
struct _mpf_functions _node_mpf_delete_line [] = { {
  NULL,_node_mpf_delete_left,
  "delete-line",mpf_delete_line,N_("Delete line"),1
} };
struct _mpf_functions _node_mpf_delete_whitespace [] = { {
  _node_mpf_delete_word_begin,_node_mpf_delete_line,
  "delete-whitespace",mpf_delete_whitespace,N_("Delete all whitespace after cursor"),1
} };
struct _mpf_functions _node_mpf_copy [] = { {
  NULL,NULL,
  "copy",mpf_copy,N_("Copy block"),1
} };
struct _mpf_functions _node_mpf_cut [] = { {
  NULL,_node_mpf_copy,
  "cut",mpf_cut,N_("Cut block"),1
} };
struct _mpf_functions _node_mpf_about [] = { {
  NULL,NULL,
  "about",mpf_about,N_("About..."),1
} };
struct _mpf_functions _node_mpf_close [] = { {
  NULL,_node_mpf_about,
  "close",mpf_close,N_("Close"),1
} };
struct _mpf_functions _node_mpf_completion [] = { {
  _node_mpf_cut,_node_mpf_close,
  "completion",mpf_completion,N_("Complete tag..."),1
} };
struct _mpf_functions _node_mpf_delete [] = { {
  _node_mpf_delete_whitespace,_node_mpf_completion,
  "delete",mpf_delete,N_("Delete char over cursor"),1
} };
struct _mpf_functions _node_mpf_delete_word_end [] = { {
  _node_mpf_exit,_node_mpf_delete,
  "delete-word-end",mpf_delete_word_end,N_("Delete to the end of word"),1
} };
struct _mpf_functions _node_mpf_grep [] = { {
  _node_mpf_mark,_node_mpf_delete_word_end,
  "grep",mpf_grep,N_("Grep (find inside) files..."),1
} };
struct _mpf_functions _node_mpf_move_page_down [] = { {
  _node_mpf_seek,_node_mpf_grep,
  "move-page-down",mpf_move_page_down,N_("Page down"),1
} };
struct _mpf_functions *mpf_functions = _node_mpf_move_page_down;
