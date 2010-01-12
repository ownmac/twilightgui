#include "canvas.h"

extern int bytes_per_line;
extern U8 *framebuffer;
extern int xres, yres;
extern FT_Face face;

static inline U16 utf8_to_unicode(const S8 *ch)
{
    U16 unicode = 0;

    if(((*ch) >> 4) == 0xE)
    {
        unicode = (ch[0] & 0x1F) << 12;
        unicode |= (ch[1] & 0x3F) << 6;
        unicode |= (ch[2] & 0x3F);
    }
    else if(((*ch) >> 7) == 0)
    {
        unicode = ch[0];
    }

    return unicode;
}


static inline int check_position_param(struct canvas *ca,
        int x, int y, int *width, int *height)
{
    if(x >= ca->width || y >= ca->height)
    {
        return 0;
    }
    if(*width + x > ca->width)
    {
        *width = ca->width - x;
    }
    if(*height + y > ca->height)
    {
        *height = ca->height - y;
    }

    return 1;
}

static inline int check_position(struct canvas *ca, int x, int y)
{
    if(x >= ca->width || y >= ca->height)
    {
        return 0;
    }

    return 1;
}

static inline COLOR alpha_blend(COLOR src, COLOR dest, int a)
{
    COLOR r, g, b, color;

    r = R(dest); g = G(dest); b = B(dest);
    r = ((R(src) * a + r * (255 - a)) >> 8);
    g = ((G(src) * a + g * (255 - a)) >> 8);
    b = ((B(src) * a + b * (255 - a)) >> 8);
    color = ARGB(255, r, g, b);
    return color;
}


struct canvas* canvas_create(int width, int height)
{
    struct canvas *ca = malloc(sizeof(struct canvas));
    if(ca == NULL)
    {
        printf("alloc canvas struct failed!\n");
        return NULL;
    }

    ca->data = malloc(width * height * sizeof(COLOR));
    if(ca->data == NULL)
    {
        printf("alloc canvas buffer failed!\n");
        return NULL;
    }

    ca->width = width;
    ca->height = height;

    return ca;
}

void canvas_release(struct canvas *ca)
{
    if(ca)
    {
        if(ca->data)
        {
            free(ca->data);
        }
        free(ca);
    }
}

void canvas_paint(struct canvas *ca, int x, int y)
{
    int i;
    int width, height;
    U8 *dest = framebuffer + y * bytes_per_line + x * sizeof(COLOR);
    U8 *src = ca->data;
    int src_step = ca->width * sizeof(COLOR);

    if(x >= xres || y >= yres)
    {
        return;
    }
    
    width = ca->width * sizeof(COLOR);
    height = ca->height;
    if(ca->width + x > xres)
    {
        width = (xres - x) * sizeof(COLOR);
    }
    if(ca->height + y > yres)
    {
        height = yres - y;
    }

    for(i = 0; i < height; i++)
    {
        memcpy(dest, src, width);
        dest += bytes_per_line;
        src += src_step;
    }
}


static inline void canvas_line_vertical(struct canvas *ca,
        int x, int y1, int y2, COLOR color)
{
    int y, a;
    COLOR *dest;

    if(y2 < y1)
        SWAP(y1, y2);


    dest = ((COLOR*)ca->data) + y1 * ca->width + x;
    a = A(color);
    for(y = y1; y <= y2; y++)
    {
        *dest = alpha_blend(color, *dest, a);
        dest += ca->width;
    }
}


static inline void canvas_line_horizontal(struct canvas *ca,
        int x1, int x2, int y, COLOR color)
{
    int x, a;
    COLOR *dest;

    if(x2 < x1)
        SWAP(x1, x2);


    dest = ((COLOR*)ca->data) + y * ca->width + x1;
    a = A(color);
    for(x = x1; x <= x2; x++)
    {
        *dest = alpha_blend(color, *dest, a);
        dest++;
    }
}

static inline void canvas_line_gentle(struct canvas *ca,
        int x1, int y1, int x2, int y2, COLOR color)
{
    int x, dx, dy, dx2, dy2, e, width;
    int a;
    COLOR *dest;

    if(x2 < x1)
    {
        SWAP(x1, x2);
        SWAP(y1, y2);
    }

	dest = ((COLOR*)ca->data) + y1 * ca->width + x1;

    dx = x2 - x1;
    dy = y2 - y1;
    dx2 = dx * 2;
    dy2 = ABS(dy) * 2;
    width = (dy < 0) ? -ca->width : ca->width;
    e = 0;

    for(x = x1; x < x2; x++)
    {
    	a = (e << 8) / dx2;
        if(e > dx)
        {
        	if(a > 255)
        	{
        		a -= 255;
        		dest += width;
        		*dest = alpha_blend(color, *dest, 255 - a);
        		*(dest + width) = alpha_blend(color, *(dest + width), a);
        	}
        	else {
				*dest = alpha_blend(color, *dest, 255 - a);
				dest += width;
				*dest = alpha_blend(color, *dest, a);
        	}
        	e += dy2 - dx2;
        }
        else
        {
			*dest = alpha_blend(color, *dest, 255 - a);
			*(dest + width) = alpha_blend(color, *(dest + width), a);
			e += dy2;
        }
        dest++;
    }
}

static inline void canvas_line_steep(struct canvas *ca,
        int x1, int y1, int x2, int y2, COLOR color)
{
    int x, y, dx, dy, e, rest;
    int a;
    COLOR *dest;

    if(y2 < y1)
    {
        SWAP(x1, x2);
        SWAP(y1, y2);
    }

    dx = x2 - x1;
    dy = y2 - y1;
    e = dx > 0 ? 1 : (-1);

    for(y = y1; y < y2; y++)
    {
        x = (dx * (y - y1)) / dy + x1;
        rest = (((dx * (y - y1)) % dy) << 8) / dy;
        rest = ABS(rest);
        a = 255 - rest;
        dest = ((COLOR*)ca->data) + y * ca->width + x;
        *dest = alpha_blend(color, *dest, a);

        if(rest != 0)
        {
            dest = ((COLOR*)ca->data) + y * ca->width + (x + e);
            a = rest;
            *dest = alpha_blend(color, *dest, a);
        }
    }
}

void canvas_line(struct canvas *ca, 
   int x1, int y1, int x2, int y2, COLOR color)
{
    int dx = x2 - x1, dy = y2 - y1;

    if(x1 >= xres || x2 >= xres || y1 >= yres || y2 >= yres
            ||((x1 == x2) && (y1 == y2)))
        return;

    if(x1 == x2)
    {
        canvas_line_vertical(ca, x1, y1, y2, color);
        return;
    }
    if(y1 == y2)
    {
        canvas_line_horizontal(ca, x1, x2, y1, color);
        return;
    }

    if(ABS(dx) > ABS(dy))
    {
        canvas_line_gentle(ca, x1, y1, x2, y2, color); // antialias
        return;
    }
    else
    {
        canvas_line_steep(ca, x1, y1, x2, y2, color); // antialias
        return;
    }
}


void canvas_fillrect(struct canvas *ca, 
   int x, int y, int width, int height, COLOR color)
{
    int i, j;
    int step;
    COLOR *dest = ((COLOR*)ca->data) + y * ca->width + x;
    int a = A(color);

    if(!check_position_param(ca, x, y, &width, &height))
        return;

    step = ca->width - width;

    for(i = 0; i < height; i++)
    {
        for(j = 0; j < width; j++)
        {
            *(dest++) = alpha_blend(color, *dest, a);
        }
        dest += step;
    }
}

void canvas_rect(struct canvas *ca, 
   int x, int y, int width, int height, COLOR color)
{
    int x1, y1, x2, y2;

    if(!check_position_param(ca, x, y, &width, &height))
        return;

    x1 = x + width - 1; y1 = y;
    canvas_line(ca, x, y, x1, y1, color);
    x2 = x1; y2 = y + height - 1;
    canvas_line(ca, x1, y1 + 1, x2, y2, color);
    x1 = x; y1 = y2;
    canvas_line(ca, x2 - 1, y2, x1, y1, color);
    canvas_line(ca, x1, y1 - 1, x, y + 1, color);

    return;
}

static inline void canvas_circle_point(struct canvas *ca,
        int x, int y, int x1, int y1, int a, COLOR color)
{
	int x2, y2;
	COLOR *dest;
	int width = ca->width;

	x2 = x + x1; y2 = y - y1;
	if(!check_position(ca, x2, y2))
		return;
	dest = ((COLOR *)ca->data) + y2 * width + x2;
	*dest = alpha_blend(color, *dest, a);

	y2 = y + y1;
	if(!check_position(ca, x2, y2))
		return;
	dest += (y1 * width) << 1;
	*dest = alpha_blend(color, *dest, a);

	x2 = x - x1;
	if(!check_position(ca, x2, y2))
		return;
	dest -= x1 << 1;
	*dest = alpha_blend(color, *dest, a);

	y2 = y - y1;
	if(!check_position(ca, x2, y2))
		return;
	dest -= (y1 * width) << 1;
	*dest = alpha_blend(color, *dest, a);

	x2 = x + y1; y2 = y - x1;
	if(!check_position(ca, x2, y2))
		return;
	dest = ((COLOR *)ca->data) + y2 * width + x2;
	*dest = alpha_blend(color, *dest, a);

	y2 = y + x1;
	if(!check_position(ca, x2, y2))
		return;
	dest += (x1 * width) << 1;
	*dest = alpha_blend(color, *dest, a);

	x2 = x - y1;
	if(!check_position(ca, x2, y2))
		return;
	dest -= y1 << 1;
	*dest = alpha_blend(color, *dest, a);

	y2 = y + x1;
	if(!check_position(ca, x2, y2))
		return;
	dest -= (x1 * width) << 1;
	*dest = alpha_blend(color, *dest, a);
}

void canvas_circle(struct canvas *ca,
        int x, int y, int r, COLOR color)
{
    int x1 = r, y1 = 0, e = 5 - (r << 2);
    int a;

    for(y1 = 1; y1 <= x1; y1++)
    {
        a = ((e + (x1 << 3)) << 8) / (x1 << 3);
        //a = ABS(e);
        printf("%d %d\n",e, a);
        if(e < 0)
        {
        	canvas_circle_point(ca, x, y, x1, y1, 255 - a, color);
        	canvas_circle_point(ca, x, y, x1 - 1, y1, a, color);
        	e += (y1 << 3) + 4;
        }
        else
        {
        	a -= 255;
        	canvas_circle_point(ca, x, y, x1, y1, a, color);
        	x1--;
        	canvas_circle_point(ca, x, y, x1, y1, 255 - a, color);
			e += (y1 << 3) - (x1 << 3) + 4;
        }
    }
}

void canvas_text(struct canvas *ca, int x, int y, int size,
        COLOR color, const S8 *str)
{
    FT_GlyphSlot slot = face->glyph;
    int i, j, a;
    int painted_width = 0, bmp_index = 0;
    int step, cur_width, cur_height;
    U16 unicode;
    const S8 *cur = str;
    const S8 *end = str + strlen(str);
    COLOR *dest;

    if(!check_position(ca, x, y) || str == NULL)
        return;

    if(FT_Set_Pixel_Sizes(face, 0, size))
    {
        printf("freetype set pixel sizes failed!\n");
        return;
    }

    while(cur < end)
    {
        unicode = utf8_to_unicode(cur);
        if(unicode == 0)
            return;
        else if(unicode < 0x7F)
            cur += 1;
        else
            cur += 3;

        if(FT_Load_Char(face, unicode, FT_LOAD_RENDER))
        {
            printf("freetype load char failed!\n");;
        }

        bmp_index = 0;
        if(slot->bitmap.width > ca->width - x - painted_width - slot->bitmap_left)
            cur_width = ca->width - x - painted_width - slot->bitmap_left;
        else
            cur_width = slot->bitmap.width;
        if(slot->bitmap.rows + y > ca->height)
            cur_height = ca->height - y;
        else
            cur_height = slot->bitmap.rows;

        step = ca->width - cur_width;
        dest = ((COLOR*)ca->data) + (y + size - slot->bitmap_top) * ca->width
                + (x + slot->bitmap_left + painted_width);

        for(i = 0; i < cur_height; i++)
        {
            for(j = 0; j < cur_width; j++)
            {
                a = slot->bitmap.buffer[bmp_index++];
                a = (A(color) * a) >> 8;
                *(dest++) = alpha_blend(color, *dest, a);
            }
            bmp_index += slot->bitmap.width - cur_width;
            dest += step;
        }
        painted_width += slot->advance.x >> 6;
        if(painted_width >= ca->width - x)
            return;
    }

    return;
}
