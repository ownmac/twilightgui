#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <tslib.h>
#include <time.h>
#include "framebuffer.h"
#include "canvas.h"
#include "font.h"
#include "ts.h"

struct button {
	int x;
	int y;
	char *str;
};

int main(int argc, char **argv)
{
    struct canvas *ca_bg = canvas_create(320, 240);
    struct canvas *ca_button = canvas_create(60, 50);
    struct tsdev *ts;
    int x, y;
    int i;
    struct button buttons[] = {
    		{60, 50, "1"}, {130, 50,"2"}, {200, 50,"3"},
    		{60, 110,"4"}, {130, 110,"5"}, {200, 110,"6"},
    		{60, 170,"7"}, {130, 170,"8"}, {200, 170,"9"}
    };

    if(framebuffer_init() < 0)
    	return 0;
    if(font_init() < 0)
    	return 0;

    ts = ts_open("/dev/event0", 0);
    if (!ts) {
		perror("ts_open");
		exit(1);
	}
	if (ts_config(ts)) {
		perror("ts_config");
		exit(1);
	}

	canvas_rect(ca_bg, 0, 0, 320, 240, ARGB(255, 0, 0, 0));
	canvas_paint(ca_bg, 0, 0);

	for(i = 0; i < sizeof(buttons) / sizeof(struct button); i++) {
		canvas_rect(ca_bg, buttons[i].x, buttons[i].y, 60, 50, ARGB(128, 0, 250, 0));
		canvas_text(ca_bg, buttons[i].x, buttons[i].y, 60, 50, ARGB(128, 0, 250, 0), buttons[i].str);
		//canvas_paint(ca_button, 60, 50);
	}

	canvas_paint(ca_bg, 0, 0);

   	while(1) {
   		getxy(ts, &x, &y);
   		for(i = 0; i < sizeof(buttons) / sizeof(struct button); i++) {
			if(x > buttons[i].x && y > buttons[i].y
					&& x < buttons[i].x + 60 && y < buttons[i].y + 50) {
				printf("%s\n", buttons[i].str);
				break;
			}
		}
   	}

	canvas_release(ca_bg);
	canvas_release(ca_button);
    return 0;
}

