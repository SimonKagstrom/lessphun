/*********************************************************************
 *
 * Copyright (C) 2004,  Blekinge Institute of Technology
 *
 * Filename:      specials.c
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:   Specials implementation
 *
 * $Id: specials.c 11188 2006-09-22 11:08:01Z ska $
 *
 ********************************************************************/
#include <boulder.h>
#include <levels.h>
#include <specials.h>

/* Generic special for explosions */
void special_explosion(game_t *p_game, uint8_t special)
{
  special_t *p_spec = &p_game->p_cur_level->p_specials[special];
  explosion_priv_t *p_priv = (explosion_priv_t*)p_spec->p_priv;
  int i;

  if (p_spec->triggered)
    return;

  for (i=0; i<p_priv->n_explosions; i++)
    {
      assert(p_priv->pt[i].x >= 0 && p_priv->pt[i].x < p_game->p_cur_level->w &&
	     p_priv->pt[i].y >= 0 && p_priv->pt[i].y < p_game->p_cur_level->h);

      explode(p_game, p_priv->pt[i].x, p_priv->pt[i].y, 1);
    }
  p_spec->triggered = TRUE;
}

/* Generic "growing-wall" special (callback) */
static void growing_wall_callback(game_t *p_game, int id, void *p_priv_in)
{
  growing_wall_priv_t *p_priv = (growing_wall_priv_t*)p_priv_in;
  point_t new_pt = pt_add_dir(p_priv->pt, get_dx(p_priv->dir));
  mask_tile_t *p_mask_tile;
  tile_t *p_tile;

  /* Reduce the speed */
  if ( !(p_priv->toggle = !p_priv->toggle) )
    return;

  assert(pt_in_rect(new_pt, pt(0,0), pt(p_game->p_cur_level->w, p_game->p_cur_level->h)));

  p_tile = &p_game->p_cur_level->p_level_data[new_pt.y * p_game->p_cur_level->w + new_pt.x];
  p_mask_tile = &MTILE_AT(p_game, new_pt.x, new_pt.y);

  if (!MASK_TILE_IS_WALKABLE(*p_mask_tile))
    {
      status_enqueue_message(p_game, "THE ROARING STOPS");
      callback_unregister(p_game, p_priv->id);
      return;
    }

  p_priv->pt = new_pt;
  *p_tile = p_priv->tile;
  p_mask_tile->id = MASK_ID(p_priv->tile);
}

/* Generic "growing-wall" special (trigger) */
void special_growing_wall(game_t *p_game, uint8_t special)
{
  special_t *p_spec = &p_game->p_cur_level->p_specials[special];
  growing_wall_priv_t *p_priv = (growing_wall_priv_t*)p_spec->p_priv;

  if (p_spec->triggered)
    return;

  p_spec->triggered = TRUE;
  /* Register the callback */
  if ( (p_priv->id = callback_register(p_game, growing_wall_callback, (void*)p_priv)) < 0)
    debug_msg("Could not register callback!\n");
  status_enqueue_message(p_game, "YOU HEAR A ROARING");
}

/* Generic "elem generator" special (callback) */
static void elem_generator_callback(game_t *p_game, int id, void *p_priv_in)
{
  elem_generator_priv_t *p_priv = (elem_generator_priv_t*)p_priv_in;
  point_t new_pt = pt_add_dir(p_priv->pt, p_priv->dir);
  mask_tile_t *p_mask_tile;
  tile_t *p_tile;

  /* Reduce the speed */
  if ( !(p_priv->toggle = !p_priv->toggle) )
    return;

  assert(pt_in_rect(new_pt, pt(0,0), pt(p_game->p_cur_level->w, p_game->p_cur_level->h)));

  p_tile = &p_game->p_cur_level->p_level_data[new_pt.y * p_game->p_cur_level->w + new_pt.x];
  p_mask_tile = &MTILE_AT(p_game, new_pt.x, new_pt.y);

  if (!MASK_TILE_IS_EMPTY(*p_mask_tile))
    return;
  /* This is the last entry */
  if (p_priv->n_elems-- <= 0)
    {
      callback_unregister(p_game, p_priv->id);
      return;
    }
  elem_add(p_game, p_priv->elem_type, pt_to_sprite(new_pt), p_priv->elem_extra_arg);
}

/* "Elem generator" special */
void special_elem_generator(game_t *p_game, uint8_t special)
{
  special_t *p_spec = &p_game->p_cur_level->p_specials[special];
  elem_generator_priv_t *p_priv = (elem_generator_priv_t*)p_spec->p_priv;

  if (p_spec->triggered)
    return;

  p_spec->triggered = TRUE;
  /* Register the callback */
  if ( (p_priv->id = callback_register(p_game, elem_generator_callback, (void*)p_priv)) < 0)
    debug_msg("Could not register callback!\n");
  status_enqueue_message(p_game, "A STRANGE SOUND IS HEARD!");
}

/* Generic special initialization (dirt, for "traps") */
bool_t special_init_generic(game_t *p_game, uint8_t special,
			    tile_t *p_tile, mask_tile_t *p_mask_tile, int16_t x, int16_t y)
{
  *p_tile = TILE_DIRT;
  p_mask_tile->id = MASK_ID_DIRT;
  p_mask_tile->attr |= MASK_ATTR_USER(special);
  return TRUE;
}
