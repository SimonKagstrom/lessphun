/*********************************************************************
 *
 * Copyright (C) 2004,  Blekinge Institute of Technology
 *
 * Filename:      boulder.h
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:   Boulder defs
 *
 * $Id: boulder.h 12189 2006-11-16 16:02:24Z ska $
 *
 ********************************************************************/
#ifndef __BOULDER_H__
#define __BOULDER_H__

#include <lessphun.h>
#include <assert.h>

#include "types.h"
#include "res.h"
#include "point.h"
#include "dir.h"
#include "utils.h"
#include "menu.h"
#include "tiles.h"
#include "file_handling.h"

#define SLEEP_PERIOD     (13*10)
#define GAME_MS(x)       ((x) * SLEEP_PERIOD)

#define BOMB_ITERATIONS  (400/SLEEP_PERIOD)

#define LAMP_SHINE_RADIUS 3

#define PLAYFIELD_HEIGHT   (screen_h - (TILE_H * 3 - (TILE_H/2)))
#define PLAYFIELD_WIDTH    (screen_w)

#define FRAMES_PLAYER   0
#define FRAMES_BOULDER  (FRAMES_PLAYER + 4)
#define FRAMES_DIAMOND  (FRAMES_BOULDER + 1)
#define FRAMES_BOMB     (FRAMES_DIAMOND + 3)
#define FRAMES_GHOST    (FRAMES_BOMB + 1)
#define FRAMES_FLAME    (FRAMES_GHOST + 1)
#define FRAMES_IRON_KEY (FRAMES_FLAME + 2)
#define FRAMES_GOLD_KEY (FRAMES_IRON_KEY + 1)
#define FRAMES_RED_KEY  (FRAMES_GOLD_KEY + 1)
#define FRAMES_BLOCK    (FRAMES_RED_KEY + 1)
#define FRAMES_LEVER    (FRAMES_BLOCK + 1)

#define FIRST_HALFTONE  (FRAMES_LEVER + 2)

#define N_FRAMES        (FIRST_HALFTONE * 2)

#define N_ELEMS     80
#define N_CALLBACKS  6
#define NAME_LEN    16

#define TRANSPORT_TO_DIR(x) (dir_t)((x) - MASK_ID_LEFT_TRANSPORT + 2)

#define MTILE_AT(p_game, x, y) ((p_game)->p_level_mask[(y) * (p_game)->p_cur_level->w + (x)])
#define TILE_SWAP(p_game, x1,y1, x2, y2) do { \
				  level_t *p_level = (p_game)->p_cur_level; \
				  mask_tile_t *p_level_mask = (p_game)->p_level_mask; \
				  mask_tile_t tile_a = p_level_mask[(y1) * p_level->w + (x1)]; \
				  mask_tile_t tile_b = p_level_mask[(y2) * p_level->w + (x2)]; \
				  p_level_mask[(y1) * p_level->w + (x1)].id = tile_b.id; \
				  p_level_mask[(y2) * p_level->w + (x2)].id = tile_a.id; \
				 } while(0)

#define ELEM_AT(p_game, mask_tile) (p_game)->elems[(mask_tile).id]

/* Forward decls */
struct s_special;
struct s_game;


typedef enum
{
  IRON   = 0,
  BRONZE = 1,
  SILVER = 2,
  GOLD   = 3,
  METAL_MAX = GOLD,
} metal_t;

typedef enum
{
  OFF = 0,
  ON = 1,
} lever_state_t;

typedef enum
{
  BOULDER = 0,
  DIAMOND = 1,
  BOMB = 2,
  GHOST = 3,
  FLAME = 4,
  BLOCK = 5,
  LEVER = 6
} elem_type_t;

typedef enum
{
  NONE = 0,
  EXPLOSION = 1,
  SCRATCHING = 2,
  DIAMOND_TAKEN = 3,
} sound_type_t;

typedef enum
{
  FN = 0,
} special_type_t;

typedef struct
{
  point_t pt;
} teleporter_t;

typedef struct
{
  point_t   pt;
  point_t   view_pt;
  dir_t     dir;
  int16_t   dx;
  int16_t   dy;
  uint8_t   frame;
  uint8_t   base_frame;
  uint8_t   view_frame;
} sprite_t;

typedef struct
{
} block_t;

typedef struct
{
} boulder_t;

typedef struct
{
} diamond_t;

typedef struct
{
  dir_t   cur_dir;
} ghost_t;

typedef struct
{
  sprite_t sprite;
  uint8_t  diamonds;
  uint8_t  bombs;
  int8_t   active_delay;

  bool_t   keys[METAL_MAX];
  char     name[NAME_LEN];
} player_t;

typedef struct
{
  uint8_t  countdown;
} bomb_t;

typedef struct
{
  uint8_t  countdown;
} flame_t;

typedef struct
{
  lever_state_t state;
  uint8_t  special; /* Index into the specials */
} lever_t;

typedef struct
{
  elem_type_t type;
  sprite_t    sprite;
  bool_t      is_active;
  bool_t      falling;
  uint8_t     id;
  uint8_t     display_counter;
  union
  {
    boulder_t boulder;
    diamond_t diamond;
    block_t   block;
    ghost_t   ghost;
    bomb_t    bomb;
    flame_t   flame;
    lever_t   lever;
  } u;
} elem_t;

typedef struct
{
  void (*fn)(struct s_game *p_game, int id, void *p_priv);
  void *p_priv;
} callback_t;

typedef struct s_special
{
  void (*fn)(struct s_game *p_game, uint8_t special);
  void *p_priv; /* Private data */
  bool_t  triggered;
  int16_t sizeof_special; /* Including private data */
} special_t;

typedef struct
{
  int8_t   w;
  int8_t   h;
  uint8_t  diamonds_required;
  uint8_t  n_specials;
  point_t  player_pt;
  uint32_t time; /* Time to finish the level */
  uint8_t  n_teleporters;

  /* Returns the tile to set */
  bool_t   (*init_special_fn)(struct s_game *p_game, uint8_t special,
			      tile_t *p_tile, mask_tile_t *p_mask_tile, int16_t x, int16_t y);
  special_t  *p_specials;
  teleporter_t *p_teleporters;
  tile_t     *p_level_data;
  union
  {
    int32_t  res_id;
    char    *filename;
  } u;
  int32_t  res_packed_size;
  int32_t  res_unpacked_size;
  int32_t  level_handle;
} level_t;

typedef struct
{
  file_handle_t handle;
  file_header_t *p_hdr;
  file_section_table_t *p_sectab;
  file_section_t *p_set;
  int16_t    level_set_idx;
} level_set_t;

typedef struct
{
  bool_t   sound;
  uint16_t cur_level_sel; /* In the menu */
  uint16_t cur_level; /* The level the player is currently at */
} conf_t;

typedef struct s_game
{
  MAP_HEADER   bgmap;
  int16_t      bg_x;
  int16_t      bg_y;
  int16_t      dx;
  int16_t      dy;
  SPRITE     **pp_sprite_frames;
  uint8_t     *p_tiles;
  SPRITE      *p_title;

  level_t     *p_cur_level;
  int16_t      cur_level;
  int16_t      exit_x;
  int16_t      exit_y;

  mask_tile_t *p_level_mask;
  tile_t      *p_view_data;
  elem_t       elems[N_ELEMS];
  callback_t   callbacks[N_CALLBACKS];
  int16_t      n_elems;
  int16_t      first_free;

  bool_t       game_on;
  uint32_t     start_ticks;
  uint32_t     frame_count;

  int32_t      level_handle;
  level_set_t  cur_level_set;

  player_t     player;
  conf_t       conf;
} game_t;

extern int32_t screen_w, screen_h;

void boulder_init(game_t *p_game, elem_t *p_elem, point_t pt);
void bomb_init(game_t *p_game, elem_t *p_elem, point_t pt);
void flame_init(game_t *p_game, elem_t *p_elem, point_t pt);
void diamond_init(game_t *p_game, elem_t *p_elem, point_t pt);
void ghost_init(game_t *p_game, elem_t *p_elem, point_t pt);
elem_t *elem_add(game_t *p_game, elem_type_t type, point_t pt, void *p_arg);
void elem_remove(game_t *p_game, elem_t *p_elem);
void elem_fall(game_t *p_game, elem_t *p_elem);
int elem_push(game_t *p_game, elem_t *p_elem, dir_t dir);
void elem_draw(game_t *p_game, elem_t *p_elem);
void ghost_move(game_t *p_game, elem_t *p_elem);
void bomb_place(game_t *p_game, point_t in_pt);

void player_init(game_t *p_game, player_t *p_player, point_t p);
void player_draw(game_t *p_game, player_t *p_player);
void player_kill(game_t *p_game, player_t *p_player);
void player_set_name(game_t *p_game, player_t *p_player, const char *p_name);

void sprite_init(sprite_t *p_sprite, point_t p, uint8_t base_frame);
void sprite_move(sprite_t *p_sprite, dir_t dir);
void explode(game_t *p_game, int16_t x_in, int16_t y_in, int16_t radius);

void game_goto_level(game_t *p_game, level_t *p_level);

level_t *level_set_load_level(game_t *p_game, level_set_t *p_set, int level_nr);
bool_t level_set_init(game_t *p_game, level_set_t *p_out, int level_set_nr, file_handle_t handle);
void level_set_fini(game_t *p_game, level_set_t *p_set);
file_handle_t level_set_open_file(game_t *p_game, const char *filename);
file_handle_t level_set_open_resource(game_t *p_game, int32_t id);
void level_set_close(game_t *p_game, file_handle_t handle);

void level_free(game_t *p_game, level_t *p_level);
void status_draw(game_t *p_game);
void status_enqueue_message(game_t *p_game, const char *p_str);
void status_clear_messages(game_t *p_game);
void sound_play(game_t *p_game, sound_type_t which);


int callback_register(game_t *p_game,
		      void (*fn)(game_t *p_game, int id, void *p_priv),
		      void *p_priv);
void callback_check_all(game_t *p_game);
void callback_unregister(game_t *p_game, int id);

#define conveyor_belt_active(p_game) (((p_game)->frame_count & 3) == 3)


static inline bool_t is_close_to_player(const game_t *p_game, const point_t pt, const int16_t dist)
{
  return (abs(pt.x - p_game->player.sprite.pt.x)/TILE_W <= dist &&
	  abs(pt.y - p_game->player.sprite.pt.y)/TILE_H <= dist);
}

#endif /* !__BOULDER_H__ */
