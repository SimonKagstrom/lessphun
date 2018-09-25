/*********************************************************************
 *
 * Copyright (C) 2004,  Blekinge Institute of Technology
 *
 * Filename:      level_handling.c
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:   Level-related stuff
 *
 * $Id: level_handling.c 11360 2006-09-28 13:06:35Z ska $
 *
 ********************************************************************/
#include <boulder.h>

#define le_to_be16(x) ( ((x) & 0xff00) >> 8) | ( ((x) & 0xff) << 8)

int level_unpack(game_t *p_game, level_t *p_level)
{
  uint16_t *tmp = (uint16_t*)get_resource(p_level->u.filename, p_level->res_packed_size);
  int i;

  /* Byte swap */
  for (i=0; i < p_level->res_packed_size / sizeof(int16_t); i++)
    tmp[i] = le_to_be16(tmp[i]);

  p_level->p_level_data = tmp;

  return 0;
}

static level_t *level_alloc(game_t *p_game, level_t *p_level)
{
  level_t *p_out;
  int32_t size = sizeof(level_t);
  int n_teleporters = 0;
  int i;

  for (i=0; i<p_level->n_specials; i++)
    size += p_level->p_specials[i].sizeof_special;
  if ( !(p_out = vNewPtr( size )) )
    error_msg("vNewPtr failed");

  *p_out = *p_level;
  if (p_level->p_specials)
    {
      uint8_t *p_cur;

      p_out->p_specials = (special_t*)( ((teleporter_t*)(p_out + 1)) + n_teleporters);
      p_cur = (uint8_t*)(p_out->p_specials + p_out->n_specials);

      for (i=0; i<p_out->n_specials; i++)
	{
	  p_out->p_specials[i] = p_level->p_specials[i];
	  p_out->p_specials[i].p_priv = (void*)p_cur;
	  memcpy(p_cur, p_level->p_specials[i].p_priv, p_level->p_specials[i].sizeof_special - sizeof(special_t));
	  p_cur += p_level->p_specials[i].sizeof_special - sizeof(special_t);
	}
    }

  if (level_unpack(p_game, p_out) < 0)
    return NULL;

  /* Count the number of teleporters */
  for (i=0; i<p_out->w*p_out->h; i++)
    {
      if (p_out->p_level_data[i] == TILE_TELEPORTER)
	n_teleporters++;
    }
  if (n_teleporters > 0)
    {
      if ( !(p_out->p_teleporters = vNewPtr(n_teleporters * sizeof(teleporter_t))) )
	error_msg("vNewPtr failed!\n");
    }
  else
    p_out->p_teleporters = NULL;

  /* Allocate the level mask */
  if ( !(p_game->p_level_mask = vNewPtr( (p_level->w * p_level->h) * sizeof(mask_tile_t))) )
    error_msg("vNewPtr failed");
  memset(p_game->p_level_mask, 0, (p_level->w * p_level->h) * sizeof(mask_tile_t));
  /* Allocate the view mask */
  if ( !(p_game->p_view_data = vNewPtr( (p_level->w * p_level->h) * sizeof(tile_t))) )
    error_msg("vNewPtr failed");
  memset(p_game->p_view_data, TILE_UNDISCOVERED, (p_level->w * p_level->h) * sizeof(tile_t));

  return p_out;
}

void level_free(game_t *p_game, level_t *p_level)
{
  if (p_level)
    {
      if (p_level->p_level_data)
	vDisposePtr(p_level->p_level_data);
      if (p_level->p_teleporters)
	vDisposePtr(p_level->p_teleporters);
      vDisposePtr(p_level);
    }

  if (p_game->p_level_mask)
    vDisposePtr(p_game->p_level_mask);
  if (p_game->p_view_data)
    vDisposePtr(p_game->p_view_data);
}

/* Goto a level */
void game_goto_level(game_t *p_game, level_t *p_level)
{
  int16_t x,y;
  MAP_HEADER *p_bgmap = &p_game->bgmap;

  /* Dispose the old map */
  vMapDispose();

  memset(p_bgmap, 0, sizeof(MAP_HEADER));
  memset(p_game->elems, 0, sizeof(elem_t)*N_ELEMS);
  memset(p_game->callbacks, 0, sizeof(callback_t)*N_CALLBACKS);
  status_clear_messages(p_game);

  /* Set current level and allocate the level mask */
  if (p_game->p_cur_level)
    level_free(p_game, p_game->p_cur_level);
  p_game->p_cur_level = level_alloc(p_game, p_level);

  /* Initialise the background map */
  p_bgmap->width = p_level->w;
  p_bgmap->height = p_level->h;
  p_bgmap->mapoffset = (uint8_t*)p_game->p_view_data;
  p_bgmap->tiledata = (uint8_t*)p_game->p_tiles;
  p_bgmap->flag = 0;//MAP_USERATTRIBUTE|MAP_AUTOANIM;
  p_bgmap->format = BG_TILES_FORMAT;    /* the format of the bitmaps */

  p_game->n_elems = 0;
  p_game->first_free = 0;

  /* Place the player */
  player_init(p_game, &p_game->player, p_level->player_pt);
  SET_BITS(MTILE_AT(p_game, p_level->player_pt.x / TILE_W, p_level->player_pt.y / TILE_H).attr, MASK_ATTR_PLAYER);
  p_game->player.diamonds = 0;
  p_game->player.bombs = 0;
  p_game->player.bombs = 0;
  memset(p_game->player.keys, 0, sizeof(p_game->player.keys));

  /* Allocate the elems and setup the tilemap */
  for (y = 0; y < p_level->h; y++)
    {
      for (x = 0; x < p_level->w; x++)
	{
	  tile_t *p_tile = &p_game->p_cur_level->p_level_data[ y * p_level->w + x];
	  mask_tile_t *p_mask_tile = &MTILE_AT(p_game, x,y);

	  switch(*p_tile)
	    {
	    case TILE_USER0 ... TILE_USER5:
	      assert(*p_tile-TILE_USER0 < p_level->n_specials &&
		     *p_tile-TILE_USER0 < N_USER_TILES);
	      assert(p_game->p_cur_level->init_special_fn);

	      p_game->p_cur_level->init_special_fn(p_game, *p_tile - TILE_USER0, p_tile, p_mask_tile, x, y);
	      break;
	    case TILE_BOULDER:
	      elem_add(p_game, BOULDER, pt_to_sprite(pt(x, y)), NULL); break;
	    case TILE_DIAMOND:
	      elem_add(p_game, DIAMOND, pt_to_sprite(pt(x, y)), NULL); break;
	    case TILE_GHOST:
	      elem_add(p_game, GHOST, pt_to_sprite(pt(x, y)), NULL); break;
	    case TILE_BLOCK:
	      elem_add(p_game, BLOCK, pt_to_sprite(pt(x, y)), NULL); break;
	    case TILE_TELEPORTER:
	      *p_tile |= TILE_ANIMATE(1);
	      p_game->p_cur_level->p_teleporters[p_game->p_cur_level->n_teleporters].pt.x = x;
	      p_game->p_cur_level->p_teleporters[p_game->p_cur_level->n_teleporters].pt.y = y;
	      p_game->p_cur_level->n_teleporters++;
	      p_mask_tile->id = MASK_ID(*p_tile);
	      break;
	    case TILE_EXIT:
	      p_mask_tile->id = MASK_ID_DIRT;
	      *p_tile = TILE_DIRT;
	      p_game->exit_x = x;
	      p_game->exit_y = y;
	      break;
	    case TILE_CONVEYOR_LEFT ... TILE_CONVEYOR_RIGHT:
	      *p_tile |= TILE_ANIMATE(3);
	    default:
	      p_mask_tile->id = MASK_ID(*p_tile); break;
	    }
	}
    }

  if (p_game->p_cur_level->n_teleporters == 1)
    error_msg("Need more or less than one teleporter!\n");

  /* Setup the background map */
  if ( !vMapInit(p_bgmap) )
    error_msg("vMapInit failed");

  vSetDisplayWindow(max(screen_w, p_level->w*TILE_W), max(screen_h, p_level->h*TILE_H));
  p_game->start_ticks = vGetTickCount();
}

#if 0
level_t *level_set_load_level(game_t *p_game, level_set_t *p_set, int level_nr)
{
  file_section_t *p_level_sec;
  file_section_t *p_level_data_sec;
  level_t *p_out;

  if ( !(p_level_sec = file_section_get(p_set->handle, p_set->p_sectab,
					p_set->p_set->u.level_set.level_idx[level_nr])) )
    goto clean_0;
  if ( !(p_level_data_sec = file_section_get(p_set->handle, p_set->p_sectab,
					     p_level_sec->u.level.level_data_sect)) )
    goto clean_1;
  if ( !(p_out = vNewPtr(sizeof(level_t))) )
    goto clean_2;

  /* Initialize the level */
  p_out->w = p_level_data_sec->u.level_data.w;
  p_out->h = p_level_data_sec->u.level_data.h;
  p_out->diamonds_required = p_level_sec->u.level.diamonds_required;
  p_out->n_specials = p_level_sec->u.level.n_specials;
  p_out->player_pt = p_level_sec->u.level.player_pt;
  p_out->time = p_level_sec->u.level.time;
  p_out->n_teleporters = 0;

  if ( !(p_out->p_level_data = vNewPtr(p_out->w * p_out->h * sizeof (tile_t))) )
    goto clean_3;
  memcpy(p_out->p_level_data, p_level_data_sec->u.level_data.data, p_out->w * p_out->h * sizeof (tile_t));
  /* FIXME! Allocate and read specials */

  vDisposePtr(p_level_data_sec);
  vDisposePtr(p_level_sec);

  return p_out;

 clean_3:
  vDisposePtr(p_out);
 clean_2:
  vDisposePtr(p_level_data_sec);
 clean_1:
  vDisposePtr(p_level_sec);
 clean_0:
  return NULL;

}


bool_t level_set_init(game_t *p_game, level_set_t *p_out, int level_set_nr, file_handle_t handle)
{
  p_out->level_set_idx = 0;
  p_out->p_hdr = file_read_header(handle);
  p_out->handle = handle;

  if (!p_out->p_hdr ||
      p_out->p_hdr->n_level_sets <= level_set_nr)
    {
      debug_msg("Error in reading file header (%d:%d)!\n",
		p_out->p_hdr ? p_out->p_hdr->n_level_sets : 0, level_set_nr);
      goto clean_0;
    }

  if ( !(p_out->p_sectab = file_get_section_table(handle, p_out->p_hdr)) )
    goto clean_1;
  if ( !(p_out->p_set = file_section_get(handle, p_out->p_sectab,
					 p_out->p_hdr->level_set_idx[p_out->level_set_idx])) )
    goto clean_2;

  return TRUE;

 clean_2:
  vDisposePtr(p_out->p_sectab);
 clean_1:
  vDisposePtr(p_out->p_hdr);
 clean_0:

  return FALSE;
}

void level_set_fini(game_t *p_game, level_set_t *p_set)
{
  vDisposePtr(p_set->p_sectab);
  vDisposePtr(p_set->p_hdr);
}


file_handle_t level_set_open_file(game_t *p_game, const char *filename)
{
  file_handle_t out;

  if ( (out = vStreamOpen(filename, STREAM_FILE | STREAM_READ )) < 0)
    debug_msg("vStreamOpen failed!\n");

  return out;
}

file_handle_t level_set_open_resource(game_t *p_game, int32_t id)
{
  file_handle_t out;

  if ( (out = vStreamOpen((const char*)NULL, (id << 16) | STREAM_RESOURCE | STREAM_READ )) < 0)
    debug_msg("vStreamOpen failed!\n");

  return out;
}

void level_set_close(game_t *p_game, file_handle_t handle)
{
  vStreamClose(handle);
}
#endif
