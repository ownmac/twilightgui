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
    U32 width;
    U32 height;
};

struct canvas* canvas_create(U32 width, U32 height);
void canvas_release(struct canvas *ca);
void canvas_paint(struct canvas *ca, U32 x, U32 y);

void canvas_line(struct canvas *ca, 
        U32 x1, U32 y1, U32 x2, U32 y2, COLOR color);
void canvas_fillrect(struct canvas *ca,
        U32 x, U32 y, U32 width, U32 height, COLOR color);
void canvas_rect(struct canvas *ca,
        U32 x, U32 y, U32 width, U32 height, COLOR color);
void canvas_text(struct canvas *ca, U32 x, U32 y, U32 wdith, U32 height,
		COLOR color, const S8 *str);

#endif
