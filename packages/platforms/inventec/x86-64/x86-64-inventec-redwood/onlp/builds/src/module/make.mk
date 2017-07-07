###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_inventec_redwood_INCLUDES := -I $(THIS_DIR)inc
x86_64_inventec_redwood_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_inventec_redwood_DEPENDMODULE_ENTRIES := init:x86_64_inventec_redwood ucli:x86_64_inventec_redwood

