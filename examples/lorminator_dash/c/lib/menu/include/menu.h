/*********************************************************************
 *
 * Copyright (C) 2004,  Blekinge Institute of Technology
 *
 * Filename:      menu.h
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:
 *
 * $Id: menu.h 11242 2006-09-25 18:48:33Z ska $
 *
 ********************************************************************/
#ifndef __MENU_H__
#define __MENU_H__

#include <lessphun.h>

typedef struct
{
  int n_entries;
  int index;
  int sel;
} submenu_t;


typedef struct
{
  char     **pp_msgs;
  VMGPFONT  *p_font;
  int16_t    x1,y1;
  int16_t    x2,y2;
  int16_t    text_w;
  int16_t    text_h;
  uint32_t   available_options;

  int        n_submenus;
  submenu_t *p_submenus;

  int        cur_sel; /* Main selection */
  int        n_entries;
} menu_t;

void menu_init(menu_t *p_menu, VMGPFONT *p_font, char *pp_msgs[],
	       int16_t x1, int16_t y1, int16_t x2, int16_t y2);
void menu_fini(menu_t *p_menu);

int menu_select(menu_t *p_menu, uint32_t available_options, int *p_submenus);

#endif /* !__MENU_H__ */
