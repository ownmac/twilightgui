#ifndef _FONT_H
#define _FONT_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include "common.h"

#define FONT_FILE "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc"
#define SHM_FONT_KEY 12345

int font_init(void);
void font_release(void);

#endif
