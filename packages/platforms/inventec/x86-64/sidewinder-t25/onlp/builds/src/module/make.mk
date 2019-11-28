###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_sidewinder_t25_INCLUDES := -I $(THIS_DIR)inc
x86_64_sidewinder_t25_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_sidewinder_t25_DEPENDMODULE_ENTRIES := init:x86_64_sidewinder_t25 ucli:x86_64_sidewinder_t25

