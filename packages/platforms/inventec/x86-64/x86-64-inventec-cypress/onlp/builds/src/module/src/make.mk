###############################################################################
#
# 
#
###############################################################################

LIBRARY := x86_64_inventec_cypress
$(LIBRARY)_SUBDIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(BUILDER)/lib.mk