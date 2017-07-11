from onl.platform.base import *
from onl.platform.inventec import *

class OnlPlatform_x86_64_inventec_redwood_r0(OnlPlatformInventec,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-inventec-redwood-r0'
    MODEL="X86-REDWOOD"
    SYS_OBJECT_ID=".1.32"

    def baseconfig(self):
        self.insmod('inv_platform')
        return True
