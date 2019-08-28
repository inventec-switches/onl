from onl.platform.base import *
from onl.platform.inventec import *

class OnlPlatform_arm_inventec_d2428_r0(OnlPlatformInventec,
                                          OnlPlatformPortConfig_24x1_4x10):
    PLATFORM='arm-inventec-d2428-r0'
    MODEL="D2428"
    SYS_OBJECT_ID=".2428.1"

    def baseconfig(self):
        self.insmod("inv_cpld")
        self.insmod("swps")
        self.insmod("vpd")

        return True
