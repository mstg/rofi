/*
 * rofi
 *
 * MIT/X11 License
 * Copyright 2013-2016 Qball Cow <qball@gmpclient.org>
 * Copyright 2016 Mustafa Gezen <mustiigezen@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/**
 * \ingroup FILEMANAGERMode
 * @{
 */
 #include <config.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include <X11/X.h>

 #include <unistd.h>
 #include <limits.h>
 #include <signal.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <dirent.h>
 #include <strings.h>
 #include <string.h>
 #include <errno.h>

 #include "rofi.h"
 #include "settings.h"
 #include "helper.h"
 #include "history.h"
 #include "dialogs/filemanager.h"

 #include "mode-private.h"

 typedef struct _FileManagerPrivateData
 {
     /** list of files/dirs in dir */
     char         **filed_list;
     /** Length of the #filed_list. */
     unsigned int filed_list_length;
 } FileManagerPrivateData;

 static int sort_func ( const void *a, const void *b, void *data __attribute__( ( unused ) ) )
 {
     const char *astr = *( const char * const * ) a;
     const char *bstr = *( const char * const * ) b;

     if ( astr == NULL && bstr == NULL ) {
         return 0;
     }
     else if ( astr == NULL ) {
         return 1;
     }
     else if ( bstr == NULL ) {
         return -1;
     }
     return g_ascii_strcasecmp ( astr, bstr );
 }

 static void set_prompt( Mode *sw, char *prompt ) {
     sw->update_prompt = prompt;
 }

 static char ** get_dir_struct ( Mode *sw, unsigned int *length )
 {
     GError *error = NULL;
     char **retv = g_malloc(1 + sizeof(char*));

     gsize l = 0;
     gchar *dir = NULL;

     if ( config.filemanager_start_path != NULL && config.filemanager_start_path[0] != '\0' ) {
         dir = g_locale_to_utf8(config.filemanager_start_path, -1, NULL, &l, &error);
     }

     if (!dir) dir = g_locale_to_utf8 (  g_get_home_dir (), -1, NULL, &l, &error );
     if ( error != NULL ) {
         fprintf ( stderr, "Failed to convert directory name to UTF-8: %s\n", error->message );
         g_clear_error ( &error );
         g_free ( dir );
         return NULL;
     }
     set_prompt(sw, g_strdup(dir));
     sw->filemanager_start_path  = g_strdup(dir);

     #define add_to_list(entry) \
         retv = g_realloc ( retv, ( ( *length ) + 2 ) * sizeof ( char* ) ); \
         retv[(*length)] = entry; \
         retv[(*length) + 1] = NULL; \
         (*length)++; \

     DIR *_dir = opendir ( dir );
     if (!_dir) {
         add_to_list("Path specified does not exist");
         return retv;
     } else {
         add_to_list("Exit");
         struct dirent *dent;
         while ( ( dent = readdir ( _dir ) ) != NULL ) {
             add_to_list(g_strdup(dent->d_name));

         }
         closedir ( _dir );
     }

     g_qsort_with_data(&retv[1], (*length), sizeof(char*), sort_func, NULL);

     g_free(dir);
     return retv;
 }

 static inline int execsh ( const char *cmd, int run_in_term )
 {
     int  retv   = TRUE;
     char **args = NULL;
     int  argc   = 0;
     if ( run_in_term ) {
         helper_parse_setup ( config.run_shell_command, &args, &argc, "{cmd}", cmd, NULL );
     }
     else {
         helper_parse_setup ( config.run_command, &args, &argc, "{cmd}", cmd, NULL );
     }
     GError *error = NULL;
     g_spawn_async ( NULL, args, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &error );
     if ( error != NULL ) {
         char *msg = g_strdup_printf ( "Failed to execute: '%s'\nError: '%s'", cmd, error->message );
         rofi_view_error_dialog ( msg, FALSE  );
         g_free ( msg );
         // print error.
         g_error_free ( error );
         retv = FALSE;
     }

     // Free the args list.
     g_strfreev ( args );
     return retv;
 }

 static int filemanager_mode_init ( Mode *sw )
 {
     FileManagerPrivateData *pd = g_malloc0 ( sizeof ( *pd ) );
     sw->private_data = (void *) pd;
     pd->filed_list = get_dir_struct ( sw, &( pd->filed_list_length ) );

     return TRUE;
 }

 static unsigned int filemanager_mode_get_num_entries ( const Mode *sw )
 {
     const FileManagerPrivateData *rmpd = (const FileManagerPrivateData *) sw->private_data;
     return rmpd->filed_list_length;
 }

 static int is_regular_file(const char *path)
{
    struct stat path_stat;
    lstat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

 static ModeMode filemanager_mode_result ( Mode *sw, int mretv, char **input, unsigned int selected_line )
 {
     if (selected_line == 0) {
         return MODE_EXIT;
     } else if (selected_line == -1) {
         return MODE_EXIT;
     }

     const FileManagerPrivateData *rmpd = (const FileManagerPrivateData *) sw->private_data;
     gchar *fpath = g_build_filename ( sw->filemanager_start_path, rmpd->filed_list[selected_line], NULL );

     if (is_regular_file(fpath)) {
         char rcm[255];
         sprintf(rcm, "xdg-open \"%s\"", fpath);
         execsh(g_strdup(rcm), TRUE);
         return MODE_EXIT;
     }

     char rpath[PATH_MAX];
     realpath(fpath, rpath);
     fpath = g_strdup(rpath);

     sw->filemanager_start_path = rpath;
     return FILEMANAGER_RELOAD_DIALOG;
 }

 static int filemanager_token_match ( const Mode *sw, char **tokens, int not_ascii, int case_sensitive, unsigned int index )
 {
     const FileManagerPrivateData *rmpd = (const FileManagerPrivateData *) sw->private_data;
     return token_match ( tokens, rmpd->filed_list[index], not_ascii, case_sensitive );
 }

 static void filemanager_mode_destroy ( Mode *sw )
 {
     FileManagerPrivateData *rmpd = (FileManagerPrivateData *) sw->private_data;
     if ( rmpd != NULL ) {
         g_free ( rmpd->filed_list );
         g_free ( rmpd );
         sw->private_data = NULL;
     }
 }

static int filemanager_is_not_ascii ( const Mode *sw, unsigned int index )
{
    return 0;
}

static char *_get_display_value ( const Mode *sw, unsigned int selected_line, G_GNUC_UNUSED int *state, int get_entry )
{
    const FileManagerPrivateData *rmpd = (const FileManagerPrivateData *) sw->private_data;
    return get_entry ? g_strdup ( rmpd->filed_list[selected_line] ) : NULL;
}

 Mode filemanager_mode =
 {
     .name               = "filemanager",
     ._init              = filemanager_mode_init,
     ._get_num_entries   = filemanager_mode_get_num_entries,
     ._result            = filemanager_mode_result,
     ._destroy           = filemanager_mode_destroy,
     ._is_not_ascii      = filemanager_is_not_ascii,
      ._token_match  = filemanager_token_match,
     ._get_display_value = _get_display_value,
     .private_data       = NULL,
     .free               = NULL
 };
 /*@}*/
