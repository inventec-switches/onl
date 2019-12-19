from onl.platform.base import *
from onl.platform.inventec import *
import logging

class OnlPlatform_x86_64_lenovo_ne2580_r0(OnlPlatformInventec,
                                              OnlPlatformPortConfig_48x25_8x100):
    PLATFORM='x86-64-lenovo-ne2580-r0'
    MODEL="NE2580"
    SYS_OBJECT_ID=".2580.1"

    _path_prefix_list=[
        "/sys/bus/i2c/devices/2-005a/hwmon/",
        "/sys/bus/i2c/devices/2-005b/hwmon/",
        "/sys/devices/platform/coretemp.0/hwmon/",
        "/sys/bus/i2c/devices/3-0048/hwmon/",
        "/sys/bus/i2c/devices/3-004a/hwmon/",
        "/sys/bus/i2c/devices/3-004d/hwmon/",
        "/sys/bus/i2c/devices/3-004e/hwmon/"
    ]
    _path_dst_list=[
        "/var/psu1",
        "/var/psu2",
        "/var/coretemp",
        "/var/thermal_6",
        "/var/thermal_7",
        "/var/thermal_8",
        "/var/thermal_9"
    ]

    def baseconfig(self):
        os.system("insmod /lib/modules/`uname -r`/kernel/drivers/gpio/gpio-ich.ko gpiobase=0")
        self.insmod('i2c-gpio')
        self.insmod('inv_platform')
        self.new_i2c_device('inv_cpld', 0x77, 0)
        self.insmod('inv_cpld')
        self.insmod('swps')
        self.new_i2c_device('inv_eeprom', 0x57, 0)
        self.insmod('inv_eeprom')

        for i in range(0,len(self._path_prefix_list)):
            if( os.path.islink(self._path_dst_list[i]) ):
                os.unlink(self._path_dst_list[i])
                logging.warning("Path %s exists, remove before link again" % self._path_dst_list[i] )
            self.link_dir(self._path_prefix_list[i],self._path_dst_list[i])

        return True

    def link_dir(self,prefix,dst):
        ret=os.path.isdir(prefix)
        if ret==True:
            dirs=os.listdir(prefix)
            ret=False
            for i in range(0,len(dirs)):
                if 'hwmon' in dirs[i]:
                    src=prefix+dirs[i]
                    os.symlink(src,dst)
                    ret=True
                    break
            if ret==False:
                logging.warning("Can't find proper dir to link under %s" % prefix)            
        else:
            logging.warning("Path %s is not a dir" % prefix)


