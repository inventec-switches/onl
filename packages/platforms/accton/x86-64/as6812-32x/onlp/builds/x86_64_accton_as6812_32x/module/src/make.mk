###############################################################################
#
# 
#
###############################################################################

LIBRARY := x86_64_accton_as6812_32x
$(LIBRARY)_SUBDIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(BUILDER)/lib.mk
