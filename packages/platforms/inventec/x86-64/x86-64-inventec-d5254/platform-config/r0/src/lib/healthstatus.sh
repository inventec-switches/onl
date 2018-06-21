#!/bin/bash

normal='0 : normal'
unpowered='2 : unpowered'
fault='4 : fault'
notinstalled='7 : not installed'
FAN_UNPLUG_NUM=0
FAN_LED_RED='fan_led_red'
NUM=1
FAN_NUM=5
#PSU_STAUS='000'
#switch is ready , transfer control of cpld to cpu
echo 1 > /sys/bus/i2c/devices/i2c-0/0-0055/ctl

while true
do

NUM=1
FAN_UNPLUG_NUM=0
#monitor how many fan modules are unplugged
while [  $NUM -le $FAN_NUM ]; do
        if [ $(cat /sys/class/hwmon/hwmon1/$FAN_LED_RED$NUM) -eq 1 ]
        then
          let FAN_UNPLUG_NUM=FAN_UNPLUG_NUM+1       
        fi
        let NUM=NUM+1
done

#echo "FAN_UNPLUG_NUM:$FAN_UNPLUG_NUM"

if [ $FAN_UNPLUG_NUM -ge 2 ]
then
  #echo "solid red"  
  echo 7 > /sys/bus/i2c/devices/i2c-0/0-0055/red_led
  echo 0 > /sys/bus/i2c/devices/i2c-0/0-0055/grn_led
elif [ $FAN_UNPLUG_NUM -eq 1 ]
then
  #echo "blink at 1hz in red"
  echo 2 > /sys/bus/i2c/devices/i2c-0/0-0055/red_led
  echo 0 > /sys/bus/i2c/devices/i2c-0/0-0055/grn_led
else

  #echo "normal"
  psu0var=$(cat /sys/bus/i2c/devices/i2c-0/0-0055/psu0)    # bottom PSU
  psu1var=$(cat /sys/bus/i2c/devices/i2c-0/0-0055/psu1)    # top PSU

   if [ "$psu0var" = "$normal" ] &&
     [ "$psu1var" = "$normal" ]                          # PSU normal operatio
   then
      #solid green
      echo 7 > /sys/bus/i2c/devices/i2c-0/0-0055/grn_led
      echo 0 > /sys/bus/i2c/devices/i2c-0/0-0055/red_led
   else  
      if [ "$psu0var" = "$unpowered" ] ||
      [ "$psu1var" = "$unpowered" ]
      then
        #echo "grn blink at hz"
        echo 2 > /sys/bus/i2c/devices/i2c-0/0-0055/grn_led
        echo 0 > /sys/bus/i2c/devices/i2c-0/0-0055/red_led
      else
        #solid red
        echo 7 > /sys/bus/i2c/devices/i2c-0/0-0055/red_led
        echo 0 > /sys/bus/i2c/devices/i2c-0/0-0055/grn_led
      fi
   fi #not both normal
fi

sleep 1
done

