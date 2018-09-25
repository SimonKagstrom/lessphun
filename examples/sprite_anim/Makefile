######################################################################
##
## Copyright (C) 2005,  Blekinge Institute of Technology
##
## Filename:      Makefile
## Author:        Simon Kagstrom <ska@bth.se>
## Description:   Generic Makefile
##
## $Id: Makefile 11066 2006-09-15 16:05:44Z ska $
##
######################################################################
SUBDIRS = c
RESOURCES=res/program.data.bin res/sprite.png res/tiles.png
TARGET   =src/CompiledProgram.class midlet.jar midlet.jad

# This is actually not needed since we include Rules-j2me, but
# we keep it here to show how new syscall sets are added. Further,
# ansi is implicitly implied by j2me, so that can be removed as well
CIBYL_SYSCALL_SETS ?= resource_manager

include $(CIBYL_BASE)/build/Rules-j2me.mk
