/*********************************************************************
 *
 * Copyright (C) 2004,  Blekinge Institute of Technology
 *
 * Filename:      elem.c
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:   Implementation of the elements
 *
 * $Id: elem.c 12029 2006-11-12 15:53:11Z ska $
 *
 ********************************************************************/
#include <boulder.h>


void boulder_init(game_t *p_game, elem_t *p_elem, point_t p)
{
  p_elem->type = BOULDER;
  sprite_init(&p_elem->sprite, p, FRAMES_BOULDER);
}

static void boulder_animate(game_t *p_game, elem_t *p_elem)
{
#if 0
  if (p_elem->falling)
    p_elem->sprite.frame = (p_elem->sprite.frame + 1) & 2;
#endif
}

void lever_init(game_t *p_game, elem_t *p_elem, point_t p, lever_state_t state)
{
  p_elem->type = LEVER;
  p_elem->u.lever.state = state;
  sprite_init(&p_elem->sprite, p, FRAMES_LEVER + state);
}

static void lever_animate(game_t *p_game, elem_t *p_elem)
{
  p_elem->sprite.frame = p_elem->u.lever.state;
}

void block_init(game_t *p_game, elem_t *p_elem, point_t p)
{
  p_elem->type = BLOCK;
  sprite_init(&p_elem->sprite, p, FRAMES_BLOCK);
}

void bomb_init(game_t *p_game, elem_t *p_elem, point_t p)
{
  p_elem->u.bomb.countdown = 20;
  p_elem->type = BOMB;
  sprite_init(&p_elem->sprite, p, FRAMES_BOMB);
}

void bomb_place(game_t *p_game, point_t in_pt)
{
  mask_tile_t *p_mask_tile = &MTILE_AT(p_game, in_pt.x, in_pt.y);

  if (p_game->player.bombs <= 0 ||
      !MASK_TILE_IS_WALKABLE(*p_mask_tile) ||
      (MASK_TILE_IS_ELEM(*p_mask_tile) &&
       p_game->elems[p_mask_tile->id].type == BOMB) )
    return;
  p_game->player.bombs--;

  elem_add(p_game, BOMB, pt_to_sprite(in_pt), NULL);
}

void flame_init(game_t *p_game, elem_t *p_elem, point_t p)
{
  p_elem->type = FLAME;
  p_elem->u.flame.countdown = 7 + (vGetRandom() & 3);
  sprite_init(&p_elem->sprite, p, FRAMES_FLAME);
}

static void flame_animate(game_t *p_game, elem_t *p_elem)
{
  p_elem->sprite.frame = vGetRandom() & 1;
}


void player_set_name(game_t *p_game, player_t *p_player, const char *p_name)
{
  int i;

  for (i=0; p_name[i] && i < NAME_LEN; i++)
    p_player->name[i] = p_name[i];
}

void player_init(game_t *p_game, player_t *p_player, point_t p)
{
  sprite_init(&p_player->sprite, p, FRAMES_PLAYER);
  p_player->sprite.dir = DOWN;
}

void player_draw(game_t *p_game, player_t *p_player)
{
  p_player->sprite.frame = p_player->sprite.dir;
  vDrawObject(p_player->sprite.pt.x-p_game->bg_x, p_player->sprite.pt.y-p_game->bg_y,
	      p_game->pp_sprite_frames[p_player->sprite.base_frame + p_player->sprite.frame]);
}

void player_kill(game_t *p_game, player_t *p_player)
{
  assert(p_player == &p_game->player);

  /* Player dies! */
  DbgPrintf("Player dies!!\n");

  if (p_player->active_delay == 0)
    {
      /* Restart the level if applicable */
      p_player->active_delay = 15;
    }
}

void diamond_init(game_t *p_game, elem_t *p_elem, point_t p)
{
  p_elem->type = DIAMOND;
  sprite_init(&p_elem->sprite, p, FRAMES_DIAMOND + (vGetRandom() % 3));
}

void ghost_init(game_t *p_game, elem_t *p_elem, point_t p)
{
  p_elem->type = GHOST;
  sprite_init(&p_elem->sprite, p, FRAMES_GHOST);
  p_elem->u.ghost.cur_dir = UP;
}

/* Move around the ghosts, randomly */
void ghost_move(game_t *p_game, elem_t *p_elem)
{
  point_t old_pt = p_elem->sprite.pt;
  mask_tile_t dirs[4];

  dirs[0] = MTILE_AT(p_game, p_elem->sprite.pt.x / TILE_W, p_elem->sprite.pt.y / TILE_H - 1); /* Up */
  dirs[1] = MTILE_AT(p_game, p_elem->sprite.pt.x / TILE_W, p_elem->sprite.pt.y / TILE_H + 1); /* Down */
  dirs[2] = MTILE_AT(p_game, p_elem->sprite.pt.x / TILE_W - 1, p_elem->sprite.pt.y / TILE_H); /* Left */
  dirs[3] = MTILE_AT(p_game, p_elem->sprite.pt.x / TILE_W + 1, p_elem->sprite.pt.y / TILE_H); /* Right */

  if (is_close_to_player(p_game, old_pt, LAMP_SHINE_RADIUS+1))
    status_enqueue_message(p_game, "YOU HEAR FOOTSTEPS");

  if (!MASK_TILE_IS_EMPTY_OR_PLAYER(dirs[p_elem->u.ghost.cur_dir]) ||
      ((vGetRandom() & 255) < 32))
    p_elem->u.ghost.cur_dir = vGetRandom() & 3;

  if (!MASK_TILE_IS_EMPTY_OR_PLAYER(dirs[p_elem->u.ghost.cur_dir]))
    return;

  sprite_move(&p_elem->sprite, p_elem->u.ghost.cur_dir);

  TILE_SWAP(p_game, old_pt.x / TILE_W, old_pt.y / TILE_H,
	    p_elem->sprite.pt.x / TILE_W, p_elem->sprite.pt.y / TILE_H);
}

elem_t *elem_add(game_t *p_game, elem_type_t type, point_t p, void *p_arg)
{
  mask_tile_t *p_mask_tile = &MTILE_AT(p_game, p.x/TILE_W, p.y/TILE_H);
  elem_t *p_elem;
  int i;

  assert(pt_in_rect(p, pt(0,0), pt_to_sprite(pt(p_game->p_cur_level->w, p_game->p_cur_level->h))) );
  assert( p_mask_tile->id != 0 ? !MASK_TILE_IS_ELEM(*p_mask_tile) : 1 );

  /* Find a free spot */
  for (i=p_game->first_free; i<N_ELEMS; i++)
    {
      if (!p_game->elems[i].is_active)
	{
	  p_game->first_free = i+1;
	  break;
	}
    }
  if (i >= N_ELEMS)
    {
      DbgPrintf("WARNING: Too many elements. %d at %d:%d!\n", type, p.x, p.y);
      return NULL;
    }
  if (i >= p_game->n_elems)
    p_game->n_elems = i+1;

  p_elem = &p_game->elems[i];

  switch(type)
    {
    case GHOST:
      ghost_init(p_game, p_elem, p); break;
    case BOULDER:
      boulder_init(p_game, p_elem, p); break;
    case DIAMOND:
      diamond_init(p_game, p_elem, p); break;
    case BOMB:
      bomb_init(p_game, p_elem, p); break;
    case BLOCK:
      block_init(p_game, p_elem, p); break;
    case FLAME:
      flame_init(p_game, p_elem, p); break;
    case LEVER:
      lever_init(p_game, p_elem, p, (lever_state_t)p_arg); break;
    default:
      error_msg("Unknown type %d\n", type);
    }
  p_elem->is_active = TRUE;

  p_mask_tile->id = i;
  p_game->p_cur_level->p_level_data[(p.y/TILE_H) * p_game->p_cur_level->w + (p.x/TILE_W)] = TILE_EMPTY;

  /* Assume this cannot be seen */
  p_elem->sprite.view_pt.x = -TILE_W;
  p_elem->sprite.view_frame = p_elem->sprite.base_frame;

  DbgPrintf("Added %d (%d:%d) at %d\n", type, p.x/TILE_W, p.y/TILE_H, i);

  return p_elem;
}

void elem_remove(game_t *p_game, elem_t *p_elem)
{
  int16_t x = p_elem->sprite.pt.x / TILE_W;
  int16_t y = p_elem->sprite.pt.y / TILE_H;

  if (p_elem->is_active)
    {
      mask_tile_t *p_mask_tile = &MTILE_AT(p_game,x,y);

      DbgPrintf("Removing %d:%d\n", p_elem->type, p_mask_tile->id);

      /* Use this as the first free if possible */
      if (p_game->first_free > p_mask_tile->id)
	p_game->first_free = p_mask_tile->id;
      if (p_game->n_elems == p_mask_tile->id + 1)
	p_game->n_elems--;

      p_mask_tile->id = MASK_ID_EMPTY;
      p_elem->is_active = FALSE;
    }
}

static bool_t elem_is_transformable(game_t *p_game, elem_t *p_elem)
{
  return (p_elem->type == BOULDER || p_elem->type == DIAMOND || p_elem->type == BOMB);
}

static void elem_transform(game_t *p_game, elem_t *p_elem, point_t new_pt)
{
  switch(p_elem->type)
    {
    case BOULDER:
      diamond_init(p_game, p_elem, new_pt); break;
    case DIAMOND:
      boulder_init(p_game, p_elem, new_pt); break;
    case BOMB:
      ghost_init(p_game, p_elem, new_pt); break;
    default:
      error_msg("Unknown transformation %d\n\n", p_elem->type);
      break;
    }
}

void elem_fall(game_t *p_game, elem_t *p_elem)
{
  uint16_t x = p_elem->sprite.pt.x / TILE_W;
  uint16_t y = p_elem->sprite.pt.y / TILE_H;
  mask_tile_t tile_south = MTILE_AT(p_game, x, y+1);

  assert(p_elem->is_active);

  if (p_elem->falling &&
      (tile_south.id == MASK_ID_BOMBS ||
       (MASK_TILE_IS_ELEM(tile_south) &&
	(p_game->elems[tile_south.id].type == GHOST)) ||
       (MASK_TILE_IS_PLAYER(tile_south) && p_game->player.active_delay <= 0)) ) /* .. or player */
    {
      explode(p_game, x, (y+1), 1);
      return;
    }

  /* The four cases when boulders fall:
   *
   *  1    2    3   4
   * #o#  #o#  #o   o#
   * # #  #M#  #o   o#
   *      # #
   *
   * o - boulder
   * # - wall
   * M - magic wall
   */
  if (MASK_TILE_IS_EMPTY(tile_south))
    {
      /* Case 1 */
      TILE_SWAP(p_game, x, y, x, y+1);
      sprite_move(&p_elem->sprite, DOWN);
      p_elem->falling = TRUE;
      return;
    }
  else if (tile_south.id == MASK_ID_MAGIC_WALL &&
	   elem_is_transformable(p_game, p_elem))
    {
      mask_tile_t far_south = MTILE_AT(p_game, x, y+2);

      /* Case 2 */
      if ( (MASK_TILE_IS_ELEM(far_south) &&
	    p_game->elems[far_south.id].type == GHOST) ||
	   MASK_TILE_IS_PLAYER(far_south))
	{
	  explode(p_game, x, (y+2), 1);
	  elem_remove(p_game, p_elem);
	  return; /* Needed - otherwise we will swap the flame with the empty space below */
	}
      else if (!MASK_TILE_IS_EMPTY(far_south))
	{
	  elem_remove(p_game, p_elem);
	  return;
	}

      elem_transform(p_game, p_elem, pt_to_sprite(pt(x,y+2)));
      TILE_SWAP(p_game, x, y, x, y+2);
    }
  else if (MASK_TILE_IS_ELEM(tile_south))
    {
      elem_t *p_elem_south = &p_game->elems[tile_south.id];

      if (p_elem_south->type == BOULDER || p_elem_south->type == DIAMOND)
	{
	  /* Case 3, 4 */
	  if ( MASK_TILE_IS_EMPTY(MTILE_AT(p_game, x+1, y)) &&
	       MASK_TILE_IS_EMPTY(MTILE_AT(p_game, x+1, y+1)) )
	    {
	      TILE_SWAP(p_game, x, y, x+1, y);
	      sprite_move(&p_elem->sprite, RIGHT);
	    }
	  else if ( MASK_TILE_IS_EMPTY(MTILE_AT(p_game, x-1, y)) &&
		    MASK_TILE_IS_EMPTY(MTILE_AT(p_game, x-1, y+1)) )
	    {
	      TILE_SWAP(p_game, x, y, x-1, y);
	      sprite_move(&p_elem->sprite, LEFT);
	    }
	}
    }

  p_elem->falling = FALSE;
}

int elem_push(game_t *p_game, elem_t *p_elem, dir_t dir)
{
  int16_t new_x = p_elem->sprite.pt.x + get_dx(dir) * TILE_W;
  int16_t new_y = p_elem->sprite.pt.y + get_dy(dir) * TILE_H;
  int16_t old_x = p_elem->sprite.pt.x;
  int16_t old_y = p_elem->sprite.pt.y;

  if (get_dy(dir) && (p_elem->type == BOULDER ||
		      p_elem->type == BOMB))
    return 0;

  if (MASK_TILE_IS_EMPTY(MTILE_AT(p_game, new_x / TILE_W, new_y / TILE_H)))
    {
      TILE_SWAP(p_game, new_x / TILE_W, new_y / TILE_H,
		old_x / TILE_W, old_y / TILE_H);
      p_elem->sprite.pt.x = new_x;
      p_elem->sprite.pt.y = new_y;
      p_game->p_cur_level->p_level_data[(new_y/TILE_H) * p_game->p_cur_level->w + (new_x/TILE_W)] = TILE_EMPTY;
      return 1;
    }

  return 0;
}

void elem_draw(game_t *p_game, elem_t *p_elem)
{
  int16_t frame;

  switch(p_elem->type)
    {
    case BOULDER:
      boulder_animate(p_game, p_elem); break;
    case FLAME:
      flame_animate(p_game, p_elem); break;
    case LEVER:
      lever_animate(p_game, p_elem); break;
    default:
      break;
    }

  if (p_elem->display_counter <= 0)
    frame = p_elem->sprite.view_frame + FIRST_HALFTONE;
  else
    frame = p_elem->sprite.view_frame = p_elem->sprite.base_frame + p_elem->sprite.frame;

  /* Move objects we know has moved out of sight (if they are not visible) */
  if (is_close_to_player(p_game, p_elem->sprite.view_pt, LAMP_SHINE_RADIUS) &&
      !is_close_to_player(p_game, p_elem->sprite.pt, LAMP_SHINE_RADIUS))
    p_elem->sprite.view_pt.x = -TILE_W;

  if (p_elem->sprite.view_pt.x >= 0)
    {
      vDrawObject(p_elem->sprite.view_pt.x-p_game->bg_x, p_elem->sprite.view_pt.y-p_game->bg_y,
		  p_game->pp_sprite_frames[frame]);
    }
}
