#include "gdi.h"

extern U32 bytes_per_line;
extern U8 *framebuffer;
extern U32 xres, yres;
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

static inline U32 check_position_param(struct canvas *ca,
		U32 x, U32 y, U32 *width, U32 *height)
{
	if(x > ca->width || y > ca->height)
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

static inline COLOR alpha_blend(COLOR src, COLOR dest, U32 a)
{
	U32 r, g, b, color;

	r = R(dest); g = G(dest); b = B(dest);
	r = ((R(src) * a + r * (255 - a)) >> 8);
	g = ((G(src) * a + g * (255 - a)) >> 8);
	b = ((B(src) * a + b * (255 - a)) >> 8);
	color = ARGB(255, r, g, b);
	return color;
}

struct canvas* create_canvas(U32 width, U32 height)
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

void release_canvas(struct canvas *ca)
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

void paint_canvas(struct canvas *ca, U32 x, U32 y)
{
    int i;
    U32 width, height;
    U8 *dest = framebuffer + y * bytes_per_line + x * sizeof(COLOR);
    U8 *src = ca->data;
    U32 src_step = ca->width * sizeof(COLOR);

    if(x > xres || y > yres)
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

void fill_rectangle(struct canvas *ca, U32 x, U32 y, U32 width, U32 height, COLOR color)
{
    int i, j;
    U32 step;
    U32 *dest = ((U32 *)ca->data) + y * ca->width + x;
    U32 a;

    if(!check_position_param(ca, x, y, &width, &height))
    	return;

    step = ca->width - width;

    for(i = 0; i < height; i++)
    {    
        for(j = 0; j < width; j++)
        {    
            a = A(color);
            *(dest++) = alpha_blend(color, *dest, a);
        }
        dest += step;
    }
}

void line(struct canvas *ca, U32 x1, U32 y1, U32 x2, U32 y2, COLOR color)
{
    S32 startx, endx, starty, endy, x, y, dx, dy, e, rest;
    U32 a;
    U32 *dest;

    if(x1 > x2)
    {
        startx = x2; starty = y2;
        endx = x1; endy = y1;
    }
    else
    {
        startx = x1; starty = y1;
        endx = x2; endy = y2;
    }
    dx = endx - startx;
    dy = endy - starty;

    e = dy > 0 ? 1 : -1;

    for(x = startx, y = starty; x < endx; x++)
    {
        y = dy * (x - startx) / dx + starty;
        rest = (((dy * (x - startx)) % dx) << 8) / dx;
        rest = rest > 0 ? rest : -rest;
        dest = ((U32 *)ca->data) + y * ca->width + x;
        a = 255 - rest;
        *dest = alpha_blend(color, *dest, a);

        if(rest != 0)
        {
            dest = ((U32 *)ca->data) + (y + e) * ca->width + x;
            a = rest;
            *dest = alpha_blend(color, *dest, a);
        }
    }
}

void text(struct canvas *ca, U32 x, U32 y, U32 width, U32 height,
		COLOR color, const S8 *str)
{
    FT_GlyphSlot slot = face->glyph;
    U32 i, j, a;
    U32 step, painted_width = 0, cur_width, cur_height, bmp_width, bmp_index = 0;
    S32 bmp_left, bmp_top;
    U16 unicode;
    const S8 *cur = str;
    const S8 *end = str + strlen(str);
    U32 *dest;

    if((!check_position_param(ca, x, y, &width, &height))
    	|| str == NULL)
		return;

    if(FT_Set_Pixel_Sizes(face, 0, height))
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
		bmp_width = slot->bitmap.width;
		bmp_left = slot->bitmap_left;
		bmp_top = slot->bitmap_top;
		if(bmp_width > width - painted_width - bmp_left)
			cur_width = width - painted_width - bmp_left; //实际显示宽度
		else
			cur_width = bmp_width;

		cur_height = slot->bitmap.rows; //实际显示高度

		step = ca->width - cur_width;
		dest = ((U32 *)ca->data) + (y + height - bmp_top) * ca->width
				+ (x + bmp_left + painted_width);

		for(i = 0; i < cur_height; i++)
		{
			for(j = 0; j < cur_width; j++)
			{
				a = slot->bitmap.buffer[bmp_index++];
				a = (A(color) * a) >> 8;
				*(dest++) = alpha_blend(color, *dest, a);
			}
			bmp_index += bmp_width - cur_width;
			dest += step;
		}
		painted_width += slot->advance.x >> 6; //已显示的字符总宽
		if(painted_width >= width)
			return;
    }

    return;
}

