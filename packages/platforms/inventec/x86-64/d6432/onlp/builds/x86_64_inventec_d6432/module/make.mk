###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_inventec_d6432_INCLUDES := -I $(THIS_DIR)inc
x86_64_inventec_d6432_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_inventec_d6432_DEPENDMODULE_ENTRIES := init:x86_64_inventec_d6432 ucli:x86_64_inventec_d6432
x86_64_inventec_d6432_BROKEN_CFLAGS += -Wno-restrict -Wno-format-truncation
