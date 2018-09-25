/*********************************************************************
 *
 * Copyright (C) 2004,  Blekinge Institute of Technology
 *
 * Filename:      tiles.h
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:   Tiles
 *
 * $Id: tiles.h 11300 2006-09-26 19:34:28Z ska $
 *
 ********************************************************************/
#ifndef __TILES_H__
#define __TILES_H__

#include <types.h>
#include <utils.h>

#define TILE_W   16
#define TILE_H   16

#define TILE_UNDISCOVERED      (tile_t)1
#define TILE_EMPTY             (tile_t)2
#define TILE_DIRT              (tile_t)3
#define TILE_BOMBS             (tile_t)4
#define TILE_MAGIC_WALL        (tile_t)5
#define TILE_LEFT_TRANSPORT    (tile_t)6
#define TILE_RIGHT_TRANSPORT   (tile_t)7
#define TILE_IRON_KEY          (tile_t)8
#define TILE_GOLD_KEY          (tile_t)9
#define TILE_RED_KEY          (tile_t)10
#define TILE_WOODEN_DOOR      (tile_t)11
#define TILE_IRON_DOOR        (tile_t)12
#define TILE_RED_DOOR         (tile_t)13
#define TILE_STONE_WALL       (tile_t)14
#define TILE_WEAK_STONE_WALL  (tile_t)15
#define TILE_TELEPORTER       (tile_t)16 /* Animated */
#define TILE_EXIT             (tile_t)18 /* Animated */
#define TILE_CONVEYOR_LEFT    (tile_t)20 /* Animated */
#define TILE_CONVEYOR         (tile_t)22 /* Animated */
#define TILE_CONVEYOR_RIGHT   (tile_t)24 /* Animated */

#define TILE_MAX              (TILE_CONVEYOR_RIGHT+1)

#define TILE_BOULDER          (tile_t)255
#define TILE_DIAMOND          (tile_t)254
#define TILE_GHOST            (tile_t)253
#define TILE_BLOCK            (tile_t)252
#define TILE_USER0            (tile_t)220
#define TILE_USER1            (tile_t)221
#define TILE_USER2            (tile_t)222
#define TILE_USER3            (tile_t)223
#define TILE_USER4            (tile_t)224
#define TILE_USER5            (tile_t)225

#define TILE_ANIMATE(x)       (tile_t)(0x4000 | ((x)<<8))

#define N_USER_TILES         6

#define MASK_TILE_IS_EMPTY(x) ((x).id == MASK_ID_EMPTY && !IS_SET((x).attr, MASK_ATTR_PLAYER))
#define MASK_TILE_IS_EMPTY_OR_PLAYER(x) ((x).id == MASK_ID_EMPTY)
#define MASK_TILE_IS_EMPTY_OR_ELEM(x) ( MASK_TILE_IS_EMPTY(x) || MASK_TILE_IS_ELEM(x))
#define MASK_TILE_IS_OCCUPIED(x) !MASK_TILE_IS_EMPTY(x)
#define MASK_TILE_IS_PLAYER(x) (IS_SET((x).attr, MASK_ATTR_PLAYER))
#define MASK_TILE_IS_EXIT(x) (IS_SET((x).attr, MASK_ATTR_EXIT))
#define MASK_TILE_IS_ELEM(x) ((x).id < N_ELEMS)
#define MASK_TILE_IS_WALKABLE(x) ( MASK_TILE_IS_EMPTY(x) || (x).id == MASK_ID_DIRT )

#define MASK_TILE_IS_CONVEYOR(x) ( (x).id >= MASK_ID_CONVEYOR_LEFT && (x).id <= MASK_ID_CONVEYOR_RIGHT)

#define MASK_TILE_GET_CONVEYOR_DIR(x) ( (x).id <= TILE_CONVEYOR_RIGHT ? RIGHT : LEFT )

#define MASK_TILE_IS_USER(x) ((x).attr & (((1<<8)-1) ^ 3))
#define MASK_TILE_USER_IS_SET(x, i) ( (x).attr & (1<<( (i)+2 )) )


#define MASK_ATTR_PLAYER     (1<<0) /* The player is here */
#define MASK_ATTR_EXIT       (1<<1) /* This is the exit place */
#define MASK_ATTR_USER(x)    (1<<(2+(x)))
#define MASK_ATTR_USER0      MASK_ATTR_USER(0)
#define MASK_ATTR_USER1      MASK_ATTR_USER(1)
#define MASK_ATTR_USER2      MASK_ATTR_USER(2)
#define MASK_ATTR_USER3      MASK_ATTR_USER(3)
#define MASK_ATTR_USER4      MASK_ATTR_USER(4)
#define MASK_ATTR_USER5      MASK_ATTR_USER(5)

/* !MASK_TYPE_ELEM */
#define MASK_ID(x)               ((255-TILE_MAX) + ((x) & 0xff))
#define MASK_ID_EMPTY            MASK_ID(TILE_EMPTY)
#define MASK_ID_DIRT             MASK_ID(TILE_DIRT)
#define MASK_ID_STONE_WALL       MASK_ID(TILE_STONE_WALL)
#define MASK_ID_WEAK_STONE_WALL  MASK_ID(TILE_WEAK_STONE_WALL)
#define MASK_ID_EXIT             MASK_ID(TILE_EXIT)
#define MASK_ID_BOMBS            MASK_ID(TILE_BOMBS)
#define MASK_ID_MAGIC_WALL       MASK_ID(TILE_MAGIC_WALL)
#define MASK_ID_LEFT_TRANSPORT   MASK_ID(TILE_LEFT_TRANSPORT)
#define MASK_ID_RIGHT_TRANSPORT  MASK_ID(TILE_RIGHT_TRANSPORT)
#define MASK_ID_TELEPORTER       MASK_ID(TILE_TELEPORTER)
#define MASK_ID_WOODEN_DOOR      MASK_ID(TILE_WOODEN_DOOR)
#define MASK_ID_IRON_DOOR        MASK_ID(TILE_IRON_DOOR)
#define MASK_ID_RED_DOOR         MASK_ID(TILE_RED_DOOR)
#define MASK_ID_IRON_KEY         MASK_ID(TILE_IRON_KEY)
#define MASK_ID_GOLD_KEY         MASK_ID(TILE_GOLD_KEY)
#define MASK_ID_RED_KEY          MASK_ID(TILE_RED_KEY)
#define MASK_ID_CONVEYOR_LEFT    MASK_ID(TILE_CONVEYOR_LEFT)
#define MASK_ID_CONVEYOR         MASK_ID(TILE_CONVEYOR)
#define MASK_ID_CONVEYOR_RIGHT   MASK_ID(TILE_CONVEYOR_RIGHT)

#define TILE_ID(tile) ( (tile) & 0xff )

#endif /* !__TILES_H__ */
