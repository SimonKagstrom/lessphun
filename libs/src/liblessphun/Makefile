######################################################################
##
## Copyright (C) 2005,  Blekinge Institute of Technology
##
## Filename:      Makefile
## Author:        Simon Kagstrom <ska@bth.se>
## Description:   Generic Makefile
##
## $Id: Makefile 12198 2006-11-16 18:30:59Z ska $
##
######################################################################
SUBDIRS =
OBJS    = lessphun.o
ALL_TARGETS=install
EXTRA_CLEAN=liblessphun.a

include $(LESSPHUN_BASE)/build/Rules-lessphun.mk

install: liblessphun.a
	cp $< $(LESSPHUN_BASE)/libs/lib
