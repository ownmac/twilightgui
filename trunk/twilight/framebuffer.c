#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include "framebuffer.h"

static struct fb_fix_screeninfo f_info;
static struct fb_var_screeninfo v_info;

U8 *framebuffer;
U32 xres, yres;

U32 bytes_per_line;
U32 bytes_per_screen;


int lcd_init()
{
    int ret;

    int fd = open("/dev/fb0", O_RDWR);
    if(fd < 0)
    {
        printf("Can't open fb device!\n");
        return 0;
    }

    ioctl(fd, FBIOGET_VSCREENINFO, &v_info);
    //printf("xres_virtual: %d\n", v_info.xres_virtual);
    //printf("yres_virtual: %d\n", v_info.yres_virtual);
    //printf("xoffset: %d\n", v_info.xoffset);
    //printf("yoffset: %d\n", v_info.yoffset);
    //printf("bits_per_pixel: %d\n", v_info.bits_per_pixel);

    xres = v_info.xres_virtual;
    yres = v_info.yres_virtual;

    bytes_per_line = v_info.xres_virtual * v_info.bits_per_pixel / 8;
    bytes_per_screen = v_info.yres_virtual * bytes_per_line;

    framebuffer = mmap(NULL, bytes_per_screen, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(framebuffer <= 0)
    {
    	printf("framebuffer mmap failed!\n");
        return 0;
    }

    close(fd);

    return 1;
}



