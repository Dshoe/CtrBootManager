#ifdef ARM9

#include "arm9/source/common.h"
#include "arm9/source/hid.h"
#include "arm9/source/fatfs/ff.h"
#include "memory.h"
#else
#include <3ds.h>
#include <sys/dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#endif

#include "draw.h"
#include "picker.h"
#include "utility.h"
#include "config.h"
#include "menu.h"

#ifdef ARM9
#define    DT_DIR         4
#define    DT_BLK         6
struct dirent {
    ino_t d_ino;
    unsigned char d_type;
    char d_name[512];
};
#endif

#define MAX_LINE 12

static picker_s *picker;

int alphasort(const void *p, const void *q) {
    const file_s *a = p;
    const file_s *b = q;

    if ((a->isDir && b->isDir)
        || (!a->isDir && !b->isDir))
        return strcasecmp(a->name, b->name);
    else
        return a->isDir ? -1 : 1;
}

void parse_file(struct dirent *file) {

    if (!strcmp(file->d_name, ".") || !strcmp(file->d_name, ".."))
        return;
    if (file->d_type != DT_DIR) {
        const char *ext = get_filename_ext(file->d_name);
        if (strcasecmp(ext, "bin") != 0
            && strcasecmp(ext, "dat") != 0
        #ifdef ARM9
            && strcasecmp(ext, "firm") != 0)
        #else
            && strcasecmp(ext, "3dsx") != 0)
        #endif
            return;
    }

    // file name
    strncpy(picker->files[picker->file_count].name, file->d_name, 128);

    //  build absolute path
    if (end_with(picker->now_path, '/')) {
        snprintf(picker->files[picker->file_count].path,
                 256, "%s%s", picker->now_path, file->d_name);
    } else {
        snprintf(picker->files[picker->file_count].path,
                 256, "%s/%s", picker->now_path, file->d_name);
    }

    // dir vs file
    if (file->d_type != DT_DIR) {
        picker->files[picker->file_count].isDir = false;
#ifndef ARM9
        // file size
        struct stat st;
        stat(picker->files[picker->file_count].path, &st);
        picker->files[picker->file_count].size = (u64) st.st_size;
#endif
    } else {
        picker->files[picker->file_count].isDir = true;
    }
    picker->file_count++;
}

void get_dir(const char *path) {

#ifdef ARM9
    DIR fd;
    FILINFO fno;
    struct dirent *file = (struct dirent *) PTR_PICKER_FILE;
    memset(file, 0, sizeof(struct dirent));
#else
    DIR *fd;
    struct dirent *file;
#endif
    char new_path[256];
    strncpy(new_path, path, 256);

#ifdef ARM9
    if (f_opendir(&fd, path) != FR_OK) {
#else
    if ((fd = opendir(new_path)) == NULL) {
#endif
        return;
    }
    strncpy(picker->now_path, new_path, 256);
    picker->file_count = 0;
    picker->file_index = 0;

#ifdef ARM9
    while (f_readdir(&fd, &fno) == FR_OK) {
        if (!fno.fname || fno.fname[0] == 0) {
            break;
        }
        strncpy(file->d_name, fno.fname, 256);
        file->d_type = fno.fattrib & AM_DIR ? DT_DIR : DT_BLK;
#else
        while ((file = readdir(fd))) {
#endif
        parse_file(file);
    }

    if (picker->file_count > 1) {
        qsort(picker->files, (size_t) picker->file_count,
              sizeof(*picker->files), alphasort);
    }

#ifdef ARM9
    f_closedir(&fd);
#else
    closedir(fd);
#endif
}

static void draw() {

    drawBg();
    drawTitle("*** Select a file ***");

    int i, y = 0;
    int page = picker->file_index / MAX_LINE;
    for (i = page * MAX_LINE; i < page * MAX_LINE + MAX_LINE; i++) {
        if (i >= picker->file_count)
            break;

        drawItemN(i == picker->file_index, 47, 16 * y, picker->files[i].name);
        if (i == picker->file_index && !picker->files[i].isDir) {
            drawInfo("Press (A) to launch\nPress (X) to add to boot menu");

        }
        y++;
    }

    swapFrameBuffers();
}

void pick_file(file_s *picked, const char *path) {

#ifdef ARM9
    picker = (picker_s *) PTR_PICKER;
#else
    picker = malloc(sizeof(picker_s));
#endif
    memset(picker, 0, sizeof(picker_s));
    picker->file_count = 0;
    picker->file_index = 0;

    get_dir(path);

    // key repeat timer
    static time_t t_start = 0, t_end = 0, t_elapsed = 0;

    while (aptMainLoop()) {

        draw();

        hidScanInput();
        u32 kHeld = hidKeysHeld();
        u32 kDown = hidKeysDown();

#ifndef ARM9
        if (hidKeysUp()) {
            time(&t_start); // reset held timer
        }
#endif
        if (kDown & KEY_DOWN || kDown & KEY_RIGHT) {
            picker->file_index += (kDown & KEY_DOWN) ? 1 : MAX_LINE;
            if (picker->file_index >= picker->file_count)
                picker->file_index = (kDown & KEY_DOWN || picker->file_index == picker->file_count - 1 + MAX_LINE) ? 0 : (picker->file_count - 1);
#ifndef ARM9
            time(&t_start);
#endif
        } else if (kHeld & KEY_DOWN || kHeld & KEY_RIGHT) {
#ifndef ARM9
            time(&t_end);
#endif
            t_elapsed = t_end - t_start;
            if (t_elapsed > 0) {
                picker->file_index += (kDown & KEY_DOWN) ? 1 : MAX_LINE;
                if (picker->file_index >= picker->file_count)
                    picker->file_index = (kDown & KEY_DOWN || picker->file_index == picker->file_count - 1 + MAX_LINE) ? 0 : (picker->file_count - 1);
                svcSleep(100);
            }
        }

        if (kDown & KEY_UP || kDown & KEY_LEFT) {
            picker->file_index -= (kDown & KEY_UP) ? 1 : MAX_LINE;
            if (picker->file_index < 0)
                picker->file_index = (kDown & KEY_UP || picker->file_index == -MAX_LINE) ? (picker->file_count - 1) : 0;
#ifndef ARM9
            time(&t_start);
#endif
        } else if (kHeld & KEY_UP || kDown & KEY_LEFT) {
#ifndef ARM9
            time(&t_end);
#endif
            t_elapsed = t_end - t_start;
            if (t_elapsed > 0) {
                picker->file_index -= (kDown & KEY_UP) ? 1 : MAX_LINE;
                if (picker->file_index < 0)
                    picker->file_index = (kDown & KEY_UP || picker->file_index == -MAX_LINE) ? (picker->file_count - 1) : 0;
                svcSleep(100);
            }
        }

        if (kDown & KEY_A) {
            if (picker->file_count > 0) {
                int index = picker->file_index;
                if (!picker->files[index].isDir) {
                    if (confirm(0, "Launch \"%s\" ?", picker->files[index].name)) {
                        strncpy(picked->name, picker->files[index].name, 128);
                        strncpy(picked->path, picker->files[index].path, 256);
                        picked->isDir = picker->files[index].isDir;
                        picked->size = picker->files[index].size;
                        break;
                    }
                }
                else {
                    get_dir(picker->files[index].path);
                }
            }
        } else if (kDown & KEY_X) {
            int index = picker->file_index;
            if (!picker->files[index].isDir) {
                const char *ext = get_filename_ext(picker->files[index].name);
                int noOffsetReq = 1;
                if (strcasecmp(ext, "bin") == 0 || (noOffsetReq = strcasecmp(ext, "dat")) == 0
#ifdef ARM9
                    || strcasecmp(ext, "firm") == 0) {
#else
                    || strcasecmp(ext, "3dsx") == 0) {
#endif
                    if (confirm(3, "Add entry to boot menu: \"%s\" ?", picker->files[index].name)) {
                        if (config->count > config->maxCount - 1) {
                            debug("Maximum entries reached (%i)\n", config->maxCount);
                        } else if (configAddEntry(picker->files[index].name, picker->files[index].path, noOffsetReq?0:0x12000) == 0) {
                            debug("Added entry: %s\n", picker->files[index].name);
                        } else {
                            debug("Error adding entry: %s\n", picker->files[index].name);
                        }
                    }
                }
            }
        }
        else if (kDown & KEY_B) {
            // exit if we can't go back
            if (strlen(picker->now_path) <= 1)
                break;

            // remove slash if needed
            if (end_with(picker->now_path, '/'))
                picker->now_path[strlen(picker->now_path) - 1] = '\0';

            // build path
            char *slash = strrchr(picker->now_path, '/');
            if (slash == NULL)
                break;
            int len = (int) (slash - picker->now_path);
            picker->now_path[len] = '\0';

            // enter new dir
            get_dir(picker->now_path);
        }
    }
#ifndef ARM9
    free(picker);
#endif
}
