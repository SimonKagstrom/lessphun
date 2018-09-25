######################################################################
##
## Copyright (C) 2006,  Blekinge Institute of Technology
##
## Filename:      Rules-java.mk
## Author:        Simon Kagstrom <ska@bth.se>
## Description:   Lessphun Makefile rules
##
## $Id: Rules-lessphun.mk 15197 2007-04-27 17:29:49Z ska $
##
######################################################################
INCLUDES += -I$(LESSPHUN_BASE)/include
EXTRA_CLEAN += res.c res.h

include $(CIBYL_BASE)/build/Rules.mk
LDOPTS   += -L$(LESSPHUN_BASE)/libs/lib
LDOPTS_DEBUG   += -L$(LESSPHUN_BASE)/libs/lib

res.c: res.h

res.h: res.txt
	$(LESSPHUN_BASE)/tools/lorc/lorc ../resources $<
