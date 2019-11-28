###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_inventec_d5256_INCLUDES := -I $(THIS_DIR)inc
x86_64_inventec_d5256_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_inventec_d5256_DEPENDMODULE_ENTRIES := init:x86_64_inventec_d5256 ucli:x86_64_inventec_d5256

