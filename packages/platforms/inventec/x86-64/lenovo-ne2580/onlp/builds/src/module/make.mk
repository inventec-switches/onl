###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_lenovo_ne2580_INCLUDES := -I $(THIS_DIR)inc
x86_64_lenovo_ne2580_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_lenovo_ne2580_DEPENDMODULE_ENTRIES := init:x86_64_lenovo_ne2580 ucli:x86_64_lenovo_ne2580

