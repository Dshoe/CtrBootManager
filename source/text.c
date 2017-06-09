#include <stdlib.h>
#include <string.h>

#ifdef ARM9

#include "arm9/source/common.h"

#else
#include <3ds.h>
#endif

#include "text.h"
//#include "font_bin.h"
#include "font.h"

//const u8 *font = font_bin;

//this code is not meant to be readable
int drawCharacter(u8 *fb, font_s *f, char c, s16 x, s16 y, u16 w, u16 h) {
    charDesc_s *cd = &f->desc[(int) c];
    if (!cd->data)return 0;
    x += cd->xo;
    y += f->height - cd->yo - cd->h;
    if (x < 0 || x + cd->w >= w || y < -cd->h || y >= h + cd->h)return cd->xa;
    u8 *charData = cd->data;
    int i, j;
    s16 cy = y, ch = cd->h, cyo = 0;
    if (y < 0) {
        cy = 0;
        cyo = -y;
        ch = cd->h - cyo;
    }
    else if (y + ch > h)ch = h - y;
    fb += (x * h + cy) * 3;
    const u8 r = f->color[0], g = f->color[1], b = f->color[2], a = f->color[3];
    if ( a == 0xFF )
    {
        for (i = 0; i < cd->w; i++) {
            charData += cyo;
            for (j = 0; j < ch; j++) {
                u8 v = *(charData++);
                if (v) {
                    fb[0] = (fb[0] * (0xFF - v) + (b * v)) >> 8;
                    fb[1] = (fb[1] * (0xFF - v) + (g * v)) >> 8;
                    fb[2] = (fb[2] * (0xFF - v) + (r * v)) >> 8;
                }
                fb += 3;
            }
            charData += (cd->h - (cyo + ch));
            fb += (h - ch) * 3;
        }
    }
    else if ( a > 0 )
    {
        int alpha = a;
        int one_minus_alpha = 255-alpha;

        for (i = 0; i < cd->w; i++) {
            charData += cyo;
            for (j = 0; j < ch; j++) {
                u8 v = *(charData++);
                if (v) {
                    u8 blend[3] = { (fb[0] * (0xFF - v) + (b * v)) >> 8,
                                    (fb[1] * (0xFF - v) + (g * v)) >> 8,
                                    (fb[2] * (0xFF - v) + (r * v)) >> 8 };
                    fb[0] = (u8)((alpha*blend[0]+one_minus_alpha*fb[0])/255);
                    fb[1] = (u8)((alpha*blend[1]+one_minus_alpha*fb[1])/255);
                    fb[2] = (u8)((alpha*blend[2]+one_minus_alpha*fb[2])/255);
                }
                fb += 3;
            }
            charData += (cd->h - (cyo + ch));
            fb += (h - ch) * 3;
        }
    }
    return cd->xa;
}

int getStringLength(font_s *f, char *str) {
    if (!f)f = &fontDefault;
    if (!str)return 0;
    int ret;
    for (ret = 0; *str; ret += f->desc[(int) *str++].xa);
    return ret;
}

void drawString(u8 *fb, font_s *f, char *str, s16 x, s16 y, u16 w, u16 h) {
    drawStringN(fb, f, str, strlen(str), x, y, w, h);
}

void drawStringN(u8 *fb, font_s *f, char *str, u16 length, s16 x, s16 y, u16 w, u16 h) {
    if (!f || !fb || !str)return;
    bool cut = false;
    int k;
    int dx = 0, dy = 0;
    k = strlen(str);
    if (k < length) length = k; else if (k > length) cut = true;
    for (k = 0; k < length; k++) {
        dx += drawCharacter(fb, f, str[k], x + dx, y + dy, w, h);
        if (str[k] == '\n') {
            dx = 0;
            dy -= 16;
        }
    }
    if (cut) {
        dx += drawCharacter(fb, f, '.', x + dx, y + dy, w, h);
        dx += drawCharacter(fb, f, '.', x + dx, y + dy, w, h);
        dx += drawCharacter(fb, f, '.', x + dx, y + dy, w, h);
    }
}
