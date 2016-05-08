#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef ARM9

#include "arm9/source/common.h"
#include "memory.h"
#else
#include <3ds.h>
#endif

#include <stdarg.h>
#include "gfx.h"
#include "text.h"

#ifdef ARM9

u8 *gfxGetFramebuffer(gfxScreen_t screen, gfx3dSide_t side, u16 *width, u16 *height) {
    if (screen == GFX_TOP) {
		if (width) *width = 240;
        if (height) *height = 400;
        return PTR_TOP_SCREEN_BUF;
    } else {
        if (width) *width = 240;
        if (height) *height = 320;
        return PTR_BOT_SCREEN_BUF;
    }
}

#endif

void drawPixel(int x, int y, char r, char g, char b, u8 *screen) {
    int height = 240;

    u32 v = (height - 1 - y + x * height) * 3;
    screen[v] = b;
    screen[v + 1] = g;
    screen[v + 2] = r;
}

void drawLine(gfxScreen_t screen, gfx3dSide_t side, int x1, int y1, int x2, int y2, char r, char g, char b) {
    u16 fbWidth, fbHeight;
    u8 *fbAdr = gfxGetFramebuffer(screen, side, &fbWidth, &fbHeight);

    int x, y;
    if (x1 == x2) {
        if (y1 < y2) for (y = y1; y < y2; y++) drawPixel(x1, y, r, g, b, fbAdr);
        else for (y = y2; y < y1; y++) drawPixel(x1, y, r, g, b, fbAdr);
    } else {
        if (x1 < x2) for (x = x1; x < x2; x++) drawPixel(x, y1, r, g, b, fbAdr);
        else for (x = x2; x < x1; x++) drawPixel(x, y1, r, g, b, fbAdr);
    }
}

void drawRect(gfxScreen_t screen, gfx3dSide_t side, int x1, int y1, int x2, int y2, char r, char g, char b) {
    drawLine(screen, side, x1, y1, x2, y1, r, g, b);
    drawLine(screen, side, x2, y1, x2, y2, r, g, b);
    drawLine(screen, side, x1, y2, x2, y2, r, g, b);
    drawLine(screen, side, x1, y1, x1, y2, r, g, b);
}

void drawRectColor(gfxScreen_t screen, gfx3dSide_t side, int x1, int y1, int x2, int y2, u8 *color) {
    drawRect(screen, side, x1, y1, x2, y2, color[0], color[1], color[2]);
}

void gfxDrawTextf(gfxScreen_t screen, gfx3dSide_t side, font_s *f, s16 x, s16 y, const char *fmt, ...) {
    char s[512];
    memset(s, 0, 512);
    va_list args;
    va_start(args, fmt);
    int len = vsprintf(s, fmt, args);
    va_end(args);
    if (len)
        gfxDrawText(screen, side, f, s, x, y);
}

void gfxDrawText(gfxScreen_t screen, gfx3dSide_t side, font_s *f, char *str, s16 x, s16 y) {
    if (!str)return;
    if (!f)f = &fontDefault;

    u16 fbWidth, fbHeight;
    u8 *fbAdr = gfxGetFramebuffer(screen, side, &fbWidth, &fbHeight);

    drawString(fbAdr, f, str, x, 240 - y, fbHeight, fbWidth);
}

void gfxDrawTextN(gfxScreen_t screen, gfx3dSide_t side, font_s *f, char *str, u16 length, s16 x, s16 y) {
    if (!str)return;
    if (!f)f = &fontDefault;

    u16 fbWidth, fbHeight;
    u8 *fbAdr = gfxGetFramebuffer(screen, side, &fbWidth, &fbHeight);

    drawStringN(fbAdr, f, str, length, x, 240 - y, fbHeight, fbWidth);
}

void gfxFillColor(gfxScreen_t screen, gfx3dSide_t side, u8 rgbColor[3]) {
    u16 fbWidth, fbHeight;
    u8 *fbAdr = gfxGetFramebuffer(screen, side, &fbWidth, &fbHeight);

    //TODO : optimize; use GX command ?
    int i;
    for (i = 0; i < fbWidth * fbHeight; i++) {
        *(fbAdr++) = rgbColor[2];
        *(fbAdr++) = rgbColor[1];
        *(fbAdr++) = rgbColor[0];
    }
}

void gfxFillColorGradient(gfxScreen_t screen, gfx3dSide_t side, u8 rgbColorStart[3], u8 rgbColorEnd[3]) {
    u16 fbWidth, fbHeight;
    u8 *fbAdr = gfxGetFramebuffer(screen, side, &fbWidth, &fbHeight);
    u8 colorLine[fbWidth * 3];

    //TODO : optimize; use GX command ?
    int i;
    float n;
    float total = (float) (fbWidth - 1);
    // make slightly bigger to prevent gradients from blending around.  SHould be removed and have the gradient color be better later.
    total *= 1.5f;
    for (i = 0; i < fbWidth; i++) {
        n = (float) i / total;
        colorLine[i * 3 + 0] = (float) rgbColorStart[2] * (1.0f - n) + (float) rgbColorEnd[2] * n;
        colorLine[i * 3 + 1] = (float) rgbColorStart[1] * (1.0f - n) + (float) rgbColorEnd[1] * n;
        colorLine[i * 3 + 2] = (float) rgbColorStart[0] * (1.0f - n) + (float) rgbColorEnd[0] * n;
    }

    for (i = 0; i < fbHeight; i++) {
        memcpy(fbAdr, colorLine, fbWidth * 3);
        fbAdr += fbWidth * 3;
    }
}

void _gfxDrawRectangle(gfxScreen_t screen, gfx3dSide_t side, u8 rgbColor[4], s16 x, s16 y, u16 width, u16 height) {
    u16 fbWidth, fbHeight;
    u8 *fbAdr = gfxGetFramebuffer(screen, side, &fbWidth, &fbHeight);

    if (x + width < 0 || x >= fbWidth)return;
    if (y + height < 0 || y >= fbHeight)return;

    if (x < 0) {
        width += x;
        x = 0;
    }
    if (y < 0) {
        height += y;
        y = 0;
    }
    if (x + width >= fbWidth)width = fbWidth - x;
    if (y + height >= fbHeight)height = fbHeight - y;

	if ( rgbColor[3] == 0xFF )
	{
		u8 colorLine[width * 3];

		int j;
		for (j = 0; j < width; j++) {
			colorLine[j * 3 + 0] = rgbColor[2];
			colorLine[j * 3 + 1] = rgbColor[1];
			colorLine[j * 3 + 2] = rgbColor[0];
		}

		fbAdr += fbWidth * 3 * y;
		for (j = 0; j < height; j++) {
			memcpy(&fbAdr[x * 3], colorLine, width * 3);
			fbAdr += fbWidth * 3;
		}
	}
	else
	{
		float alpha = (float)rgbColor[3] / 255.f;
		float one_minus_alpha = 1.f - alpha;
		int i, j;
		fbAdr += fbWidth * 3 * y;
		for (j = 0; j < height; j++)
		{
			for (i = 0; i < width; i++)
			{
				fbAdr[3*(i+x)+0] = (u8)(alpha*(float)rgbColor[0]+one_minus_alpha*(float)fbAdr[3*(i+x)+0]);
				fbAdr[3*(i+x)+1] = (u8)(alpha*(float)rgbColor[1]+one_minus_alpha*(float)fbAdr[3*(i+x)+1]);
				fbAdr[3*(i+x)+2] = (u8)(alpha*(float)rgbColor[2]+one_minus_alpha*(float)fbAdr[3*(i+x)+2]);	
			}
			fbAdr += fbWidth * 3;
		}
	}
}

void gfxDrawRectangle(gfxScreen_t screen, gfx3dSide_t side, u8 rgbColor[4], s16 x, s16 y, u16 width, u16 height) {
    _gfxDrawRectangle(screen, side, rgbColor, 240 - y, x, height, width);
}

void gfxClearTop(u8 top1[3], u8 top2[3]) {
    gfxFillColorGradient(GFX_TOP, GFX_LEFT, top1, top2);
}

void gfxClearBot(u8 bot[8]) {
    gfxFillColor(GFX_BOTTOM, GFX_LEFT, bot);
}

void gfxClear() {
#ifdef ARM9
    memset(PTR_TOP_SCREEN, 0, TOP_SCREEN_SIZE);
    memset(PTR_BOT_SCREEN, 0, BOT_SCREEN_SIZE);
    memset(PTR_TOP_SCREEN_BUF, 0, TOP_SCREEN_SIZE);
    memset(PTR_BOT_SCREEN_BUF, 0, BOT_SCREEN_SIZE);
    memset(PTR_TOP_BG, 0, TOP_SCREEN_SIZE);
    memset(PTR_BOT_BG, 0, BOT_SCREEN_SIZE);
#else
    gfxFillColor(GFX_TOP, GFX_LEFT, (u8[3]) {0x00, 0x00, 0x00});
    gfxFillColor(GFX_BOTTOM, GFX_LEFT, (u8[3]) {0x00, 0x00, 0x00});
#endif
}

void gfxSwap() {
#ifdef ARM9
    memcpy(PTR_TOP_SCREEN, PTR_TOP_SCREEN_BUF, TOP_SCREEN_SIZE);
    memcpy(PTR_BOT_SCREEN, PTR_BOT_SCREEN_BUF, BOT_SCREEN_SIZE);
#else
    gfxFlushBuffers();
    gfxSwapBuffers();
    gspWaitForVBlank();
#endif
}
