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
	struct canvas *ca;
	int x;
	int y;
	char *str;
};

int main(int argc, char **argv)
{
    struct canvas *ca_bg = canvas_create(320, 240);
    struct tsdev *ts;
    int x, y;
    int i;
    struct button buttons[] = {
    		{NULL, 60, 50, "1"}, {NULL, 130, 50,"2"}, {NULL, 200, 50,"3"},
    		{NULL, 60, 110,"4"}, {NULL, 130, 110,"5"}, {NULL, 200, 110,"6"},
    		{NULL, 60, 170,"7"}, {NULL, 130, 170,"8"}, {NULL, 200, 170,"9"}
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
		buttons[i].ca = canvas_create(60, 50);
		canvas_rect(buttons[i].ca, 0, 0, 60, 50, ARGB(128, 0, 255, 0));
		canvas_text(buttons[i].ca, 18, 0, 40, ARGB(128, 0, 255, 0), buttons[i].str);
		canvas_paint(buttons[i].ca, buttons[i].x, buttons[i].y);
	}


   	while(1) {
   		getxy(ts, &x, &y);
   		for(i = 0; i < sizeof(buttons) / sizeof(struct button); i++) {
			if(x > buttons[i].x && y > buttons[i].y
					&& x < buttons[i].x + 60 && y < buttons[i].y + 50) {
				canvas_fillrect(buttons[i].ca, 1, 1, 58, 48, ARGB(128, 0, 250, 0));
				canvas_text(buttons[i].ca, 18, 0, 40, ARGB(128, 255, 255, 255), buttons[i].str);
				canvas_paint(buttons[i].ca, buttons[i].x, buttons[i].y);
				usleep(90000);
				canvas_fillrect(buttons[i].ca, 1, 1, 58, 48, ARGB(255, 0, 0, 0));
				canvas_text(buttons[i].ca, 18, 0, 40, ARGB(128, 0, 255, 0), buttons[i].str);
				canvas_paint(buttons[i].ca, buttons[i].x, buttons[i].y);
				printf("%s\n", buttons[i].str);
				break;
			}
		}
   	}

	canvas_release(ca_bg);

    return 0;
}

