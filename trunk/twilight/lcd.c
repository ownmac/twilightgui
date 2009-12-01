#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "framebuffer.h"
#include "gdi.h"
#include "font.h"

struct ts_event 
{
	unsigned short pressure;
	unsigned short x;
	unsigned short y;
	unsigned short pad;
};

int main(int argc, char **argv)
{
    int i;
    struct canvas *ca = create_canvas(320, 240);

    if(lcd_init() < 0)
    	return 0;
    if(font_init() < 0)
    	return 0;

    fill_rectangle(ca, 0, 0, 320, 240, ARGB(255, 0, 0, 0));

    
    for(i = 0; i < 1; i++)
    {
        fill_rectangle(ca, 10, 10, 30, 30, ARGB(100, 0, 255, 0));
        fill_rectangle(ca, 20, 20, 30, 30, ARGB(200, 0, 255, 0));
        //fill_rectangle(ca, 20, 20, 30, 30, ARGB(100, 0, 255, 0));
        //fill_rectangle(ca, 10 + i * 30, 50, 30, 30, ARGB(100, 0, 255, 0));
        //fill_rectangle(ca, 10 + i * 30, 90, 30, 30, ARGB(100, 0, 0, 255));
        line(ca, 0, 0, 320, 240, ARGB(100, 0, 255, 0));
        line(ca, 0, 0, 320, 120, ARGB(100, 0, 255, 0));
        line(ca, 0, 0, 160, 240, ARGB(100, 0, 255, 0));
        line(ca, 320, 0, 0, 240, ARGB(100, 0, 255, 0));
    }

    text(ca, 10, 100, 310, 24, ARGB(255, 0, 0, 255), "天王盖地虎，宝塔镇河妖");

    paint_canvas(ca, 0, 0);
    release_canvas(ca);

}

