###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_inventec_lavender_INCLUDES := -I $(THIS_DIR)inc
x86_64_inventec_lavender_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_inventec_lavender_DEPENDMODULE_ENTRIES := init:x86_64_inventec_lavender ucli:x86_64_inventec_lavender

