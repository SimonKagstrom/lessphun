/*********************************************************************
 *
 * Copyright (C) 2004,  Blekinge Institute of Technology
 *
 * Filename:      utils.c
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:   Utils
 *
 * $Id: utils.c 11121 2006-09-20 10:17:51Z ska $
 *
 ********************************************************************/
#include <lessphun.h>

#include "utils.h"

/* Get the screen size, from 2dbackground.c */
int get_screen_size(int32_t *p_w, int32_t *p_h)
{
  VIDEOCAPS videocaps;

  videocaps.size = sizeof(VIDEOCAPS);

  if(vGetCaps(CAPS_VIDEO, &videocaps))
    {
      *p_w  = videocaps.width;
      *p_h = videocaps.height;
    }
  else
    return -1;

  return 0;
}


/* Bresenhams algorithm, from http://www.fact-index.com/b/br/bresenham_s_line_algorithm_c_code.html */
int bresenham(int x0, int y0, int x1, int y1,
	      int (*callback)(int x, int y, void *p), void *p_callback_arg)
{
  int steep = 1;
  int sx, sy;  /* step positive or negative (1 or -1) */
  int dx, dy;  /* delta (difference in X and Y between points) */
  int e;
  int i;

  /*
   * inline swap. On some architectures, the XOR trick may be faster
   */
  int tmpswap;
#define SWAP(a,b) tmpswap = a; a = b; b = tmpswap;

  /*
   * optimize for vertical and horizontal lines here
   */
  dx = abs(x1 - x0);
  sx = ((x1 - x0) > 0) ? 1 : -1;
  dy = abs(y1 - y0);
  sy = ((y1 - y0) > 0) ? 1 : -1;
  if (dy > dx)
    {
      steep = 0;
      SWAP(x0, y0);
      SWAP(dx, dy);
      SWAP(sx, sy);
    }
  e = (dy << 1) - dx;
  for (i = 0; i < dx; i++)
    {
      if (steep)
	{
	  if (callback(x0,y0, p_callback_arg))
	    return 1;
	}
      else
	{
	  if (callback(y0,x0, p_callback_arg))
	    return 1;
	}
      while (e >= 0)
	{
	  y0 += sy;
	  e -= (dx << 1);
	}
      x0 += sx;
      e += (dy << 1);
    }
#undef SWAP

  return 0;
}

void print_font(VMGPFONT *p_font, uint32_t color, int32_t x, int32_t y, char *msg)
{
  vSetForeColor(color);

  vSetActiveFont(p_font);
  vPrint(MODE_TRANS, x,y, msg);
}

/* One, general but more complex, way of doing this */
SPRITE **unpack_sprite_frames(SPRITE *src, int n_sprites)
{
  return LPH_splitSprites(src, n_sprites);
}
