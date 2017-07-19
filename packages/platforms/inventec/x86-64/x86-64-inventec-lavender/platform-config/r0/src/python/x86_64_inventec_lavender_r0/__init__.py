from onl.platform.base import *
from onl.platform.inventec import *

class OnlPlatform_x86_64_inventec_lavender_r0(OnlPlatformInventec,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-inventec-lavender-r0'
    MODEL="X86-LAVENDER"
    SYS_OBJECT_ID=".1.32"

    def baseconfig(self):
        os.system("insmod /lib/modules/`uname -r`/onl/inventec/x86-64-inventec-lavender/gpio-ich.ko gpiobase=0")
        self.insmod('inv_platform')
        self.insmod('inv_psoc')
        self.insmod('inv_cpld')
        self.insmod('inv_mux')
        self.insmod('io_expander')
        self.insmod('transceiver')
        self.insmod('inv_swps')
        return True
