###############################################################################
#
# 
#
###############################################################################

LIBRARY := x86_64_inventec_d6356j
$(LIBRARY)_SUBDIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(BUILDER)/lib.mk