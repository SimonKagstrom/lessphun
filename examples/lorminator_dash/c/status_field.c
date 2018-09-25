/*********************************************************************
 *
 * Copyright (C) 2004,  Blekinge Institute of Technology
 *
 * Filename:      status_field.c
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:   Implementation of the status field
 *
 * $Id: status_field.c 11321 2006-09-27 12:13:52Z ska $
 *
 ********************************************************************/
#include <boulder.h>

#define MESSAGE_TIME 13
#define N_MSGS 3

typedef struct
{
  const char *p_str;
  uint8_t     countdown;
} message_t;

typedef struct
{
  message_t  msgs[N_MSGS];
  int16_t    front;
  int16_t    end;
} message_queue_t;

static message_queue_t queue;

static void dequeue_message(game_t *p_game, message_queue_t *p_queue)
{
  p_queue->msgs[p_queue->front].p_str = NULL;
  p_queue->front++;

  if (p_queue->front >= N_MSGS)
    p_queue->front = 0;
}

void status_enqueue_message(game_t *p_game, const char *p_str)
{
  message_queue_t *p_queue = &queue;

  /* Is this already the front message? Skip it */
  if (p_queue->msgs[p_queue->front].p_str == p_str)
    return;
  p_queue->msgs[p_queue->end].p_str = p_str;
  p_queue->msgs[p_queue->end].countdown = MESSAGE_TIME;
  p_queue->end++;

  if (p_queue->end >= N_MSGS)
    p_queue->end = 0;
}

void status_clear_messages(game_t *p_game)
{
  memset(&queue, 0, sizeof(queue));
}


static void game_draw_message(game_t *p_game, const char *msg)
{
  int16_t x_start = 40;
  int16_t y_start = PLAYFIELD_HEIGHT+2;

  print_font(&SMALL_FONT, vRGB(0,255,128), x_start, y_start, msg);
}

static void print_status_entry(game_t *p_game, int n, int16_t x, int16_t y, int32_t val)
{
  char buf[8];

  vitoa(val, buf, n, '0');
  print_font(&SMALL_FONT, vRGB(128,255,255), x, y, buf);
}

static void print_status_entry_with_symbol(game_t *p_game, int n, int16_t x, int16_t y, SPRITE *p_symbol, int32_t val)
{
  int32_t w = p_symbol ? p_symbol->width : 0;

  if (p_symbol)
    vDrawObject(x, y, p_symbol);
  print_status_entry(p_game, n, x + w + 2, y + 1, val);
}

void status_draw(game_t *p_game)
{
  int32_t diamonds = p_game->p_cur_level->diamonds_required - p_game->player.diamonds;
  int32_t time_left = p_game->p_cur_level->time - ((vGetTickCount() - p_game->start_ticks) / 1000);
  int16_t x_start;
  int16_t y_start;
  int i;

  if (diamonds < 0)
    diamonds = 0;
  if (time_left < 0)
    time_left = 0;

  y_start = PLAYFIELD_HEIGHT;
  x_start = 0;

  vSetForeColor(vRGB(0,128,0));
  vFillRect(x_start, y_start+1, screen_w, screen_h);

  vSetForeColor(vRGB(0,255,0));
  vFillRect(x_start, y_start, screen_w, y_start);

  /* Print the number of diamonds and bombs */
  print_status_entry_with_symbol(p_game, 2, x_start + 2, y_start + 1,
		     p_game->pp_sprite_frames[FRAMES_DIAMOND], diamonds);
  print_status_entry_with_symbol(p_game, 2, x_start + 2, y_start + TILE_H + 1,
		     p_game->pp_sprite_frames[FRAMES_BOMB], p_game->player.bombs);

  for (i=0; i<METAL_MAX; i++)
    {
      if (p_game->player.keys[i])
	vDrawObject(x_start + 64 + i*(TILE_H+1), y_start + 1, p_game->pp_sprite_frames[FRAMES_IRON_KEY + i]);
    }
  print_status_entry(p_game, 3, PLAYFIELD_WIDTH-16, y_start + 2, time_left);

  /* Display the queued messages */
  if (queue.msgs[queue.front].p_str)
    {
      if (--queue.msgs[queue.front].countdown > 0)
	game_draw_message(p_game, queue.msgs[queue.front].p_str);
      else
	dequeue_message(p_game, &queue);
    }
}
