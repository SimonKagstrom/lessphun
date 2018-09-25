/*********************************************************************
 *
 * Copyright (C) 2004,  Blekinge Institute of Technology
 *
 * Filename:      utils.h
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:   Util defs.
 *
 * $Id: utils.h 11057 2006-09-15 15:32:00Z ska $
 *
 ********************************************************************/
#ifndef __UTILS_H__
#define __UTILS_H__

#ifndef abs
# define abs(x) ( (x) < 0 ? -(x) : (x) )
#endif

#ifndef max
# define max(x, y) ( (x) > (y) ? (x) : (y) )
#endif

#ifndef min
# define min(x, y) ( (x) < (y) ? (x) : (y) )
#endif

static inline int sign(int x)
{
  if (x < 0)
    return -1;
  if (x > 0)
    return 1;
  return 0;
}

static inline int get_manhattan_dist(int x1, int y1, int x2, int y2)
{
  return abs(x2-x1) + abs(y2-y1);
}

int get_screen_size(int32_t *p_w, int32_t *p_h);
int bresenham(int x0, int y0, int x1, int y1, int (*callback)(int x, int y, void *p), void *p_callback_arg);
SPRITE **unpack_sprite_frames(SPRITE *src, int n_sprites);
void print_font(VMGPFONT *p_font, uint32_t color, int32_t x, int32_t y, char *msg);

#endif /* !__UTILS_H__ */
