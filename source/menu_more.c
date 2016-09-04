#ifdef ARM9

#include "arm9/source/common.h"
#include "arm9/source/hid.h"

#else

#include <3ds.h>
#include <string.h>
#include "menu_netloaderarm9.h"

#endif

#include "config.h"
#include "draw.h"
#include "utility.h"
#include "loader.h"
#include "menu.h"
#include "picker.h"

#ifdef ARM9
#define MENU_COUNT 4
static char menu_item[4][64] = {"File browser", "Settings", "Reboot", "PowerOff"};
#else
#define MENU_COUNT 6
static char menu_item[6][64] = {"File browser", "Netload 3dsx",
                                 "Netload arm9", "Settings", "Reboot", "PowerOff"};
#endif

static int menu_index = 0;
static void draw();

int menu_choose() {

    file_s picked;
    memset(&picked, 0, sizeof(file_s));
    pick_file(&picked, "/");

    int pathLength = 0;
    if ((pathLength = strlen(picked.path)) > 0) {
        int offset = ( pathLength > 3 && strcasecmp(&picked.path[pathLength-3], "dat") == 0 ) ? 0x12000 : 0;
        return load(picked.path, offset, NULL, 0, config->splashTopDef, config->splashBotDef);
    }

    return -1;
}

int menu_more() {

    menu_password();
    menu_index = 0;

    while (aptMainLoop()) {

        hidScanInput();
        u32 kDown = hidKeysDown();

        if (kDown & KEY_DOWN) {
            menu_index++;
            if (menu_index >= MENU_COUNT)
                menu_index = 0;
        }
        else if (kDown & KEY_UP) {
            menu_index--;
            if (menu_index < 0)
                menu_index = MENU_COUNT - 1;
        }
        else if (kDown & KEY_RIGHT) {
            menu_index = (menu_index == MENU_COUNT - 1) ? 0 : (MENU_COUNT - 1);
        }
        else if (kDown & KEY_LEFT) {
            menu_index = (menu_index == 0) ?  (MENU_COUNT - 1) : 0;
        }
        else if (kDown & KEY_A) {
            if (menu_index == 0 && menu_choose() == 0) {
                return 0;
#ifdef ARM9
            } else if (menu_index == 1) {
                menu_config();
            } else if (menu_index == 2) {
                reboot();
            } else if (menu_index == 3) {
                poweroff();
            }
#else
            } else if (menu_index == 1 && menu_netloader() == 0) {
                return 0;
            } else if (menu_index == 2 && menu_netloaderarm9() == 0) {
                return 0;
            } else if (menu_index == 3) {
                menu_config();
            } else if (menu_index == 4) {
                reboot();
            } else if (menu_index == 5) {
                poweroff();
            }
#endif
        }
        else if (kDown & KEY_B) {
            return -1;
        }

        draw();
    }
    return -1;
}

static void draw() {

    int i = 0;

    drawBg();
    drawTitle("*** Select an option ***");

    for (i = 0; i < MENU_COUNT; i++) {
        drawItem(i == menu_index, 16 * i, menu_item[i]);
    }

    // draw "help"
    switch (menu_index) {
#ifdef ARM9
        case 0:
            drawInfo("Browse for a file to boot");
            break;
        case 1:
            drawInfo("Edit boot settings");
            break;
        case 2:
            drawInfo("Reboot the 3ds...");
            break;
        case 3:
            drawInfo("Shutdown the 3ds...");
            break;
#else
        case 0:
            drawInfo("Browse for a file to boot or add a boot entry");
            break;
        case 1:
            drawInfo("Netload a file (3dsx) from the computer with 3dslink");
            break;
        case 2:
            drawInfo("Netload a file (arm9) from the computer with nc");
            break;
        case 3:
            drawInfo("Edit boot settings");
            break;
        case 4:
            drawInfo("Reboot the 3ds...");
            break;
        case 5:
            drawInfo("Shutdown the 3ds...");
            break;
#endif
        default:
            break;
    }

    swapFrameBuffers();
}
