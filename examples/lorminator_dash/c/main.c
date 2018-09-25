/*********************************************************************
 *
 * Copyright (C) 2004,  Blekinge Institute of Technology
 *
 * Filename:      main.c
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:   Boulder dash (mophun).
 *
 * $Id: main.c 13233 2007-01-27 11:16:54Z ska $
 *
 ********************************************************************/
#include <boulder.h>
#include <levels.h>

int32_t screen_w;
int32_t screen_h;
static game_t game; /* For phone calls */

static void player_handle_input(game_t *p_game, dir_t dir, uint32_t fire);
static void game_update_view_buffer(game_t *p_game);
static void game_erase_view_buffer(game_t *p_game);
static void handle_user_tile(game_t *p_game, mask_tile_t mask_tile);

static void game_check_all_elems(game_t *p_game);
static void game_init(game_t *p_game);
static void game_do(game_t *p_game);


static void teleport(game_t *p_game, player_t *p_player)
{
  mask_tile_t cur_tile = MTILE_AT(p_game, p_player->sprite.pt.x / TILE_W,
				  p_player->sprite.pt.y / TILE_H);
  int nr;

  /* Teleport! */
  do
    {
      nr = vGetRandom() % p_game->p_cur_level->n_teleporters;
    } while (pt_eq(p_player->sprite.pt, pt_to_sprite(p_game->p_cur_level->p_teleporters[nr].pt)));

  /* Erase the old buffer */
  game_erase_view_buffer(p_game);
  p_player->sprite.pt = pt_to_sprite(p_game->p_cur_level->p_teleporters[nr].pt);
  CLEAR_BITS(cur_tile.attr, MASK_ATTR_PLAYER);
}

static bool_t is_visible(mask_tile_t mtile)
{
  switch(mtile.id)
    {
    case MASK_ID_CONVEYOR_LEFT ... MASK_ID_CONVEYOR_RIGHT:
    case MASK_ID_STONE_WALL:
    case MASK_ID_WEAK_STONE_WALL:
    case MASK_ID_EXIT:
    case MASK_ID_MAGIC_WALL:
    case MASK_ID_LEFT_TRANSPORT ... MASK_ID_RIGHT_TRANSPORT:
    case MASK_ID_WOODEN_DOOR ... MASK_ID_RED_DOOR:
      return FALSE;
    default:
      break;
    }
  return TRUE;
}

static void handle_user_tile(game_t *p_game, mask_tile_t mask_tile)
{
  int i;

  for (i=0; i<N_USER_TILES; i++)
    {
      if (MASK_TILE_USER_IS_SET(mask_tile, i))
	{
	  assert(i < p_game->p_cur_level->n_specials);
	  p_game->p_cur_level->p_specials[i].fn(p_game, i);
	}
    }
}

void explode(game_t *p_game, int16_t x_in, int16_t y_in, int16_t r)
{
  level_t *p_level = p_game->p_cur_level;
  int16_t x,y;

  DbgPrintf("Explode %d:%d\n", x_in, y_in);
  status_enqueue_message(p_game, "YOU HEAR AN EXPLOSION");
  sound_play(p_game, EXPLOSION);

  for (y = y_in - r; y <= y_in + r; y++)
    {
      if ( y < 0 || y > p_level->h )
	continue;

      for (x = x_in - r; x <= x_in + r; x++)
	{
	  mask_tile_t *p_mask_tile;

	  /* Don't explode outside the playfield */
	  if ( x < 0 || x > p_level->w)
	    continue;

	  /* Explode! */
	  p_mask_tile = &MTILE_AT(p_game, x,y);
	  switch(p_mask_tile->id)
	    {
	    case MASK_ID_CONVEYOR_LEFT ... MASK_ID_CONVEYOR_RIGHT:
	    case MASK_ID_STONE_WALL:
	    case MASK_ID_EXIT:
	    case MASK_ID_MAGIC_WALL:
	    case MASK_ID_LEFT_TRANSPORT ... MASK_ID_RIGHT_TRANSPORT:
	    case MASK_ID_IRON_DOOR ... MASK_ID_RED_DOOR:
	      break;
	    default:
	      {
		if (MASK_TILE_IS_ELEM(*p_mask_tile))
		  {
		    if (ELEM_AT(p_game, *p_mask_tile).type != FLAME)
		      {
			elem_remove(p_game, &ELEM_AT(p_game, *p_mask_tile));
			elem_add(p_game, FLAME, pt_to_sprite(pt(x, y)), NULL);
		      }
		    else
		      flame_init(p_game, &ELEM_AT(p_game, *p_mask_tile), pt_to_sprite(pt(x, y)));
		  }
		else
		  elem_add(p_game, FLAME, pt_to_sprite(pt(x, y)), NULL);
		if (MASK_TILE_IS_PLAYER(*p_mask_tile))
		  player_kill(p_game, &p_game->player);
		break;
	      }
	    }
	}
    }
}

static void game_check_all_elems(game_t *p_game)
{
  int i;

  for (i=0; i<p_game->n_elems; i++)
    {
      elem_t *p_elem = &p_game->elems[i];
      point_t old_pt;
      mask_tile_t tile_south;

      /* Unused elems are skipped */
      if (!p_elem->is_active)
	continue;

      old_pt = pt_to_tile(p_elem->sprite.pt);
      tile_south = MTILE_AT(p_game, old_pt.x, old_pt.y+1);

      /* Conveyor belt active? */
      if (conveyor_belt_active(p_game) && MASK_TILE_IS_CONVEYOR(tile_south))
	{
	  elem_push(p_game, p_elem, MASK_TILE_GET_CONVEYOR_DIR(tile_south));
	  old_pt = p_elem->sprite.pt;
	}

      switch(p_elem->type)
	{
	case BOMB:
	  if (--p_elem->u.bomb.countdown <= 0)
	    explode(p_game, old_pt.x, old_pt.y, 1);
	  else /* Fall if possible */
	    goto fall;
	  break;
	case GHOST:
	  if (p_game->frame_count & 1)
	    ghost_move(p_game, p_elem);
	  break;
	case FLAME:
	  if (--p_elem->u.flame.countdown <= 0)
	    {
	      elem_remove(p_game, p_elem);
	      break; /* Otherwise it is inactive when it falls */
	    }
	  else if (MASK_TILE_IS_PLAYER(MTILE_AT(p_game, old_pt.x,old_pt.y)))
	    player_kill(p_game, &p_game->player);
	case BOULDER:
	case DIAMOND:
	  {
	  fall:
	    if (old_pt.y >= p_game->p_cur_level->h - 1)
	      break; /* Off the world */

	    elem_fall(p_game, p_elem); /* Try to fall! */
	    break;
	  }
	default:
	  break;
	}
      /* Decrease the display counter (i.e. sprite visible) */
      if (p_elem->display_counter > 0)
	{
	  p_elem->sprite.view_pt = p_elem->sprite.pt;
	  p_elem->display_counter--;
	}
      elem_draw(p_game, p_elem);
    }
}

static void player_handle_input(game_t *p_game, dir_t dir, uint32_t fire)
{
  point_t new_pt = pt_add_dir(pt_to_tile(p_game->player.sprite.pt), dir);
  point_t old_pt = pt_to_tile(p_game->player.sprite.pt);
  mask_tile_t *p_next_tile;
  mask_tile_t *p_old_tile;
  bool_t save_new_tile = FALSE;

  p_game->player.sprite.dir = dir;

 retry:
  /* Out of bounds, don't move */
  if (!pt_in_rect(new_pt, pt(0,0), pt(p_game->p_cur_level->w, p_game->p_cur_level->h)))
    return;

  p_next_tile = &MTILE_AT(p_game, new_pt.x, new_pt.y);

  switch (p_next_tile->id)
    {
    case MASK_ID_WOODEN_DOOR ... MASK_ID_RED_DOOR:
      {
	int needed_key = p_next_tile->id-MASK_ID_WOODEN_DOOR;

	if (!(fire & KEY_FIRE) ||
	    !p_game->player.keys[needed_key])
	  return;
	p_game->player.keys[needed_key] = FALSE;
	break;
      }
    case MASK_ID_MAGIC_WALL:
    case MASK_ID_WEAK_STONE_WALL:
    case MASK_ID_STONE_WALL:
    case MASK_ID_CONVEYOR_LEFT ... MASK_ID_CONVEYOR_RIGHT:
      return; /* Cannot move onto it */
    case MASK_ID_BOMBS:
      p_game->player.bombs += 4;
      break;
    case MASK_ID_IRON_KEY ... MASK_ID_RED_KEY:
      p_game->player.keys[p_next_tile->id - MASK_ID_IRON_KEY] = TRUE;
      break;
    case MASK_ID_TELEPORTER:
      save_new_tile = TRUE;
      break;
    case MASK_ID_LEFT_TRANSPORT ... MASK_ID_RIGHT_TRANSPORT:
      {
	mask_tile_t after_wall_tile;

	if (fire || dir != TRANSPORT_TO_DIR(p_next_tile->id))
	  return;
	after_wall_tile = MTILE_AT(p_game, new_pt.x + get_dx(dir), new_pt.y + get_dy(dir));

	if (MASK_TILE_IS_WALKABLE(after_wall_tile) ||
	    (MASK_TILE_IS_ELEM(after_wall_tile) && ELEM_AT(p_game, after_wall_tile).type == DIAMOND) )
	  {
	    new_pt.x += get_dx(dir);
	    goto retry;
	  }
	return;
      }
    case MASK_ID_DIRT:
    case MASK_ID_EMPTY:
      /* Walk right through these */
      break;
    default:
      {
	elem_t *p_elem = &p_game->elems[p_next_tile->id];

	assert(MASK_TILE_IS_ELEM(*p_next_tile));

	if (p_elem->type == BOULDER || p_elem->type == BLOCK ||
	    p_elem->type == BOMB)
	  {
	    /* Try to push the boulder */
	    if (!elem_push(p_game, p_elem, dir))
	      return; /* Could not push it */
	  }
	else if (p_elem->type == LEVER)
	  {
	    if (fire & KEY_FIRE)
	      {
		assert(p_game->p_cur_level->p_specials &&
		       p_elem->u.lever.special < p_game->p_cur_level->n_specials);

		p_elem->u.lever.state = !p_elem->u.lever.state;
		p_game->p_cur_level->p_specials[p_elem->u.lever.special].fn(p_game, p_elem->u.lever.special);
	      }
	    else
	      return;
	  }
	else if (p_elem->type == DIAMOND)
	  {
	    if (p_elem->falling)
	      break; /* We will explode later */
	    sound_play(p_game, DIAMOND_TAKEN);
	    p_game->player.diamonds++;
	    if (p_game->player.diamonds >= p_game->p_cur_level->diamonds_required)
	      {
		mask_tile_t *p_exit_tile = &MTILE_AT(p_game, p_game->exit_x, p_game->exit_y);

		SET_BITS(p_exit_tile->attr, MASK_ATTR_EXIT);
		p_exit_tile->id = MASK_ID_DIRT;
		p_game->p_cur_level->p_level_data[p_game->exit_y*p_game->p_cur_level->w + p_game->exit_x] =
		  TILE_EXIT | TILE_ANIMATE(1);

		DbgPrintf("Enough diamonds!");
	      }
	    elem_remove(p_game, p_elem);
	  }
      }
      break;
    }

  p_old_tile = &MTILE_AT(p_game, old_pt.x, old_pt.y);
  if (!save_new_tile ||
      (fire & KEY_FIRE) )
    {
      p_game->p_cur_level->p_level_data[new_pt.y*p_game->p_cur_level->w+new_pt.x] = TILE_EMPTY;
      vMapSetTile(new_pt.x, new_pt.y, TILE_EMPTY);

      /* Clean up after the player */
      if (!MASK_TILE_IS_ELEM(*p_next_tile))
	p_next_tile->id = MASK_ID_EMPTY;
    }
  /* Place bombs */
  if (fire & KEY_FIRE2)
    bomb_place(p_game, new_pt);
  if (!fire)
    {
      /* Check for user tiles */
      if (MASK_TILE_IS_USER(*p_next_tile))
	handle_user_tile(p_game, *p_next_tile);
      /* Clear the old tile */
      if (p_old_tile->id != MASK_ID_TELEPORTER)
	{
	  p_game->p_cur_level->p_level_data[old_pt.y*p_game->p_cur_level->w+old_pt.x] = TILE_EMPTY;
	  if (!MASK_TILE_IS_ELEM(*p_old_tile))
	    p_old_tile->id = MASK_ID_EMPTY;
	}

      /* Erase the old view buffer */
      game_erase_view_buffer(p_game);
      /* Move the player */
      CLEAR_BITS(p_old_tile->attr, MASK_ATTR_PLAYER);
      SET_BITS(p_next_tile->attr, MASK_ATTR_PLAYER);
      p_game->player.sprite.pt = pt_to_sprite(new_pt);
    }
}

static void game_center_map(game_t *p_game)
{
  player_t *p_player = &p_game->player;
  level_t *p_level = p_game->p_cur_level;
  int16_t dist_x = min( (p_player->sprite.pt.x - p_game->bg_x) - screen_w / 2,
			(p_game->p_cur_level->w-1)*TILE_W - p_game->bg_x);
  int16_t dist_y = min( (p_player->sprite.pt.y - p_game->bg_y) - screen_h / 2,
			(p_game->p_cur_level->h-1)*TILE_H - p_game->bg_y);

  /* Accelerate after the player */
  if (abs(dist_x) > 16)
    p_game->dx += abs(p_game->dx + sign(dist_x)*1) >= 9 ? 0 : sign(dist_x)*1;
  else if (dist_x != 0)
    p_game->dx -= sign(p_game->dx);
  if (abs(dist_y) > 16)
    p_game->dy += abs(p_game->dy + sign(dist_y)*1) >= 9 ? 0 : sign(dist_y)*1;
  else if (dist_y != 0)
    p_game->dy -= sign(p_game->dy);

  /* Center the view */
  p_game->bg_x += p_game->dx;
  p_game->bg_y += p_game->dy;

  if (p_game->bg_x < 0)
    {
      p_game->dx = 0;
      p_game->bg_x = 0; /* Stop at (0,?) */
    }
  else if (p_game->bg_x > p_level->w*TILE_W - PLAYFIELD_WIDTH)
    {
      p_game->dx = 0;
      p_game->bg_x = p_level->w*TILE_W - PLAYFIELD_WIDTH; /* Stop at (screen_w, ?)*/
    }

  if (p_game->bg_y < 0)
    {
      p_game->dy = 0;
      p_game->bg_y = 0;
    }
  else if (p_game->bg_y > p_level->h*TILE_H - PLAYFIELD_HEIGHT)
    {
      p_game->dy = 0;
      p_game->bg_y = p_level->h*TILE_H - PLAYFIELD_HEIGHT;
    }


  switch(sign(p_game->bg_y - (p_player->sprite.pt.y - screen_h / 2)))
    {
    case -1:
      p_game->bg_y++; break;
    case 1:
      p_game->bg_y--; break;
    default:
      break;
    }

  vMapSetXY(p_game->bg_x, p_game->bg_y);
}

static bool_t show_tile(game_t *p_game, point_t tile_pt)
{
  mask_tile_t mask_tile;
  tile_t tile;

  mask_tile = MTILE_AT(p_game, tile_pt.x, tile_pt.y);
  tile = p_game->p_cur_level->p_level_data[tile_pt.y * p_game->p_cur_level->w + tile_pt.x];

  vMapSetTile(tile_pt.x,tile_pt.y,tile);
  if (MASK_TILE_IS_ELEM(mask_tile))
    p_game->elems[mask_tile.id].display_counter = 10;

  return !is_visible(mask_tile);
}

/* (to the right)         (up)
 *     .    3             4 5 6
 *   . 7 8  2             2 3 . .
 * . 4 5 6  1             1 . . .
 * X 1 2 3  0             X . . .
 *   1 2 3
 *
 * 1,2,3                  1,2,4
 * 4,5,6                  1,3,5
 * 4,5,7,8                1,3,6
 */
#define PT(x_in,y_in) {x_in, y_in}
static point_t horiz_offs[3][3] =
  {
    { PT(1,0), PT(2,0), PT(3,0) },
    { PT(1,1), PT(2,1), PT(3,1) },
    { PT(1,1), PT(2,1), PT(2,2) }  /* PT(3,2) */
  };
static point_t vert_offs[3][3] =
  {
    { PT(0,1), PT(0,2), PT(0,3) },
    { PT(0,1), PT(1,2), PT(1,3) },
    { PT(0,1), PT(1,2), PT(1,2) },
  };
#undef PT

static void game_update_view_buffer(game_t *p_game)
{
  point_t orig_pt = pt_to_tile(p_game->player.sprite.pt);
  int i,j;

  for (i=0; i<3; i++)
    {
      bool_t skip[4] = { FALSE, FALSE, FALSE, FALSE };
      bool_t skip_vert[4] = { FALSE, FALSE, FALSE, FALSE };

      for (j=0; j<3; j++)
	{
	  /* up right, up left, down right, down left */
	  if (!skip[0]) skip[0] = show_tile(p_game, pt_add(orig_pt, horiz_offs[i][j]));
	  if (!skip[1]) skip[1] = show_tile(p_game, pt_add(orig_pt, pt_flip(horiz_offs[i][j])));
	  if (!skip[2]) skip[2] = show_tile(p_game, pt_add(orig_pt, pt_flop(horiz_offs[i][j])));
	  if (!skip[3]) skip[3] = show_tile(p_game, pt_add(orig_pt, pt_flip_flop(horiz_offs[i][j])));

	  if (!skip_vert[0]) skip_vert[0] = show_tile(p_game, pt_add(orig_pt, vert_offs[i][j]));
	  if (!skip_vert[1]) skip_vert[1] = show_tile(p_game, pt_add(orig_pt, pt_flip(vert_offs[i][j])));
	  if (!skip_vert[2]) skip_vert[2] = show_tile(p_game, pt_add(orig_pt, pt_flop(vert_offs[i][j])));
	  if (!skip_vert[3]) skip_vert[3] = show_tile(p_game, pt_add(orig_pt, pt_flip_flop(vert_offs[i][j])));
	}
    }
}

static void game_erase_view_buffer(game_t *p_game)
{
  int16_t x_start = max(p_game->player.sprite.pt.x/TILE_W - LAMP_SHINE_RADIUS, 0);
  int16_t x_end = min(p_game->player.sprite.pt.x/TILE_W + LAMP_SHINE_RADIUS, p_game->p_cur_level->w);
  int16_t y_start = max(p_game->player.sprite.pt.y/TILE_H - LAMP_SHINE_RADIUS, 0);
  int16_t y_end = min(p_game->player.sprite.pt.y/TILE_H + LAMP_SHINE_RADIUS, p_game->p_cur_level->h);
  int16_t x, y;

  /* Update the view buffer */
  for (y=y_start; y<=y_end; y++)
    {
      for (x=x_start; x<=x_end; x++)
	{
	  mask_tile_t mask_tile = MTILE_AT(p_game, x,y);
	  tile_t tile = vMapGetTile(x,y);

	  if (tile != TILE_UNDISCOVERED &&
	      tile != TILE_EMPTY &&
	      tile < TILE_MAX)
	    {
	      vMapSetTile(x,y, tile + TILE_MAX);
	      if (MASK_TILE_IS_ELEM(mask_tile) &&
		  p_game->elems[mask_tile.id].display_counter > 0)
		p_game->elems[mask_tile.id].display_counter = 10;
	    }
	}
    }
}


/* Init the game structure */
static void game_init(game_t *p_game)
{
  memset(p_game, 0, sizeof(game_t));

  p_game->pp_sprite_frames = LPH_splitSprites(&SPRITE_FRAMES, N_FRAMES);

  p_game->p_title = &LOGO;
  p_game->p_tiles = BG_TILES;

  p_game->conf.sound = FALSE;

  player_set_name(p_game, &p_game->player, "SIMON");
}

static void game_fini(game_t *p_game)
{
  level_free(p_game, p_game->p_cur_level);

  vDisposePtr(p_game->p_title);
  vDisposePtr(p_game->p_tiles);
  vDisposePtr(p_game->pp_sprite_frames);

  /* FIXME: Display finish screen */
  vMapDispose();
}

/* The main game loop */
static void game_do(game_t *p_game)
{
  int16_t teleporter_count = 0;
  p_game->game_on = TRUE;

  while(p_game->game_on)
    {
      mask_tile_t cur_tile;
      uint32_t before, after;
      uint32_t keys;

      before = vGetTickCount(); /* Get the current ms ticks */

      /* Read the keys */
      keys = vGetButtonData();

      if ((p_game->frame_count & 4) &&
	  p_game->p_cur_level->time - ((before - p_game->start_ticks) / 1000) <= 0)
	{
	  status_enqueue_message(p_game, "TIME UP!");
	  p_game->player.active_delay = 15;
	}
      if (p_game->player.active_delay > 0)
	{
	  if (--p_game->player.active_delay <= 0)
	    game_goto_level(p_game, &levels[p_game->cur_level]);
	}
      else
	{
	  uint32_t fire = keys & (KEY_FIRE | KEY_FIRE2);
	  dir_t dir = IDLE;

	  /* Move the player around */
	  if (keys & KEY_LEFT)
	    dir = LEFT;
	  else if (keys & KEY_RIGHT)
	    dir = RIGHT;
	  else if (keys & KEY_UP)
	    dir = UP;
	  else if (keys & KEY_DOWN)
	    dir = DOWN;

	  if (dir != IDLE)
	    player_handle_input(p_game, dir, fire);
	}
      if (keys & KEY_SELECT)
	return;

      /* Draw the game */
      game_center_map(p_game);
      game_update_view_buffer(p_game);

      vUpdateMap();
      game_check_all_elems(p_game); /* Handle elements (and draw them) */

      /* Check/call all callbacks */
      callback_check_all(p_game);

      cur_tile = MTILE_AT(p_game, p_game->player.sprite.pt.x / TILE_W, p_game->player.sprite.pt.y / TILE_H);

      /* Check for exit */
      if (MASK_TILE_IS_EXIT(cur_tile))
	{
	  DbgPrintf("Level finished!\n");
	  if (++p_game->cur_level >= N_LEVELS)
	    {
	      DbgPrintf("No more levels!\n");
	      return; /* Not ongoing */
	    }

	  game_goto_level(p_game, &levels[p_game->cur_level]);
	}
      else if (cur_tile.id == MASK_ID_TELEPORTER)
	{
	  /* Check for teleporters */
	  if (teleporter_count++ > 15)
	    {
	      teleport(p_game, &p_game->player);
	      teleporter_count = 0;
	    }
	}
      else
	teleporter_count = 0;
      if (MASK_TILE_IS_ELEM(cur_tile))
	{
	  elem_t *p_elem = &p_game->elems[cur_tile.id];

	  if (p_elem->type == GHOST &&
	      pt_eq(p_game->player.sprite.pt, p_elem->sprite.pt))
	    explode(p_game, p_elem->sprite.pt.x / TILE_W, p_elem->sprite.pt.y / TILE_W, 1);
	}

      player_draw(p_game, &p_game->player);
      /* Draw the status field */
      if ((p_game->frame_count & 2) == 0)
	{
	  vSetClipWindow(0, PLAYFIELD_HEIGHT, screen_w, screen_h);
	  status_draw(p_game);
	  vSetClipWindow(0,0, screen_w, PLAYFIELD_HEIGHT-1);
	}

      vFlipScreen(1);

      after = vGetTickCount(); /* Get the current ms ticks */
      /* Every loop iteration should take about SLEEP_PERIOD, see to that */
      if ((after - before) < SLEEP_PERIOD)
	{
	  msSleep( SLEEP_PERIOD - (after-before) );
	}
#if 0
      else
	DbgPrintf("Missed frame: %d ms\n", (after-before)-SLEEP_PERIOD);
#endif
      p_game->frame_count++;
    }

  return;
}

static char *main_menu_msgs[] =
  {
    "START NEW GAME",  /* 0 */
    "RESUME GAME",     /* 1 */
    "LOAD OLD GAME",   /* 2 */
    "OPTIONS",         /* 3 */
    "HELP",            /* 4 */
    "SELECT LEVEL",
    "^|1|2|3",
    " ",               /* 7 */
    "QUIT",            /* 8 */
    (char*)NULL,
  };

static char *options_menu_msgs[] =
  {
    "SOUND EFFECTS",   /* 0 */
    "^|ON|OFF",
    " ",               /* 2 */
    "EXIT MENU",       /* 3 */
    (char*)NULL,
  };

int main(int argc, char *argv[])
{
  bool_t done = FALSE;
  menu_t main_menu, options_menu;

  LPH_init();
  LPH_setTileImageSize(16,16);

  DbgPrintf("Boulder Dash build %s, %s\n", __DATE__, __TIME__);

  /* srand() */
  vSetRandom(vGetTickCount());

  /* Get the size of the screen */
  get_screen_size(&screen_w, &screen_h);

  /* Initialise the graphics */
  vSetTransferMode(MODE_TRANS);

  /* Init the game and the menu */
  game_init(&game);
  menu_init(&main_menu, &SMALL_FONT, main_menu_msgs,
	    0, screen_h/2, screen_w, screen_h);
  menu_init(&options_menu, &SMALL_FONT, options_menu_msgs,
	    0, screen_h/2, screen_w, screen_h);


  while (!done)
    {
      uint32_t mask = ~(1<<2);
      int submenus[1];

      mask &= (game.game_on ? ~0 : ~(1<<1));

      vSetClipWindow(0,0, screen_w, screen_h);
      vClearScreen(vRGB(0,0,0));
      vDrawObject(screen_w/2 - game.p_title->width / 2, 16, game.p_title);

      game.frame_count = 0;

      switch(menu_select(&main_menu, mask, submenus))
	{
	case 0:
	  {
	    game.cur_level = submenus[0];
	    game_goto_level(&game, &levels[game.cur_level]);
	  }
	case 1:
	  /* The main game loop */
	  game_do(&game);
	  break;
	case 3:
	  {
	    int submenus[1];
	    switch(menu_select(&options_menu, ~0, submenus))
	      {
	      case 3:
		if (submenus[0] == 0) /* On */
		  game.conf.sound = TRUE;
		else if (submenus[0] == 1) /* Off */
		  game.conf.sound = FALSE;

		DbgPrintf("Sound: %d\n", submenus[0]);
		break;
	      default:
		error_msg("Unknown menu option!\n");
		break;
	      }
	  }
	  break;
	case 4:
	  vMsgBox(VMB_OK,
		  "Use the joystick or number pad to move the player, "
		  "keep the left fire button pressed to use keys or to push "
		  "an adjacent tile. The right fire button together with a "
		  "direction is used to place bombs."
		  );
	  break;
	case -1:
	case 8:
	  done = TRUE;
	  break;
	default:
	  error_msg("Unknown menu option!\n");
	}
    }

  menu_fini(&main_menu);
  menu_fini(&options_menu);
  game_fini(&game);

  return 0;
}
