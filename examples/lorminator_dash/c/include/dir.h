/*********************************************************************
 *
 * Copyright (C) 2004,  Blekinge Institute of Technology
 *
 * Filename:      dir.h
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:   Direction + handling
 *
 * $Id: dir.h 11276 2006-09-26 12:00:43Z ska $
 *
 ********************************************************************/
#ifndef __DIR_H__
#define __DIR_H__

typedef enum
{
  UP    = 0,
  DOWN  = 1,
  LEFT  = 2,
  RIGHT = 3,
  IDLE  = 4,
} dir_t;

static inline int16_t get_dx(const dir_t dir)
{
  if (dir == LEFT)
    return -1;
  if (dir == RIGHT)
    return 1;
  return 0;
}

static inline int16_t get_dy(const dir_t dir)
{
  if (dir == UP)
    return -1;
  if (dir == DOWN)
    return 1;
  return 0;
}


#endif /* !__DIR_H__ */
