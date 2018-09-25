/*********************************************************************
 *
 * Copyright (C) 2006,  Blekinge Institute of Technology
 *
 * Filename:      lessphun.c
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:   Lessphun implementation
 *
 * $Id: lessphun.c 15428 2007-05-19 11:31:46Z ska $
 *
 ********************************************************************/
#include <lessphun.h>
#include <java/lang.h>

lessphun_info_t __LPH_info;
#define info __LPH_info

/* --- Tilemaps --- */
void vMapDispose(void)
{
  if (info.tiledLayer)
    NOPH_delete(info.tiledLayer);
  if (info.tilemapImage)
    NOPH_delete(info.tilemapImage);
}

int vMapInit(MAP_HEADER *p)
{
  int x,y;

  memcpy(&info.cur_tilemap, p, sizeof(MAP_HEADER));

  /* Create the tilemap image and the tiled layer */
  info.tilemapImage = NOPH_ResourceManager_getImage(info.resourceManager,
						    (char*)info.cur_tilemap.tiledata); /* Actually a string here */
  info.tiledLayer = NOPH_TiledLayer_new(info.cur_tilemap.width + 1, info.cur_tilemap.height + 1,
					info.tilemapImage, info.tile_w, info.tile_h);

  /* Fill in the tilemap */
  for (y = 0; y < info.cur_tilemap.height; y++)
    {
      for (x = 0; x < info.cur_tilemap.width; x++)
	vMapSetTile(x, y, info.cur_tilemap.mapoffset[ y * info.cur_tilemap.width + x]);
    }

  return 1;
}

void vUpdateSpriteMap(void)
{
  int i;

  vUpdateMap();

  for (i = 0; i < info.n_sprite_slots; i++)
    {
      sprite_entry_t *p = info.sprite_slots[i];

      if (p)
	NOPH_Graphics_drawImage(info.graphics, p->sprite->handle,
				p->x, p->y, NOPH_Graphics_TOP | NOPH_Graphics_LEFT);
    }
}



/* Sprites */
void vSpriteClear(void)
{
  memset(info.sprite_slots, 0, info.n_sprite_slots * sizeof(sprite_entry_t*));
}

int vSpriteInit(int slots)
{
  info.n_sprite_slots = slots;

  info.sprite_slots = malloc( info.n_sprite_slots * sizeof(sprite_entry_t*) );

  return 1;
}

void vSpriteSet(int slot, SPRITE *sprite, int x, int y)
{
  sprite_entry_t *cur = info.sprite_slots[slot];

  /* Allocate a new entry if this is referenced the first time */
  if ( !cur )
    cur = info.sprite_slots[slot] = malloc(sizeof(sprite_entry_t));

  cur->sprite = sprite;
  cur->x = x;
  cur->y = y;
}

/* --- Graphics --- */
void vSetTransferMode(int mode)
{
  /* Ignored */
}

void vSetClipWindow(int x, int y, int w, int h)
{
  NOPH_Graphics_setClip(info.graphics, x,y, w,h);
}

void vSetDisplayWindow(int w, int h)
{
}

void vClearScreen(int color)
{
  vSetForeColor(color);
  vFillRect(0,0,NOPH_GameCanvas_getWidth(info.gameCanvas), NOPH_GameCanvas_getHeight(info.gameCanvas));
}

void vFillRect(int x1, int y1, int x2, int y2)
{
  if (x1 > x2)
    {
      int tmp = x1;
      x1 = x2;
      x2 = tmp;
    }
  if (y1 > y2)
    {
      int tmp = y1;
      y1 = y2;
      y2 = tmp;
    }

  NOPH_Graphics_fillRect(info.graphics, x1,y1, (x2-x1) + 1, (y2-y1) + 1);
}

void vSetForeColor(int color)
{
  NOPH_Graphics_setColor_int(info.graphics, color);
}

/* -- Sound -- */
void vBeep(int freq, int ms)
{
  NOPH_Manager_playTone(freq, ms, 50);
}


/* -- Lessphun-specific stuff -- */
void LPH_setTileImageSize(int w, int h)
{
  info.tile_w = w;
  info.tile_h = h;
}


SPRITE **LPH_splitSprites(SPRITE *src, int n_sprites)
{
  SPRITE **pp; /* Return val */
  SPRITE *sprites;
  int w,h;
  int i;

  w = NOPH_Image_getWidth(src->handle);
  h = NOPH_Image_getHeight(src->handle);

  /* Create new images for this sprite */
  sprites = malloc(sizeof(SPRITE) * n_sprites);
  pp = malloc(sizeof(SPRITE*) * n_sprites);
  for (i = 0; i < n_sprites; i++)
    {
      NOPH_Image_t new;
      SPRITE *p = &sprites[i];

      new = NOPH_Image_createImage(src->handle,
				   0, i * h / n_sprites,
				   w, h / n_sprites,
				   0);

      p->width = w;
      p->height = h / n_sprites;
      p->handle = new;
      p->frame = i;

      pp[i] = p;
    }

  return pp;
}


/* --- Font handling --- */

void vSetActiveFont(VMGPFONT *font)
{
  info.current_font = font;
}

void vPrint(int mode, int x, int y, const char *msg)
{
  if (info.current_font == NULL)
    return;

  while (*msg != '\0')
    {
      char c = *msg;
      int c_x = (c % info.current_font->chars_per_line) + 1;
      int c_y = (c / info.current_font->chars_per_line) - 1;
      int xoff = c_x * (info.current_font->width + 2) + 2;
      int yoff = c_y * (info.current_font->height + 2) + 2;

      NOPH_Graphics_drawRegion(info.graphics, info.current_font->image,
			       xoff, yoff,
			       info.current_font->width, info.current_font->height,
			       0, x, y, NOPH_Graphics_TOP | NOPH_Graphics_LEFT);

      x += info.current_font->width + 1;
      msg++;
    }
}

/* --- Timing --- */
extern uint32_t vGetTickCount(void)
{
  uint32_t out = (uint32_t)(NOPH_System_currentTimeMillis() - info.start_time);

  return out;
}

/* --- Misc --- */
int vGetCaps(int mode, void *data)
{
  switch(mode)
    {
    case CAPS_VIDEO:
      {
	VIDEOCAPS *p = (VIDEOCAPS*)data;

	p->width = NOPH_GameCanvas_getWidth(info.gameCanvas);
	p->height = NOPH_GameCanvas_getHeight(info.gameCanvas);
      }
      break;
    default:
      return 0;
      break;
    }
  return 1;
}

void vTerminateVMGP(void)
{
  exit(0);
}

void vitoa(int val, char *buf, int n_chars, char pad)
{
  /* FIXME: This is not the behavior of the Mophun variant */
  snprintf(buf, n_chars + 1, "%0d", val);
}


/* --- File-handling things ---  */
int vStreamOpen(const char *filename, int mode)
{
  char m[4];
  char *p=m;

  memset(m, 0, sizeof(m));
  if (mode & STREAM_READ)
    *p++ = 'r';
  if (mode & STREAM_WRITE)
    *p++ = 'w';

  return (int)fopen(filename, m);
}

int vStreamRead(int fd, void *out, size_t size)
{
  return fread(out, size, 1, (FILE*)fd);
}

int vStreamWrite(int fd, void *in, size_t size)
{
  return fwrite(in, size, 1, (FILE*)fd);
}

int vStreamSeek(int fd, int offset, int whence)
{
  return fseek((FILE*)fd, offset, whence);
}

void vStreamClose(int fd)
{
  fclose((FILE*)fd);
}

/* --- Resource handling --- */
void LPH_initSprite(SPRITE *dst, char *filename)
{
  dst->handle = NOPH_ResourceManager_getImage( info.resourceManager, filename );
  dst->width = NOPH_Image_getWidth(dst->handle);
  dst->height = NOPH_Image_getHeight(dst->handle);
  dst->frame = 0;
}

void LPH_initFont(VMGPFONT *dst, char *filename, int width, int height)
{
  dst->image = NOPH_ResourceManager_getImage( info.resourceManager, filename);
  dst->width = width;
  dst->height = height;
  dst->chars_per_line = NOPH_Image_getWidth(dst->image) / (width + 2);
}


/* --- Initialization --- */
extern void LPH_res_init(void);
void LPH_init(void)
{
  info.resourceManager = NOPH_ResourceManager_getInstance();

  info.gameCanvas = NOPH_GameCanvas_get();
  info.graphics = NOPH_GameCanvas_getGraphics(info.gameCanvas);

  NOPH_Canvas_setFullScreenMode(info.gameCanvas, 1);

  info.tile_w = 8;
  info.tile_h = 8;

  info.start_time = NOPH_System_currentTimeMillis();

  LPH_res_init();
}
