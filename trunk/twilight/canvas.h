#ifndef _GDI_H
#define _GDI_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include "common.h"

#define ARGB(a, r, g, b) ((COLOR)((((U32)a) << 24) | (((U32)r) << 16) | (((U32)g) << 8) | (((U32)b))))
#define R(color) ((U8)(color >> 16))
#define G(color) ((U8)(color >> 8))
#define B(color) ((U8)(color))
#define A(color) ((U8)(color >> 24))

struct canvas {
    void *data;
    int width;
    int height;
};

struct canvas* canvas_create(int width, int height);
void canvas_release(struct canvas *ca);
void canvas_paint(struct canvas *ca, int x, int y);

void canvas_line(struct canvas *ca, 
        int x1, int y1, int x2, int y2, COLOR color);
void canvas_fillrect(struct canvas *ca,
        int x, int y, int width, int height, COLOR color);
void canvas_rect(struct canvas *ca,
        int x, int y, int width, int height, COLOR color);
void canvas_point(struct canvas *ca, int x, int y, COLOR color);
void canvas_circle(struct canvas *ca, int x, int y, int r, COLOR color);
void canvas_text(struct canvas *ca, int x, int y, int wdith, int height,
		COLOR color, const S8 *str);

#endif
