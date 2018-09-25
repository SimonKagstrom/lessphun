#ifndef __SPECIALS_H__
#define __SPECIALS_H__

#include <boulder.h>
#include <levels.h>

typedef struct
{
  int     n_explosions;
  point_t *pt;
} explosion_priv_t;

typedef struct
{
  point_t pt;
  dir_t   dir;
  int     id;
  tile_t  tile;
  bool_t  toggle;
} growing_wall_priv_t;

typedef struct
{
  elem_type_t elem_type;
  void       *elem_extra_arg; /* Must not be a real pointer! */
  point_t     pt;
  dir_t       dir;
  int         id;
  bool_t      toggle;
  int16_t     n_elems;
} elem_generator_priv_t;

#define POINT(in_x,in_y) {.x = (in_x), .y = (in_y)}

/* Some convenience macros */
#define SPECIAL_EXPLOSION(n_pts, in_pt...)                               \
    { /* Explosion special */                                            \
      .fn = special_explosion,                                           \
      .p_priv = &((explosion_priv_t){ .n_explosions = n_pts, .pt = in_pt }), \
      .sizeof_special = sizeof(special_t) + sizeof(explosion_priv_t) + (n_pts)*sizeof(point_t),  \
    }
#define SPECIAL_GROWING_WALL(in_pt,in_tile,in_dir)                          \
    { /* Growing wall special */					        \
      .fn = special_growing_wall,					        \
      .p_priv = &((growing_wall_priv_t){ .pt = in_pt, .dir = (in_dir), .tile = in_tile }),  \
      .sizeof_special = sizeof(special_t) + sizeof(growing_wall_priv_t),        \
    }
#define SPECIAL_ELEM_GENERATOR(in_pt,in_dir, in_type, in_n_elems, in_elem_extra_arg) \
    { /* Growing wall special */					        \
      .fn = special_elem_generator,					        \
      .p_priv = &((elem_generator_priv_t){ .pt = in_pt, .dir = (in_dir),        \
                                           .elem_type = (in_type), .n_elems = (in_n_elems), \
                                           .elem_extra_arg = (in_elem_extra_arg) }), \
      .sizeof_special = sizeof(special_t) + sizeof(elem_generator_priv_t),      \
    }

void special_explosion(game_t *p_game, uint8_t special);
void special_growing_wall(game_t *p_game, uint8_t special);
void special_elem_generator(game_t *p_game, uint8_t special);
bool_t special_init_generic(game_t *p_game, uint8_t special,
			    tile_t *p_tile, mask_tile_t *p_mask_tile, int16_t x, int16_t y);

#endif /* !__SPECIALS_H__ */
