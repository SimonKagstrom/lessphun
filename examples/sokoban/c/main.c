/*********************************************************************
 *
 * Copyright (C) 2004,  Blekinge Institute of Technology
 *
 * Filename:      main.c
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:   Sokoban game (mophun).
 *
 *                See http://www.cs.ualberta.ca/~games/Sokoban/ for
 *                information about the game of Sokoban.
 *
 * $Id: main.c 11402 2006-09-30 11:14:55Z ska $
 *
 ********************************************************************/
#include <lessphun.h>
#include "res.h"
#include "sokoban.h"
#include "levels.h"

/* Global variables (uach!) */
static int32_t screen_w, screen_h;

/* --- Function prototypes --- */
static void get_screen_size(int32_t *p_w, int32_t *p_h);
static uint32_t wait_for_keypress(void);
static void game_goto_level(game_t *p_game, level_t *p_level, MAP_HEADER *p_bgmap);
static void game_draw(game_t *p_game);
static int ball_move(game_t *p_game, ball_t *p_ball, int16_t dx, int16_t dy);
static void ball_draw(game_t *p_game, ball_t *p_ball);
static int player_move(game_t *p_game, player_t *p_player, int16_t dx, int16_t dy);
static void player_draw(game_t *p_game, player_t *p_player);


/* --- Some helpers --- */
/* Get the screen size, from 2dbackground.c */
static void get_screen_size(int32_t *p_w, int32_t *p_h)
{
  VIDEOCAPS videocaps;

  videocaps.size = sizeof(VIDEOCAPS);

  if(vGetCaps(CAPS_VIDEO, &videocaps))
    {
      *p_w = videocaps.width;
      *p_h = videocaps.height;
    }
  else
    vTerminateVMGP(); /* We are in deep shit if we cannot get the screen size */
}

/* Wait until some key is pressed */
static uint32_t wait_for_keypress(void)
{
  uint32_t out;

  while ( (out = vGetButtonData()) == 0);

  return out;
}


/* --- Game functionality --- */
/* Init the game */
static void game_init(game_t *p_game)
{
  p_game->player.sprite_slot = 8; /* Sprite slot for the player (after the last ball) */
}

/* Goto a game level */
static void game_goto_level(game_t *p_game, level_t *p_level, MAP_HEADER *p_bgmap)
{
  int i;

  memset(p_bgmap, 0, sizeof(MAP_HEADER));

  /* Initialise the background map */
  p_bgmap->width = p_level->w;
  p_bgmap->height = p_level->h;
  p_bgmap->mapoffset = p_level->p_level_data;
  p_bgmap->tiledata = BG_TILES;
  p_bgmap->format = BG_TILES_FORMAT;    /* the format of the bitmaps */

  /* Place the player */
  p_game->player.x = p_level->player_x;
  p_game->player.y = p_level->player_y;

  /* Setup the background map */
  if ( !vMapInit(p_bgmap) )
    vTerminateVMGP(); /* Oh no! */

  /* Init the game structure and copy the balls from the level */
  p_game->taken_balls = 0;
  p_game->p_level = p_level;
  for (i=0; i<p_level->n_balls; i++)
    {
      memcpy(&p_game->balls[i], &p_level->p_balls[i], sizeof(ball_t));
      p_game->balls[i].sprite_slot = i;
    }

  /* TRICK: move the player dx=0, dy=0 to center the map around the player */
  player_move(p_game, &p_game->player, 0, 0);
}

/* Draw the entire game */
static void game_draw(game_t *p_game)
{
  level_t *p_level = p_game->p_level;
  int i;

  /* Place the background */
  vMapSetXY(p_game->bg_x*8, p_game->bg_y*8);

  /* Place the sprites on the screen (or off it) */
  for (i=0; i<p_level->n_balls; i++)
    ball_draw(p_game, &p_game->balls[i]);
  player_draw(p_game, &p_game->player);

  /* Update sprites and the background */
  vUpdateSpriteMap();
}

/* Move a ball, returns 1 if the move is illegal */
static int ball_move(game_t *p_game, ball_t *p_ball, int16_t dx, int16_t dy)
{
  int16_t new_x = p_ball->x + dx;
  int16_t new_y = p_ball->y + dy;
  level_t *p_level = p_game->p_level;
  int i;

  /* We cannot move two balls at a time */
  for (i=0; i<p_level->n_balls; i++)
    {
      ball_t *p_curr_ball = &p_game->balls[i];

      /* Skip ourselves */
      if (p_ball == p_curr_ball)
	continue;
      if (p_curr_ball->x == new_x &&
	  p_curr_ball->y == new_y)
	return 1; /* Nope, cannot move two balls */
    }

  switch (p_level->p_level_data[new_y*p_level->w + new_x])
    {
    case WALL: /* Wall, don't move */
      return 1;
    case HOLE: /* Hole, one more ball taken (unless it was already on a hole) */
      if (p_level->p_level_data[p_ball->y*p_level->w + p_ball->x] != HOLE)
	p_game->taken_balls++;
      break;
    default:
      if (p_level->p_level_data[p_ball->y*p_level->w + p_ball->x] == HOLE)
	p_game->taken_balls--; /* Moved OFF a hole, one less taken! */
    }
  p_ball->x = new_x;
  p_ball->y = new_y;

  return 0;
}

/* Draw a ball */
static void ball_draw(game_t *p_game, ball_t *p_ball)
{
  vSpriteSet(p_ball->sprite_slot, &BALL_SPRITE, (p_ball->x-p_game->bg_x)*8, (p_ball->y-p_game->bg_y)*8);
}

/* Update the location of the sprite */
static int player_move(game_t *p_game, player_t *p_player, int16_t dx, int16_t dy)
{
  int16_t new_x = p_player->x + dx; /* Where the player tries to move */
  int16_t new_y = p_player->y + dy;
  level_t *p_level = p_game->p_level;
  int i;

  /* Push a ball? */
  for (i=0; i<p_level->n_balls; i++)
    {
      if (p_game->balls[i].x == new_x &&
	  p_game->balls[i].y == new_y)
	{
	  /* We are moving towards the ball - try to push it! */
	  if (ball_move(p_game, &p_game->balls[i], dx, dy))
	    return 1; /* Nope, not possible... */
	}
    }

  /* Stop the player if she hits the walls */
  switch (p_level->p_level_data[new_y*p_level->w + new_x])
    {
    case WALL: /* Don't move through walls! */
      break;
    default:
      p_player->x = new_x; /* Just walk otherwise */
      p_player->y = new_y;
    }

  /* Center the view around the player */
  p_game->bg_x = p_player->x - (screen_w / 8) / 2;
  if (p_game->bg_x < 0)
    p_game->bg_x = 0; /* Stop at (0,?) */
  else if (p_game->bg_x > p_level->w - screen_w/8)
    p_game->bg_x = p_level->w - screen_w/8; /* Stop at (screen_w, ?)*/

  p_game->bg_y = p_player->y - (screen_h / 8) / 2;
  if (p_game->bg_y < 0)
    p_game->bg_y = 0;
  else if (p_game->bg_y > p_level->h - screen_h/8)
    p_game->bg_y = p_level->h - screen_h/8;

  return 0;
}

/* Draw the player sprite */
static void player_draw(game_t *p_game, player_t *p_player)
{
  /* bg_x, bg_y is the background offset */
  vSpriteSet(p_player->sprite_slot, &PLAYER_SPRITE, (p_player->x-p_game->bg_x)*8, (p_player->y-p_game->bg_y)*8);
}


int main(int argc, char *argv[])
{
  MAP_HEADER bgmap;
  int should_exit = 0;
  game_t game;
  int curr_level = 0;

  LPH_init();

  /* Get the size of the screen */
  get_screen_size(&screen_w, &screen_h);

  game_init(&game);
  /* Init the first level */
  game_goto_level(&game, &levels[curr_level], &bgmap);

  /* Initialise the graphics */
  vClearScreen(vRGB(0,0,0));
  vSetClipWindow(0,0, screen_w, screen_h);
  vSetTransferMode(MODE_TRANS);

  /* Allocate sprite slots */
  if ( !vSpriteInit(9) )
    return 1; /* Oh no! */
  vSpriteClear();

  /* Draw everything */
  game_draw(&game);
  vFlipScreen(1);

  /* The main game loop */
  while(!should_exit)
    {
      uint32_t pre_ticks, post_ticks;
      uint32_t keys;

      /* Wait for a keypress */
      keys = wait_for_keypress();

      pre_ticks = vGetTickCount(); /* Get the current ms ticks */

      /* Move the player around */
      if (keys & KEY_LEFT)
	player_move(&game, &game.player, -1, 0);
      else if (keys & KEY_RIGHT)
	player_move(&game, &game.player, 1, 0);
      else if (keys & KEY_UP)
	player_move(&game, &game.player, 0, -1);
      else if (keys & KEY_DOWN)
	player_move(&game, &game.player, 0, 1);
      else if (keys & KEY_SELECT)
	should_exit = 1;

      /* Draw everything */
      game_draw(&game);
      vFlipScreen(1);

      /* If we have taken all balls - goto next level! */
      if (game.taken_balls >= game.p_level->n_balls)
	{
	  if (++curr_level < N_LEVELS)
	    game_goto_level(&game, &levels[curr_level], &bgmap);
	  else
	    break; /* This was the last level! */
	}

      post_ticks = vGetTickCount(); /* Get the current ms ticks */

      /* Every loop iteration should take about SLEEP_PERIOD, see to that */
      if ((post_ticks - pre_ticks) < SLEEP_PERIOD)
	msSleep( SLEEP_PERIOD - (post_ticks-pre_ticks) );
    }

  return 0;
}
