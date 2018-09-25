/*********************************************************************
 *
 * Copyright (C) 2004,  Blekinge Institute of Technology
 *
 * Filename:      file_handling.c
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:   File handling utils
 *
 * $Id: file_handling.c 11360 2006-09-28 13:06:35Z ska $
 *
 ********************************************************************/
#include <utils.h>
#include <file_handling.h>
#include <tiles.h>

file_section_t *file_section_get(file_handle_t handle, file_section_table_t *p_sectab, int level_index)
{
  file_section_t *p_out;

  if (level_index >= p_sectab->n_sections)
    {
      debug_msg("Index out of bounds\n");
      goto clean_0;
    }

  if ( !(p_out = vNewPtr(p_sectab->entries[level_index].size)) )
    goto clean_0;
  if (vStreamSeek(handle, p_sectab->entries[level_index].file_offset, VSEEK_SET) < 0)
    goto clean_1;
  if ( vStreamRead(handle, p_out, p_sectab->entries[level_index].size) < 0)
    {
      debug_msg("Reading failed!\n");
      goto clean_1;
    }
  if (p_out->magic != HDR_MAGIC)
    {
      debug_msg("Section magic wrong: 0x%08x!\n", p_out->magic);
      goto clean_1;
    }

  return p_out;

 clean_1:
  vDisposePtr(p_out);
 clean_0:
  return NULL;
}


file_section_table_t *file_get_section_table(file_handle_t handle, file_header_t *p_hdr)
{
  file_section_table_t *p_out;

  if ( !(p_out = vNewPtr(p_hdr->section_table_size)) )
    goto clean_0;

  if (vStreamSeek(handle, p_hdr->section_table_offs, VSEEK_SET) < 0)
    goto clean_1;
  if (vStreamRead(handle, p_out, p_hdr->section_table_size) < 0)
    goto clean_1;

  return p_out;

 clean_1:
  vDisposePtr(p_out);
 clean_0:
  return NULL;
}


file_header_t *file_read_header(file_handle_t handle)
{
  file_header_t *p_out;
  file_header_t hdr;

  if (vStreamRead(handle, &hdr, sizeof(file_header_t)) < 0)
    goto clean_0;

  if (hdr.magic != FILE_MAGIC)
    goto clean_0;

  if ( !(p_out = vNewPtr(hdr.size)) )
    goto clean_0;

  if (vStreamSeek(handle, 0, VSEEK_SET) < 0)
    goto clean_1;
  if (vStreamRead(handle, p_out, hdr.size) < 0)
    goto clean_1;

  return p_out;

 clean_1:
  vDisposePtr(p_out);
 clean_0:
  return NULL;
}
