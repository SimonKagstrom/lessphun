/*********************************************************************
 *
 * Copyright (C) 2004,  Blekinge Institute of Technology
 *
 * Filename:      main.c
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:   Mophun sprite test
 *
 * $Id: main.c 11103 2006-09-19 10:40:19Z ska $
 *
 ********************************************************************/
#include <lessphun.h>

#include "res.h"
#include "tilemap.h"
#include "utils.h"

#define N_FRAMES     2
#define N_DIRECTIONS 4


/* A structure for a sprite */
typedef struct
{
  int16_t   x;
  int16_t   y;
  int16_t   dir;
  uint8_t   frame;
  int16_t   sprite_slot;
  SPRITE  **pp_frames;
} sprite_t;

/* Animate the sprite (done on moves) */
static void sprite_animate(sprite_t *p_sprite)
{
  /*   _              0
   *  / |             ^
   * 0->1 (frame)  1<-+->3 (dir)
   *                  v
   *                  2
   */
  p_sprite->frame = ((p_sprite->frame+1) & (N_FRAMES-1)) + p_sprite->dir*(N_FRAMES);
}

/* Update the location of the sprite */
static void sprite_move(sprite_t *p_sprite, int16_t dx, int16_t dy)
{
  uint8_t x_move = (dx != 0) ? 1 : 0;
  uint8_t y_move = (dy != 0) ? 1 : 0;

  p_sprite->x += dx;
  p_sprite->y += dy;

  /* Assuming that we cannot move diagonally, this will give our direction.
   *
   * There are of course much simpler ways of setting the direction :-)
   */
  p_sprite->dir = (dy+1)*y_move + (dx+2)*x_move;
  sprite_animate(p_sprite);
}

static void sprite_draw(sprite_t *p_sprite)
{
  uint8_t frame = p_sprite->frame;

  vSpriteSet(p_sprite->sprite_slot, p_sprite->pp_frames[frame], p_sprite->x, p_sprite->y);
}

/* Set up the sprite frames */
static void setup_sprite_frames(sprite_t *p_sprite)
{
  /* See utils.c */
  if ( !(p_sprite->pp_frames = unpack_sprite_frames(&PLAYER_FRAMES, (N_FRAMES*N_DIRECTIONS))) )
    {
      DbgPrintf("unpack_sprite_frames failed!\n");
      vTerminateVMGP();
    }
}

#if 0
/* Another way, needs lots of BMP-files and wastes memory */
static void setup_sprite_frames(sprite_t *p_sprite)
{
  /* We assume this has been allocated (most likely is static) */
  p_sprite->pp_frames[0] = &PLAYER_SPRITE_FRAME0;
  p_sprite->pp_frames[1] = &PLAYER_SPRITE_FRAME1;
  p_sprite->pp_frames[2] = &PLAYER_SPRITE_FRAME2;
  p_sprite->pp_frames[3] = &PLAYER_SPRITE_FRAME3;
  p_sprite->pp_frames[4] = &PLAYER_SPRITE_FRAME4;
  p_sprite->pp_frames[5] = &PLAYER_SPRITE_FRAME5;
  p_sprite->pp_frames[6] = &PLAYER_SPRITE_FRAME6;
  p_sprite->pp_frames[7] = &PLAYER_SPRITE_FRAME7;
}
#endif


int main(int argc, char **argv)
{
  int32_t screen_w, screen_h;
  int should_exit = 0;
  sprite_t sprite;

  LPH_init();

  /* Init our sprite */
  sprite.x = 10;
  sprite.y = 15;
  sprite.dir = 0;
  sprite.frame = 0;
  sprite.sprite_slot = 0; /* Use the first slot */
  /* Allocate the player frames */
  setup_sprite_frames(&sprite);

  if (!sprite.pp_frames)
    {
      DbgPrintf("allocate_frames failed!\n");
      return 1;
    }

  /* Get the size of the screen */
  get_screen_size(&screen_w, &screen_h);

  if ( !vMapInit(&bgmap) )
    return 1;

  /* Initialise the graphics */
  vSetClipWindow(0,0,screen_w,screen_h); /* What can be displayed? */
  vClearScreen(vRGB(0,0,0));
  vSetTransferMode(MODE_TRANS);


  /* Allocate space for one sprite */
  if ( !vSpriteInit(1) )
    return 1;
  vSpriteClear();

  /* The main game loop */
  while(!should_exit)
    {
      int32_t keys = vGetButtonData();

      /* Change the viewpoint if a key was pressed */
      if (keys & KEY_LEFT)
	sprite_move(&sprite, -1, 0);
      else if (keys & KEY_RIGHT)
	sprite_move(&sprite, 1, 0);
      else if (keys & KEY_UP)
	sprite_move(&sprite, 0, -1);
      else if (keys & KEY_DOWN)
	sprite_move(&sprite, 0, 1);
      else if (keys & KEY_FIRE)
	break;

      sprite_draw(&sprite);

      /* Update sprites */
      vUpdateSpriteMap();
      /* Draw everything */
      vFlipScreen(1);
      msSleep(3);
    }

  vDisposePtr(sprite.pp_frames);

  return 0;
}
