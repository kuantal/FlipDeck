/*
 * ╔══════════════════════════════════════════════════════════════╗
 *  FlipDeck — SD Card Storage Implementation
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include "flip_deck_storage.h"
#include "flip_deck_pages.h"
#include <storage/storage.h>
#include <string.h>

/* Default texts; overwritten if macros.txt exists on SD card */
char macro_buf[MACRO_COUNT][MACRO_LEN] = {
    "Hello,\nHow are you?\n",
    "Best regards,\n",
    "admin@example.com",
    "+1 555 000 0000",
    "https://github.com/",
};

/* Default labels; overwritten if macros.txt exists on SD card */
char macro_label_buf[MACRO_COUNT][LABEL_LEN] = {
    "Hello",
    "SignOff",
    "Email",
    "Phone",
    "GitHub",
};

static char s_macro_path[64];

static void build_macro_path(uint8_t pidx) {
    snprintf(s_macro_path, sizeof(s_macro_path),
             "/ext/apps_data/flip_deck/macros_%u.txt", (unsigned)pidx);
}

void macros_load_sd(uint8_t pidx) {
    build_macro_path(pidx);
    Storage* store = furi_record_open(RECORD_STORAGE);
    File* f = storage_file_alloc(store);
    if(storage_file_open(f, s_macro_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        uint8_t mx = 0, ci = 0;
        char line[MACRO_LEN];
        char c;
        while(mx < MACRO_COUNT && storage_file_read(f, &c, 1) > 0) {
            if(c == '\r') continue;
            if(c == '\n' || ci >= MACRO_LEN - 1) {
                line[ci] = '\0';
                if(ci > 0) {
                    /* Parse format: label|text (if no pipe, entire line is text) */
                    char* pipe = strchr(line, '|');
                    if(pipe != NULL) {
                        /* Split label and text */
                        *pipe = '\0';  /* terminate label */
                        strncpy(macro_label_buf[mx], line, LABEL_LEN - 1);
                        macro_label_buf[mx][LABEL_LEN - 1] = '\0';
                        strncpy(macro_buf[mx], pipe + 1, MACRO_LEN - 1);
                        macro_buf[mx][MACRO_LEN - 1] = '\0';
                    } else {
                        /* No pipe - entire line is text, keep default label */
                        strncpy(macro_buf[mx], line, MACRO_LEN - 1);
                        macro_buf[mx][MACRO_LEN - 1] = '\0';
                    }
                    mx++; ci = 0;
                }
            } else { line[ci++] = c; }
        }
        if(ci > 0 && mx < MACRO_COUNT) {
            line[ci] = '\0';
            char* pipe = strchr(line, '|');
            if(pipe != NULL) {
                *pipe = '\0';
                strncpy(macro_label_buf[mx], line, LABEL_LEN - 1);
                macro_label_buf[mx][LABEL_LEN - 1] = '\0';
                strncpy(macro_buf[mx], pipe + 1, MACRO_LEN - 1);
                macro_buf[mx][MACRO_LEN - 1] = '\0';
            } else {
                strncpy(macro_buf[mx], line, MACRO_LEN - 1);
            }
        }
        storage_file_close(f);
    }
    storage_file_free(f);
    furi_record_close(RECORD_STORAGE);
    
    /* Update macro page pointers */
    pages_update_macro_pointers();
}

void macro_save_sd(uint8_t pidx) {
    build_macro_path(pidx);
    Storage* store = furi_record_open(RECORD_STORAGE);
    storage_simply_mkdir(store, DATA_DIR);
    File* f = storage_file_alloc(store);
    if(storage_file_open(f, s_macro_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        for(uint8_t i = 0; i < MACRO_COUNT; i++) {
            /* Save format: label|text */
            storage_file_write(f, macro_label_buf[i], strlen(macro_label_buf[i]));
            storage_file_write(f, "|", 1);
            storage_file_write(f, macro_buf[i], strlen(macro_buf[i]));
            storage_file_write(f, "\n", 1);
        }
        storage_file_close(f);
    }
    storage_file_free(f);
    furi_record_close(RECORD_STORAGE);
}

void state_save(const AppState* st) {
    Storage* store = furi_record_open(RECORD_STORAGE);
    storage_simply_mkdir(store, DATA_DIR);
    File* f = storage_file_alloc(store);
    if(storage_file_open(f, STATE_FILE, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        uint8_t buf[2] = {st->page_idx, st->profile_idx};
        storage_file_write(f, buf, 2);
        storage_file_close(f);
    }
    storage_file_free(f);
    furi_record_close(RECORD_STORAGE);
}

void state_load(AppState* st) {
    Storage* store = furi_record_open(RECORD_STORAGE);
    File* f = storage_file_alloc(store);
    if(storage_file_open(f, STATE_FILE, FSAM_READ, FSOM_OPEN_EXISTING)) {
        uint8_t buf[2] = {0, 0};
        if(storage_file_read(f, buf, 2) > 0) {
            if(buf[0] < pages_get_count())  st->page_idx    = buf[0];
            if(buf[1] < PROFILE_COUNT)      st->profile_idx = buf[1];
        }
        storage_file_close(f);
    }
    storage_file_free(f);
    furi_record_close(RECORD_STORAGE);
}
