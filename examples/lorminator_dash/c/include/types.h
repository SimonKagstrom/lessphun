/*********************************************************************
 *
 * Copyright (C) 2004,  Blekinge Institute of Technology
 *
 * Filename:      types.h
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:   Typedefs
 *
 * $Id: types.h 11348 2006-09-28 08:53:56Z ska $
 *
 ********************************************************************/
#ifndef __TYPES_H__
#define __TYPES_H__

typedef uint16_t tile_t;

typedef struct
{
  uint8_t id;
  uint8_t attr;
} mask_tile_t;

#endif /* !__TYPES_H__ */
