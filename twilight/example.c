#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <tslib.h>
#include "framebuffer.h"
#include "canvas.h"
#include "font.h"

int main(int argc, char **argv)
{
    struct canvas *ca = canvas_create(320, 240);
    struct canvas *ca_ts = canvas_create(20, 20);

    if(framebuffer_init() < 0)
    	return 0;
    if(font_init() < 0)
    	return 0;

    canvas_fillrect(ca, 0, 0, 320, 240, ARGB(255, 0, 0, 0));
    canvas_paint(ca, 0, 0);

    struct tsdev *ts;

	ts = ts_open("/dev/event0", 0);
	if (!ts)
	{
		perror("tsdevice");
		exit(1);
	}

	if (ts_config(ts)) {
		perror("ts_config");
		exit(1);
	}

	while (1) {
		struct ts_sample samp;
		int ret;

		ret = ts_read(ts, &samp, 1);
		ret = ts_read(ts, &samp, 1);
		ret = ts_read(ts, &samp, 1);
		ret = ts_read(ts, &samp, 1);
		if (ret < 0)
		{
			perror("ts_read");
			exit(1);
		}

		if(ret != 1 || samp.pressure == 0)
			continue;

		canvas_rect(ca_ts, 0, 0, 20, 20, ARGB(255, 0, 255, 0));
		canvas_paint(ca_ts, samp.x - 10, samp.y - 10);
		usleep(50000);
		canvas_fillrect(ca_ts, 0, 0, 20, 20, ARGB(255, 0, 0, 0));
		canvas_paint(ca_ts, samp.x - 10, samp.y - 10);
		printf("@x: %d, y: %d\n", samp.x, samp.y);
	}

	canvas_release(ca);
	canvas_release(ca_ts);
    return 0;
}

