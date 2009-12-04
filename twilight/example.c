#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <tslib.h>
#include <time.h>
#include "framebuffer.h"
#include "canvas.h"
#include "font.h"

int main(int argc, char **argv)
{
    struct canvas *ca_bg = canvas_create(320, 240);
    struct canvas *ca = canvas_create(320, 240);
    time_t cur_tm;

    if(framebuffer_init() < 0)
    	return 0;
    if(font_init() < 0)
    	return 0;

    canvas_fillrect(ca_bg, 0, 0, 320, 240, ARGB(255, 0, 0, 0));
    canvas_paint(ca_bg, 0, 0);

    while(1)
    {
    	time(&cur_tm);
    	canvas_fillrect(ca, 10, 100, 300, 30, ARGB(255, 0, 0, 0));
    	canvas_text(ca, 10, 100, 300, 24, ARGB(255, 0, 0, 255), ctime(&cur_tm));
    	canvas_paint(ca, 0, 0);
    	sleep(1);
    }

	canvas_release(ca_bg);
	canvas_release(ca);
    return 0;
}

