#ifndef ROFI_MODE_H
#define ROFI_MODE_H

/**
 * @defgroup MODE Mode
 *
 * The 'object' that makes a mode in rofi.
 * @{
 */
typedef struct _Mode   Mode;

/**
 * Enum used to sum the possible states of ROFI.
 */
typedef enum
{
    /** Exit. */
    MODE_EXIT       = 1000,
    /** Skip to the next cycle-able dialog. */
    NEXT_DIALOG     = 1001,
    /** Reload current DIALOG */
    RELOAD_DIALOG   = 1002,
    /** Previous dialog */
    PREVIOUS_DIALOG = 1003,
    /** Filemanager next */
    FILEMANAGER_RELOAD_DIALOG = 1004
} ModeMode;

/**
 * State returned by the rofi window.
 */
typedef enum
{
    /** Entry is selected. */
    MENU_OK           = 0x00010000,
    /** User canceled the operation. (e.g. pressed escape) */
    MENU_CANCEL       = 0x00020000,
    /** User requested a mode switch */
    MENU_NEXT         = 0x00040000,
    /** Custom (non-matched) input was entered. */
    MENU_CUSTOM_INPUT = 0x00080000,
    /** User wanted to delete entry from history. */
    MENU_ENTRY_DELETE = 0x00100000,
    /** User wants to jump to another switcher. */
    MENU_QUICK_SWITCH = 0x00200000,
    /** Go to the previous menu. */
    MENU_PREVIOUS     = 0x00400000,
    /** Modifiers */
    MENU_SHIFT        = 0x10000000,
    /** Mask */
    MENU_LOWER_MASK   = 0x0000FFFF
} MenuReturn;

/**
 * @param mode The mode to initialize
 *
 * Initialize mode
 *
 * @returns FALSE if there was a failure, TRUE if successful
 */
int mode_init ( Mode *mode );

/**
 * @param mode The mode to destroy
 *
 * Destroy the mode
 */
void mode_destroy ( Mode *mode );

/**
 * @param mode The mode to query
 *
 * Get the number of entries in the mode.
 *
 * @returns an unsigned in with the number of entries.
 */
unsigned int mode_get_num_entries ( const Mode *sw );

/**
 * @param mode The mode to query
 * @param selected_line The entry to query
 * @param state The state of the entry [out]
 * @param get_entry If the should be returned.
 *
 * Returns the string as it should be displayed for the entry and the state of how it should be displayed.
 *
 * @returns allocated new string and state when get_entry is TRUE otherwise just the state.
 */
char * mode_get_display_value ( const Mode *mode, unsigned int selected_line, int *state, int get_entry );

/**
 * @param mode The mode to query
 * @param selected_line The entry to query
 *
 * Return a string that can be used for completion. It has should have no markup.
 *
 * @returns allocated string.
 */
char * mode_get_completion ( const Mode *mode, unsigned int selected_line );

/**
 * @param mode The mode to query
 * @param selected_line The entry to query
 *
 * Check if the entry has non-ascii characters.
 *
 * @returns TRUE when selected line has non-ascii characters.
 */
int mode_is_not_ascii ( const Mode *mode, unsigned int selected_line );

/**
 * @param mode The mode to query
 * @param mretv The menu return value.
 * @param input Pointer to the user input string.
 * @param selected_line the line selected by the user.
 *
 * Acts on the user interaction.
 *
 * @returns the next #ModeMode.
 */
ModeMode mode_result ( Mode *mode, int menu_retv, char **input, unsigned int selected_line );

/**
 * @param mode The mode to query
 * @param tokens The set of tokens to match against
 * @param not_ascii If the entry is pure-ascii
 * @param case_sensitive If the entry should be matched case sensitive
 * @param selected_line The index of the entry to match
 *
 * Match entry against the set of tokens.
 *
 * @returns TRUE if matches
 */
int mode_token_match ( const Mode *mode, char **tokens, int not_ascii, int case_sensitive, unsigned int selected_line );

/**
 * @param mode The mode to query
 *
 * Get the name of the mode.
 *
 * @returns the name of the mode.
 */
const char * mode_get_name ( const Mode *mode );

/**
 * @param mode The mode to query
 * @param key The KeySym to match
 * @param state The Modmask to match
 *
 * Match keybinding of mode.
 *
 * return TRUE when matching, FALSE otherwise
 */
int mode_check_keybinding ( const Mode *mode, KeySym key, unsigned int modstate );

/**
 * @param mode The mode to query
 *
 * Free the resources allocated for this mode.
 */
void mode_free ( Mode **mode );

/**
 * @param mode The mode to query
 *
 * Setup the keybinding for this mode.
 */
void mode_setup_keybinding ( Mode *mode );

/**
 * @param mode The mode to query
 * @param display The X Server display handle.
 *
 * Grab the key on the display.
 * This first parses the key string and if successful asks X11 for a grab.
 *
 * return FALSE when key could not be grabbed.
 */
int mode_grab_key ( Mode *mode, Display *display );

/**
 * @param mode The mode to query
 * @param display The X Server display handle.
 *
 * Releases previously grabbed key.
 */
void mode_ungrab_key ( Mode *mode, Display *display );

/**
 * @param mode The mode to query
 *
 * Print the current keybing for this mode to stdout.
 */
void mode_print_keybindings ( const Mode *mode );

/**
 * @param mode The mode to query
 *
 * Helper functions for mode.
 * Get the private data object.
 */
void *mode_get_private_data ( const Mode *mode );

/**
 * @param mode The mode to query
 *
 * Helper functions for mode.
 * Set the private data object.
 */
void mode_set_private_data ( Mode *mode, void *pd );

void mode_set_update_prompt ( Mode *mode, char *update_prompt );
char * mode_get_update_prompt ( const Mode *mode );
void mode_set_path ( Mode *mode, char *path );
char * mode_get_path( const Mode *mode );


const char *mode_get_display_name ( const Mode *mode );

void mode_set_config ( Mode *mode );
/*@}*/
#endif
