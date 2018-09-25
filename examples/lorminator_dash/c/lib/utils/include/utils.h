/*********************************************************************
 *
 * Copyright (C) 2004,  Blekinge Institute of Technology
 *
 * Filename:      utils.h
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:   Util defs.
 *
 * $Id: utils.h 11645 2006-10-16 13:22:26Z ska $
 *
 ********************************************************************/
#ifndef __UTILS_H__
#define __UTILS_H__

#include <lessphun.h>

#ifndef abs
# define abs(x) ( (x) < 0 ? -(x) : (x) )
#endif

#ifndef max
# define max(x, y) ( (x) > (y) ? (x) : (y) )
#endif

#ifndef min
# define min(x, y) ( (x) < (y) ? (x) : (y) )
#endif

#define TRUE 1
#define FALSE 0

#define IS_SET(x, val) ( (x) & val )
#define SET_BITS(x, val) ( (x) |= (val) )
#define SET_BIT_NR(x, nr) ( (x) | (1<<(nr)) )
#define CLEAR_BIT_NR(x, nr) ( (x) | ~(1<<(nr)) )
#define CLEAR_BITS(x, val) ( (x) &= ~(val) )

#ifndef NDEBUG
#  define debug_msg(x...) do {DbgPrintf(x); } while(0)
#else
#  define debug_msg(x...) do {DbgPrintf(x); } while(0)
#endif /* NDEBUG */
#define error_msg(x...) do {debug_msg(x); DbgBreak(); vTerminateVMGP();} while(0)

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
void *get_resource(char *filename, uint32_t size);
void print_font(VMGPFONT *p_font, uint32_t color, int32_t x, int32_t y, const char *msg);
void store_midifile(const char *filename, uint8_t *p_data, uint32_t size);
void play_midifile(const char *filename);

#endif /* !__UTILS_H__ */
