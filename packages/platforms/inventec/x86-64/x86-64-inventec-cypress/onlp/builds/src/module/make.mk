###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_inventec_cypress_INCLUDES := -I $(THIS_DIR)inc
x86_64_inventec_cypress_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_inventec_cypress_DEPENDMODULE_ENTRIES := init:x86_64_inventec_cypress ucli:x86_64_inventec_cypress

