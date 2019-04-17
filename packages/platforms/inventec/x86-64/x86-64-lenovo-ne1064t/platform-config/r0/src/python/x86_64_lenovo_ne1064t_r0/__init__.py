from onl.platform.base import *
from onl.platform.inventec import *

class OnlPlatform_x86_64_lenovo_ne1064t_r0(OnlPlatformInventec,
                                              OnlPlatformPortConfig_48x10_6x100):
    PLATFORM='x86-64-lenovo-ne1064t-r0'
    MODEL="NE1064T"
    SYS_OBJECT_ID=".1064.1"

    def baseconfig(self):
	os.system("insmod /lib/modules/`uname -r`/kernel/drivers/gpio/gpio-ich.ko gpiobase=0")
	self.insmod('i2c-gpio')
	self.insmod('inv_platform')
	self.new_i2c_device('inv_cpld', 0x77, 0)
	self.insmod('inv_cpld')
	self.new_i2c_device('inv_eeprom', 0x57, 0)
	self.insmod('inv_eeprom')
	self.insmod('swps')
	#RYU #self.insmod('vpd')
	#RYU self.insmod('inv_pthread')
	#RYU self.insmod('inv_psoc')
	return True
