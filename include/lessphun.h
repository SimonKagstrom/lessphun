/*********************************************************************
 *
 * Copyright (C) 2006,  Blekinge Institute of Technology
 *
 * Filename:      lessphun.h
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:   Lessphun is a free Mophun implementation
 *
 * $Id: lessphun.h 15427 2007-05-19 11:31:28Z ska $
 *
 ********************************************************************/
#ifndef __LESSPHUN_H__
#define __LESSPHUN_H__

#include <java/lang.h>
#include <javax/microedition/lcdui.h>
#include <javax/microedition/lcdui/game.h>
#include <javax/microedition/media.h>
#include <resource-manager.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
  int width;           /**< The width of this SPRITE */
  int height;          /**< The height of this SPRITE */

  /* Private */
  int frame;           /* The frame this SPRITE shows */
  NOPH_Image_t handle; /* Java image object */
} SPRITE;

typedef struct
{
  int width;
  int height;
  uint8_t *mapoffset;
  uint8_t *tiledata;      /* Actually a tilemap image */
  int flag;
  int format;

  NOPH_TiledLayer_t tilemap; /* Java tilemap */
} MAP_HEADER;

typedef struct
{
  int size;

  int width;
  int height;
} VIDEOCAPS;

typedef struct
{
  int width;
  int height;
  int chars_per_line;
  NOPH_Image_t image;
} VMGPFONT;


typedef struct
{
  int x;
  int y;
  SPRITE *sprite;
} sprite_entry_t;

typedef struct
{
  NOPH_ResourceManager_t resourceManager;
  NOPH_GameCanvas_t gameCanvas;
  NOPH_Graphics_t   graphics;

  int n_sprite_slots;
  sprite_entry_t **sprite_slots;

  NOPH_TiledLayer_t tiledLayer;
  NOPH_Image_t tilemapImage;
  int tile_w;
  int tile_h;
  MAP_HEADER cur_tilemap;
  VMGPFONT *current_font;
  int64_t start_time;
} lessphun_info_t;

extern lessphun_info_t __LPH_info;

/* Defines */
#define vRGB(r,g,b) ( ((r) << 16) | ((g) << 8) | (b) )
#define KEY_DOWN   NOPH_GameCanvas_DOWN_PRESSED
#define KEY_UP     NOPH_GameCanvas_UP_PRESSED
#define KEY_LEFT   NOPH_GameCanvas_LEFT_PRESSED
#define KEY_RIGHT  NOPH_GameCanvas_RIGHT_PRESSED
#define KEY_FIRE   NOPH_GameCanvas_FIRE_PRESSED
#define KEY_FIRE2  NOPH_GameCanvas_GAME_A_PRESSED
#define KEY_SELECT NOPH_GameCanvas_GAME_B_PRESSED

#define MODE_TRANS 1
#define MODE_BLOCK 2

#define CAPS_VIDEO 1


#define STREAM_FILE     (1<<1)
#define STREAM_READ     (1<<2)
#define STREAM_WRITE    (1<<3)
#define STREAM_BINARY   (1<<4)
#define STREAM_RESOURCE (1<<5)
#define STREAM_CREATE   (1<<6)

#define VSEEK_SET 1
#define VSEEK_CUR 2
#define VSEEK_END 3

/* File-handling things  */
extern int vStreamOpen(const char *filename, int mode);
extern int vStreamRead(int fd, void *out, size_t size);
extern int vStreamWrite(int fd, void *in, size_t size);
extern int vStreamSeek(int fd, int offset, int whence);
extern void vStreamClose(int fd);

/* Memory allocation */
#define vNewPtr malloc
#define vDisposePtr free

/* Timing */
extern uint32_t vGetTickCount(void);
#define msSleep(x) NOPH_Thread_sleep(x)

/* Debug stuff */
#define DbgPrintf printf
#define DbgBreak() /* FIXME! */
#define vMsgBox(x, y) /* FIXME! */

/* Graphics */
extern void vSetTransferMode(int mode);
extern void vSetClipWindow(int x, int y, int w, int h);
extern void vClearScreen(int color);
static inline void vDrawObject(int x, int y, SPRITE *spr)
{
  NOPH_Graphics_drawImage(__LPH_info.graphics, spr->handle, x, y, NOPH_Graphics_TOP | NOPH_Graphics_LEFT);
}
static inline void vFlipScreen(int ignored)
{
  NOPH_GameCanvas_flushGraphics(__LPH_info.gameCanvas);
}
extern void vSetDisplayWindow(int w, int h);
extern void vFillRect(int x1, int y1, int x2, int y2);

/* Sprite handling */
extern int vSpriteInit(int slots);
extern void vSpriteClear(void);
extern void vSpriteSet(int slot, SPRITE *sprite, int x, int y);

/* Tilemap handling */
extern void vMapDispose(void);
extern int vMapInit(MAP_HEADER *p);
static inline void vMapSetXY(int x, int y)
{
  NOPH_TiledLayer_setPosition(__LPH_info.tiledLayer, -x, -y);
}

static inline void vMapSetTile(int x, int y, int tile)
{
  if (x < 0 || y < 0 || x >= __LPH_info.cur_tilemap.width || y >= __LPH_info.cur_tilemap.height)
    return;
  NOPH_TiledLayer_setCell(__LPH_info.tiledLayer, x, y, tile & 0xff);
}

static inline int vMapGetTile(int x, int y)
{
  return NOPH_TiledLayer_getCell(__LPH_info.tiledLayer, x, y);
}

static inline void vUpdateMap(void)
{
  NOPH_TiledLayer_paint(__LPH_info.tiledLayer, __LPH_info.graphics);
}

extern void vUpdateSpriteMap(void);

/* Sound */
void vBeep(int freq, int ms);

/* Font handling */
void vSetForeColor(int color);
void vSetActiveFont(VMGPFONT *font);
void vPrint(int mode, int x, int y, const char *msg);

/* Misc */
extern int vGetCaps(int mode, void *data);
static inline int vGetButtonData(void)
{
  return NOPH_GameCanvas_getKeyStates(__LPH_info.gameCanvas);
}
extern void vTerminateVMGP(void);
extern void vitoa(int val, char *buf, int n_chars, char pad);
#define vSetRandom(x) srand(x)
#define vGetRandom() rand()

/**< Lessphun-private method to decompress sprites */
extern void LPH_setTileImageSize(int w, int h);
extern SPRITE **LPH_splitSprites(SPRITE *src, int n_sprites);
extern void LPH_init(void);

extern void LPH_initSprite(SPRITE *dst, char *filename);
extern void LPH_initFont(VMGPFONT *dst, char *filename, int width, int height);

#endif /* !__LESSPHUN_H__ */
