from onl.platform.base import *
from onl.platform.inventec import *

class OnlPlatform_x86_64_inventec_redwood_r0(OnlPlatformInventec,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-inventec-redwood-r0'
    MODEL="X86-REDWOOD"
    SYS_OBJECT_ID=".1.32"

    def baseconfig(self):
	os.system("insmod /lib/modules/`uname -r`/kernel/drivers/gpio/gpio-ich.ko gpiobase=0")
        self.insmod('inv_platform')
        self.insmod('inv_psoc')
        self.insmod('inv_cpld')
        self.insmod('inv_mux')
        self.insmod('io_expander')
        self.insmod('transceiver')
        self.insmod('onie_tlvinfo')
        self.insmod('inv_vpd')
        self.insmod('inv_swps')
        #self.insmod('inventec_class')
        #self.insmod('led_main_device')
        #self.insmod('ports_device')
        #self.insmod('psu_device')
        #self.insmod('fan_device')
        #self.insmod('sensor_device')
        #self.insmod('system_device')
        #self.insmod('inventec_thread')
        return True
