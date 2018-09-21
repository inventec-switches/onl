from onl.platform.base import *
from onl.platform.inventec import *

class OnlPlatform_x86_64_inventec_cypress_r0(OnlPlatformInventec,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-inventec-cypress-r0'
    MODEL="X86-CYPRESS"
    SYS_OBJECT_ID=".1.32"

    def baseconfig(self):
	os.system("insmod /lib/modules/`uname -r`/kernel/drivers/gpio/gpio-ich.ko")
        self.insmod('inv_platform')
        self.insmod('inv_psoc')
        self.insmod('inv_cpld')
        self.insmod('inv_mux')
        self.insmod('io_expander')
        self.insmod('transceiver')
        self.insmod('inv_swps')
        self.insmod('onie_tlvinfo')
        self.insmod('inv_vpd')
        return True
