INCLUDES=include .
KERNEL := onl-kernel-4.4-lts-armel-iproc-all:armel
DTS_LIST := arm-inventec-d2428.dts
VPATH := $(ONL)/packages/platforms/inventec/armel/arm-inventec-d2428/platform-config/r0/builds/dtb
include $(ONL)/make/dtbs.mk

#
# The D2428 relies on the common arm devices tree includes. These are linked here from the kernel package.
#
setup::
	onlpm --link-dir onl-kernel-4.4-lts-armel-iproc-all:armel /usr/share/onl/packages/armel/onl-kernel-4.4-lts-armel-iproc-all/mbuilds/arch/arm/boot/dts dts
	onlpm --link-dir onl-kernel-4.4-lts-armel-iproc-all:armel /usr/share/onl/packages/armel/onl-kernel-4.4-lts-armel-iproc-all/mbuilds/include include

setup-clean::
	rm -f dts include
