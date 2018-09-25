/*********************************************************************
 *
 * Copyright (C) 2004,  Blekinge Institute of Technology
 *
 * Filename:      sokoban.h
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:   Definitions for the sokoban game
 *
 * $Id: sokoban.h 9224 2006-08-04 10:23:21Z ska $
 *
 ********************************************************************/
#ifndef __SOKOBAN_H__
#define __SOKOBAN_H__

/* Ball structure */
typedef struct
{
  int16_t x; /* map position */
  int16_t y;
  uint8_t sprite_slot; /* Which sprite slot this ball has */
} ball_t;

/* A structure for the player */
typedef struct
{
  int16_t x; /* map position */
  int16_t y;
  uint8_t sprite_slot; /* The sprite slot of the player */
} player_t;

/* A level, see levels.h */
typedef struct
{
  uint8_t *p_level_data;
  uint8_t  w;
  uint8_t  h;
  uint8_t  player_x;
  uint8_t  player_y;
  ball_t  *p_balls; /* A pointer to the balls */
  int      n_balls; /* The number of balls */
} level_t;

/* The game */
typedef struct
{
  level_t  *p_level;
  player_t  player;
  ball_t    balls[8];
  int       taken_balls; /* How many balls do we have? */
  int16_t   bg_x;        /* The part of the level which is shown. */
  int16_t   bg_y;
} game_t;


/* The tile types */
#define FLOOR   1
#define WALL    2
#define HOLE    3

/* How long between the moves */
#define SLEEP_PERIOD 100

#endif /* !__SOKOBAN_H__ */
