#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include "font.h"

FT_Face face;
int shm_font;
U8 *font_buffer = NULL;

int font_init(void)
{
    FT_Library library;
    int fd;
    struct stat st;
    int font_buffer_size;

    int error = FT_Init_FreeType(&library);
    if(error)
    {
        printf("freetype init failed!\n");
        return 0;
    }

    if(stat(FONT_FILE,	&st) < 0)
	{
		printf("stat failed!\n");
		return 0;
	}
	font_buffer_size = st.st_size;

	if((shm_font = shmget(SHM_FONT_KEY, font_buffer_size, 0)) >= 0)
	{
		if((font_buffer = shmat(shm_font, NULL, 0)) <= 0)
		{
			printf("get share memory address failed!\n");
			goto shmat_error;
		}
	}
	else if(errno == ENOENT)
    {
    	if((shm_font = shmget(SHM_FONT_KEY, font_buffer_size, IPC_CREAT)) < 0)
		{
    		printf("share memory get failed!\n");
    		return 0;
		}
		if((font_buffer = shmat(shm_font, NULL, 0)) <= 0)
		{
			printf("share memory at font buffer failed!\n");
			return 0;
		}

		if((fd = open(FONT_FILE, O_RDONLY)) < 0)
		{
			printf("open font file failed!\n");
			goto open_error;
		}
		if(read(fd, font_buffer, font_buffer_size) < font_buffer_size)
		{
			printf("read font file failed! %d\n", font_buffer_size);
			perror("a");
			goto read_error;
		}
		close(fd);
    }
    else
    {
    	printf("share memory get failed! errno: %d\n", error);
    	return 0;
    }

    error = FT_New_Memory_Face(library,
				font_buffer, font_buffer_size, 0, &face);
    if(error)
    {
        printf("freetype new font face failed! errno: %d\n", error);
        return 0;
    }

    error = FT_Set_Pixel_Sizes(face, 0, 16);
    if(error)
	{
		printf("freetype set pixel sizes! errno: %d\n", error);
		return 0;
	}

    error = FT_Load_Char(face, 'A', FT_LOAD_RENDER);

    return 1;

read_error:
    close(fd);
open_error:
	shmdt(font_buffer);
shmat_error:
	shmctl(shm_font, IPC_RMID, NULL);
	return 0;
}

void font_release(void)
{
	shmdt(font_buffer);
	shmctl(shm_font, IPC_RMID, NULL);
}
