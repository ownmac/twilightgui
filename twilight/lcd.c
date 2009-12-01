#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <tslib.h>
#include "framebuffer.h"
#include "gdi.h"
#include "font.h"

int main(int argc, char **argv)
{
    int total;
	float user, nice, system, idle;
	char cpu[32];
	char str[64];
	FILE *fp;

    struct canvas *ca = create_canvas(320, 240);
    struct canvas *ca_cpu = create_canvas(300, 20);
    struct canvas *ca_ts = create_canvas(20, 20);

    if(lcd_init() < 0)
    	return 0;
    if(font_init() < 0)
    	return 0;

    fill_rectangle(ca, 0, 0, 320, 240, ARGB(255, 0, 0, 0));
    paint_canvas(ca, 0, 0);

	memset(cpu, 0, sizeof(cpu));
    sprintf(cpu, "user nice   sys   idle");
    fill_rectangle(ca_cpu, 0, 0, 160, 20, ARGB(255, 0, 0, 255));
    text(ca_cpu, 0, 0, 300, 16, ARGB(255, 0, 0, 0), cpu);
    paint_canvas(ca_cpu, 10, 80);

    while(0)
    {
    	memset(str, 0, sizeof(str));
    	memset(cpu, 0, sizeof(cpu));

		fp = fopen("/proc/stat", "r");
		while (fgets(str, 200, fp))
		{
			if (strstr(str, "cpu"))
            {
                sscanf(str, "%s %f %f %f %f", cpu, &user, &nice, &system, &idle);
                break;
            }
        }
        fclose(fp);

        total = (user + nice + system + idle);
        user = (user / total) * 100;
        nice = (nice / total) * 100;
        system = (system / total) * 100;
        idle = (idle / total) * 100;
        sprintf(cpu, "%4.2f  %4.2f  %4.2f  %4.2f", user, nice, system, idle);

        fill_rectangle(ca_cpu, 0, 0, 300, 20, ARGB(255, 0, 0, 0));
        text(ca_cpu, 0, 0, 300, 16, ARGB(255, 0, 0, 255), cpu);
        paint_canvas(ca_cpu, 10, 100);

        sleep(1);
    }

    struct tsdev *ts;
	int x, y;
	unsigned int i;
	unsigned int mode = 0;

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
		if (ret < 0)
		{
			perror("ts_read");
			exit(1);
		}

		if(ret != 1)
			continue;

		fill_rectangle(ca_ts, 0, 0, 20, 20, ARGB(255, 0, 255, 0));
		paint_canvas(ca_ts, samp.x - 10, samp.y - 10);
		usleep(50000);
		fill_rectangle(ca_ts, 0, 0, 20, 20, ARGB(255, 0, 0, 0));
		paint_canvas(ca_ts, samp.x - 10, samp.y - 10);
		printf("@x: %d, y: %d\n", samp.x, samp.y);
	}

    release_canvas(ca_cpu);
    release_canvas(ca);
    return 0;
}

