###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_lenovo_ne1064t_INCLUDES := -I $(THIS_DIR)inc
x86_64_lenovo_ne1064t_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_lenovo_ne1064t_DEPENDMODULE_ENTRIES := init:x86_64_lenovo_ne1064t ucli:x86_64_lenovo_ne1064t

