from onl.platform.base import *
from onl.platform.inventec import *

class OnlPlatform_x86_64_inventec_d5254_r0(OnlPlatformInventec,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-inventec-d5254-r0'
    MODEL="D5254"
    SYS_OBJECT_ID=".1.32"

    def baseconfig(self):
        os.system("insmod /lib/modules/`uname -r`/onl/inventec/x86-64-inventec-d5254/gpio-ich.ko")
        #self.insmod('gpio-ich.ko')
        self.insmod('i2c-gpio')
        self.insmod('inv_platform')
        self.insmod('inv_psoc')
        os.system("echo inv_cpld 0x55 > /sys/bus/i2c/devices/i2c-0/new_device")
        os.system("echo inv_cpld 0x77 > /sys/bus/i2c/devices/i2c-0/new_device")
        self.insmod('inv_cpld')
        self.insmod('inv_mux')
        self.insmod('io_expander')
        self.insmod('transceiver')
        self.insmod('inv_swps')
        self.insmod('onie_tlvinfo')
        self.insmod('inv_vpd')
        os.system("/lib/platform-config/x86-64-inventec-d5254-r0/onl/healthstatus.sh &")

        return True

