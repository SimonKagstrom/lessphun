######################################################################
##
## Copyright (C) 2006,  Blekinge Institute of Technology
##
## Filename:      Makefile
## Author:        Simon Kagstrom <ska@bth.se>
## Description:
##
## $Id: Makefile 12437 2006-11-24 14:54:48Z ska $
##
######################################################################
SUBDIRS=libs

all: $(SUBDIRS)

clean:
	make -C $(SUBDIRS) clean
	find . -name "*~" -or -name "*.pyc" | xargs rm -f

FORCE:

.PHONY: $(SUBDIRS)

$(SUBDIRS):
	if [ -f $@/Makefile ]; then make -C $@ ; fi
