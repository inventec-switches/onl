from onl.platform.base import *
from onl.platform.inventec import *

class OnlPlatform_x86_64_inventec_d6356_r0(OnlPlatformInventec,
                                              OnlPlatformPortConfig_48x25_8x100):
    PLATFORM='x86-64-inventec-d6356-r0'
    MODEL="D6356"
    SYS_OBJECT_ID=".6356.1"

    def baseconfig(self):
	os.system("insmod /lib/modules/`uname -r`/kernel/drivers/gpio/gpio-ich.ko gpiobase=0")
	self.insmod('i2c-gpio')
	self.insmod('inv_platform')
	self.insmod('inv_psoc')
	self.new_i2c_device('inv_cpld', 0x55, 0)
	self.insmod('inv_cpld')
	self.new_i2c_device('inv_eeprom', 0x53, 0)
	self.insmod('inv_eeprom')
	self.insmod('swps')
	#self.insmod('vpd')
	self.insmod('inv_pthread')
	return True
