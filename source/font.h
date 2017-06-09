#ifndef FONT_H
#define FONT_H

typedef struct {
    char c;
    int x, y, w, h, xo, yo, xa;
    u8 *data;
} charDesc_s;
typedef struct {
    u8 *data;
    charDesc_s *desc;
    u8 height;
    u8 color[4];
} font_s;

extern u8 defaultData[];
extern charDesc_s defaultDesc[];
extern font_s fontDefault;

#endif
