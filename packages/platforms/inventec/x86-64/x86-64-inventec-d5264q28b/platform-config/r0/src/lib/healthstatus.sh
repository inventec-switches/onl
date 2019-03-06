#!/bin/bash
#lavender platform
normal='0 : normal'
unpowered='2 : unpowered'
fault='4 : fault'
notinstalled='7 : not installed'

PSOC_PATH=""
CPLD_PATH="/sys/bus/i2c/devices/i2c-0/0-0055"
ROUTE="/sys/class/hwmon/"
for KEY in $(ls ${ROUTE})
do
	KEY=${ROUTE}${KEY}
	if [ -f $KEY"/name" ]; then
		if [ $(cat $KEY"/name") == "inv_psoc" ]; then
			PSOC_PATH=$KEY
		fi
	fi
done
if [[ $PSOC_PATH == "" ]]; then
	echo "ERROR! Unable to find inv_psoc!"
	exit
fi

while true
do
  psu0var=$(cat $CPLD_PATH/psu0)    # bottom PSU
  psu1var=$(cat $CPLD_PATH/psu1)    # top PSU
  fan1in=$(cat $PSOC_PATH/fan1_input)  # fanmodule1, far right, back view
  fan2in=$(cat $PSOC_PATH/fan2_input)  # fanmodule1, far right, back view
  fan3in=$(cat $PSOC_PATH/fan3_input)  # fanmodule2
  fan4in=$(cat $PSOC_PATH/fan4_input)  # fanmodule2
  fan5in=$(cat $PSOC_PATH/fan5_input)  # fanmodule3
  fan6in=$(cat $PSOC_PATH/fan6_input)  # fanmodule3
  fan7in=$(cat $PSOC_PATH/fan7_input)  # fanmodule4, far left, back view
  fan8in=$(cat $PSOC_PATH/fan8_input)  # fanmodule4, far left, back view

  if [ "$psu0var" = "$normal" ] &&
     [ "$psu1var" = "$normal" ] &&                         # PSU normal operation
     [ "$fan1in" -gt 0 ] && [ "$fan2in" -gt 0 ] &&         # fan on
     [ "$fan3in" -gt 0 ] && [ "$fan4in" -gt 0 ] &&
     [ "$fan5in" -gt 0 ] && [ "$fan6in" -gt 0 ] &&
     [ "$fan7in" -gt 0 ] && [ "$fan8in" -gt 0 ]
  then
    echo 1 > $CPLD_PATH/ctl
    echo 0 > $CPLD_PATH/red_led     # red off
    echo 7 > $CPLD_PATH/grn_led     # grn solid

  elif [ "$psu0var" = "$unpowered" ] ||
       [ "$psu1var" = "$unpowered" ] ||                    # PSU unpowered
       [ "$fan1in" -eq 0 ] || [ "$fan2in" -eq 0 ] ||       # fan off
       [ "$fan3in" -eq 0 ] || [ "$fan4in" -eq 0 ] ||
       [ "$fan5in" -eq 0 ] || [ "$fan6in" -eq 0 ] ||
       [ "$fan7in" -eq 0 ] || [ "$fan8in" -eq 0 ]
  then
    echo 1 > $CPLD_PATH/ctl
    echo 0 > $CPLD_PATH/red_led     # red off
    echo 3 > $CPLD_PATH/grn_led     # grn @2Hz

  elif [ "$psu0var" = "$fault" ] ||
       [ "$psu0var" = "$notinstalled" ] ||
       [ "$psu1var" = "$fault" ] ||
       [ "$psu1var" = "$notinstalled" ]                    # PSU fault or PSU not installed
  then
    echo 1 > $CPLD_PATH/ctl
    echo 0 > $CPLD_PATH/grn_led     # grn off
    echo 7 > $CPLD_PATH/red_led     # red solid
  fi
  sleep 1
done
