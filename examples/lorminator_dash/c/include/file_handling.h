/*********************************************************************
 *
 * Copyright (C) 2004,  Blekinge Institute of Technology
 *
 * Filename:      file_handling.h
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:   File handling defs
 *
 * $Id: file_handling.h 11242 2006-09-25 18:48:33Z ska $
 *
 ********************************************************************/
#ifndef __FILE_HANDLING_H__
#define __FILE_HANDLING_H__

typedef int32_t  file_handle_t;

#include "types.h"
#include "point.h"

typedef uint16_t header_type_t;

#define HDR_TYPE_SECT_TABLE  1
#define HDR_TYPE_LEVEL_SET   2
#define HDR_TYPE_LEVEL       3
#define HDR_TYPE_LEVEL_DATA  4
#define HDR_TYPE_SPECIAL     5

#define HDR_MAGIC            0xaffebaff
#define FILE_MAGIC           0x19760513

#define API_VERSION          0x01

typedef struct
{
  uint32_t magic;
  uint32_t section_table_offs;
  uint16_t section_table_size;
  uint16_t size;
  uint8_t  API_version;
  uint8_t  n_level_sets;

  uint8_t  padding[2];
  uint16_t level_set_idx[];
} file_header_t;

typedef struct
{
  uint32_t      file_offset;
  header_type_t type;
  uint16_t      size;
} section_table_entry_t;

typedef struct
{
  uint8_t n_levels;
  uint8_t name[15];

  uint16_t level_idx[];
} file_level_set_t;

typedef struct
{
  uint16_t  n_sections;
  uint8_t   padding[2];

  section_table_entry_t entries[];
} file_section_table_t;

typedef struct
{
  uint32_t  time;
  point_t   player_pt; /* 4 bytes */
  uint8_t   diamonds_required;
  uint8_t   n_specials;

  uint8_t   special_init_idx;
  uint8_t   level_data_sect;

  uint8_t   padding[3];

  uint16_t  specials_idx[];
} file_level_t;

typedef struct
{
  uint8_t   w;
  uint8_t   h;

  uint8_t   padding[2];

  tile_t    data[];
} file_level_data_t;

typedef struct
{
  int16_t   private_data_size;
  uint8_t   special_fn_idx;
  uint8_t   type;


  uint8_t   private_data[];
} file_special_t;

typedef struct
{
  uint32_t      magic;
  header_type_t type;
  uint16_t      size;

  union
  {
    file_section_table_t sec_table;
    file_level_t         level;
    file_level_set_t     level_set;
    file_level_data_t    level_data;
    file_special_t       special;
  } u;
} file_section_t;

#define FILE_SECTION_MINSIZE ( sizeof(uint32_t) + sizeof(header_type_t) + sizeof(uint16_t) )
#define FILE_HEADER_MINSIZE  sizeof(file_header_t)

file_section_t *file_section_get(file_handle_t handle, file_section_table_t *p_sectab, int level_index);
file_section_table_t *file_get_section_table(file_handle_t handle, file_header_t *p_hdr);
file_header_t *file_read_header(file_handle_t handle);


#endif /* !__FILE_HANDLING_H__ */
