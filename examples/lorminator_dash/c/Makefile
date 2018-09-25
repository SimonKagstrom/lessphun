######################################################################
##
## Copyright (C) 2005,  Blekinge Institute of Technology
##
## Filename:      Makefile
## Author:        Simon Kagstrom <ska@bth.se>
## Description:   Generic Makefile
##
## $Id: Makefile 12294 2006-11-20 20:33:23Z ska $
##
######################################################################
SUBDIRS =
TARGET  = program
INCLUDES=-Iinclude -Ilib/include
OBJS    = res.o elem.o callbacks.o main.o levels.o level_handling.o \
          sprite.o sound.o highscore.o specials.o status_field.o file_handling.o \
          lib/menu/menu.o lib/utils/utils.o
LDLIBS  = -lc -llessphun

include $(LESSPHUN_BASE)/build/Rules-lessphun.mk
